#include <ctime>
#include <functional>
#include <iomanip>
#include <iostream>

#include "backtest/backtest_engine.hpp"
#include "rsi_strategy.hpp"
#include "sma_crossover.hpp"
#include "yfinance.hpp"

struct Defer {
    std::function<void()> f;
    explicit Defer(std::function<void()> f)
        : f(std::move(f)) {}
    ~Defer() {
        if (f) {
            f();
        }
    }
};

inline std::string formatTime(const int64_t timestamp) {
    const std::time_t t = static_cast<std::time_t>(timestamp);
    char              mbstr[100];
    std::strftime(mbstr, sizeof(mbstr), "%Y-%m-%d", std::localtime(&t));
    return mbstr;
}

void printSummary(const BacktestResult& result, const StockInfo& data) {
    // clang-format off
    std::clog << "\n"
        << "=== Backtest Result: " << result.strategyName << " ===" << "\n"
        << "Ticker:         " << result.ticker << "\n"
        << "Period:         "
            << formatTime(data.timestamps.front()) << " ~ "
            << formatTime(data.timestamps.back()) << "\n"
        << std::fixed << std::setprecision(2)
        << "Initial:        $" << result.initialCapital << "\n"
        << "Final:          $" << result.finalCapital << "\n"
        << "-" << "\n"
        << "Total Return:   " << result.totalReturnPct << "%" << "\n"
        << "Win Rate:       " << (result.winRate * 100.0) << "%"
            << " (" << std::count_if(result.trades.begin(), result.trades.end(),
                                     [](const Trade& t) { return t.returnPct > 0; })
            << "/" << result.trades.size() << ")" << "\n"
        << "Max Drawdown:   " << result.maxDrawdownPct << "%" << "\n"
        << "Sharpe Ratio:   " << result.sharpeRatio << "\n"
        << "-" << "\n"
        << "SCORE:          " << result.score << " / 100" << "\n"
        << std::endl;
    // clang-format on
}

void printTrades(const BacktestResult& result, const StockInfo& data) {
    if (result.trades.empty()) {
        std::clog << "(No trades executed)" << std::endl;
        return;
    }

    // clang-format off
    std::clog << "=== Trades ===" << "\n"
        << std::left
        << std::setw(16) << "(Buy Date)"
        << std::setw(16) << "(Sell Date)"
        << std::setw(12) << "(Buy)"
        << std::setw(12) << "(Sell)"
        << std::setw(12) << "(Return)"
        << "\n-"
        << std::endl;
    // clang-format on

    for (const auto& trade : result.trades) {
        // clang-format off
        std::clog << std::left
            << std::setw(16) << formatTime(data.timestamps[trade.buyIndex])
            << std::setw(16) << formatTime(data.timestamps[trade.sellIndex])
            << std::fixed << std::setprecision(2)
            << "$" << std::setw(11) << trade.buyPrice
            << "$" << std::setw(11) << trade.sellPrice
            << (trade.returnPct >= 0.0 ? "+" : "") << trade.returnPct << "%"
            << std::endl;
        // clang-format on
    }
}

int main() {
    yFinance::init();
    Defer _cleanup([] { yFinance::close(); });

    // Fetch 5 years of SPY daily data
    const auto data = yFinance::getStockInfo("SPY", "2021-01-01", "2026-01-01", "1d");
    if (!data) {
        std::cerr << "Failed to fetch stock data." << std::endl;
        return 1;
    }

    std::clog << "Fetched " << data->close.size() << " data points for " << data->ticker << std::endl;

    BacktestEngine engine(10000.0);

    // Strategy 1: SMA Crossover (20/50)
    SmaCrossover sma(20, 50);
    const auto   smaResult = engine.run(sma, *data);
    printSummary(smaResult, *data);
    printTrades(smaResult, *data);

    // Strategy 2: RSI (14, 30/70)
    RsiStrategy rsi(14, 30.0, 70.0);
    const auto  rsiResult = engine.run(rsi, *data);
    printSummary(rsiResult, *data);
    printTrades(rsiResult, *data);

    return 0;
}
