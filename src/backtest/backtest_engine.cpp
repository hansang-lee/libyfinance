#include "backtest/backtest_engine.hpp"

#include <algorithm>
#include <cmath>
#include <numeric>

BacktestEngine::BacktestEngine(double initialCapital)
    : initialCapital_(initialCapital) {}

BacktestResult BacktestEngine::run(IStrategy& strategy, const StockInfo& data) {
    BacktestResult result;
    result.ticker         = data.ticker;
    result.strategyName   = strategy.name();
    result.initialCapital = initialCapital_;

    if (data.close.empty()) {
        result.finalCapital = initialCapital_;
        return result;
    }

    // Initialize strategy (precompute indicators)
    strategy.init(data);

    const auto warmup = strategy.warmupPeriod();
    const auto n      = data.close.size();

    // Simulation state
    double      capital  = initialCapital_;
    double      shares   = 0.0;
    bool        inPos    = false;
    double      buyPrice = 0.0;
    std::size_t buyIdx   = 0;

    // Equity curve for drawdown & sharpe calculation
    std::vector<double> equity;
    equity.reserve(n);

    for (std::size_t i = 0; i < n; ++i) {
        const double price         = data.close[i];
        const double currentEquity = inPos ? (shares * price) : capital;
        equity.push_back(currentEquity);

        if (i < warmup) {
            continue;
        }

        const auto signal = strategy.evaluate(data, i);

        if (signal == Signal::BUY && !inPos) {
            // Buy: invest all capital
            shares   = capital / price;
            buyPrice = price;
            buyIdx   = i;
            inPos    = true;
            capital  = 0.0;
        } else if (signal == Signal::SELL && inPos) {
            // Sell: liquidate all shares
            capital = shares * price;

            Trade trade;
            trade.buyIndex  = buyIdx;
            trade.sellIndex = i;
            trade.buyPrice  = buyPrice;
            trade.sellPrice = price;
            trade.returnPct = (price - buyPrice) / buyPrice * 100.0;

            result.trades.push_back(trade);

            shares = 0.0;
            inPos  = false;
        }
    }

    // If still in position at the end, close at last price
    if (inPos && !data.close.empty()) {
        const double lastPrice = data.close.back();
        capital                = shares * lastPrice;

        Trade trade;
        trade.buyIndex  = buyIdx;
        trade.sellIndex = n - 1;
        trade.buyPrice  = buyPrice;
        trade.sellPrice = lastPrice;
        trade.returnPct = (lastPrice - buyPrice) / buyPrice * 100.0;

        result.trades.push_back(trade);

        shares = 0.0;
        inPos  = false;
    }

    result.finalCapital = capital;

    // --- Compute metrics ---

    // 1. Total Return
    result.totalReturnPct = (result.finalCapital - initialCapital_) / initialCapital_ * 100.0;

    // 2. Win Rate
    if (!result.trades.empty()) {
        const auto wins =
            std::count_if(result.trades.begin(), result.trades.end(), [](const Trade& t) { return t.returnPct > 0.0; });
        result.winRate = static_cast<double>(wins) / static_cast<double>(result.trades.size());
    }

    // 3. Max Drawdown
    if (!equity.empty()) {
        double peak  = equity[0];
        double maxDD = 0.0;
        for (const auto& eq : equity) {
            peak            = std::max(peak, eq);
            const double dd = (eq - peak) / peak * 100.0;
            maxDD           = std::min(maxDD, dd);
        }
        result.maxDrawdownPct = maxDD;
    }

    // 4. Sharpe Ratio (annualized, assuming daily data, risk-free = 0)
    if (equity.size() > 1) {
        std::vector<double> dailyReturns;
        dailyReturns.reserve(equity.size() - 1);
        for (std::size_t i = 1; i < equity.size(); ++i) {
            if (equity[i - 1] > 0.0) {
                dailyReturns.push_back((equity[i] - equity[i - 1]) / equity[i - 1]);
            }
        }

        if (!dailyReturns.empty()) {
            const double mean = std::accumulate(dailyReturns.begin(), dailyReturns.end(), 0.0)
                              / static_cast<double>(dailyReturns.size());

            double variance = 0.0;
            for (const auto& r : dailyReturns) {
                variance += (r - mean) * (r - mean);
            }
            variance /= static_cast<double>(dailyReturns.size());

            const double stdDev = std::sqrt(variance);
            if (stdDev > 1e-12) {
                // Annualize: multiply by sqrt(252 trading days)
                result.sharpeRatio = (mean / stdDev) * std::sqrt(252.0);
            }
        }
    }

    // 5. Composite Score
    result.score = computeScore(result.totalReturnPct, result.winRate, result.maxDrawdownPct, result.sharpeRatio);

    return result;
}

double BacktestEngine::computeScore(double totalReturnPct, double winRate, double maxDrawdownPct, double sharpeRatio) {
    // Normalize each component to roughly [0, 1] range, then weight and sum.
    // This is a heuristic scoring system.

    // Total Return: clamp to [-50, 100], map to [0, 1]
    const double retNorm = std::clamp((totalReturnPct + 50.0) / 150.0, 0.0, 1.0);

    // Win Rate: already [0, 1]
    const double wrNorm = std::clamp(winRate, 0.0, 1.0);

    // Max Drawdown: range [-50, 0], lower (more negative) is worse
    // Map: 0% MDD → 1.0, -50% MDD → 0.0
    const double mddNorm = std::clamp(1.0 + (maxDrawdownPct / 50.0), 0.0, 1.0);

    // Sharpe Ratio: clamp to [-1, 3], map to [0, 1]
    const double sharpeNorm = std::clamp((sharpeRatio + 1.0) / 4.0, 0.0, 1.0);

    // Weighted sum → scale to [0, 100]
    const double weighted = (retNorm * 0.30) + (wrNorm * 0.25) + (mddNorm * 0.20) + (sharpeNorm * 0.25);

    return std::clamp(weighted * 100.0, 0.0, 100.0);
}
