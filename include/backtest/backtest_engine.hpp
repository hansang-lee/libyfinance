#pragma once

#include <cstddef>
#include <string>
#include <vector>

#include "strategy/istrategy.hpp"

struct Trade {
    std::size_t buyIndex  = 0;
    std::size_t sellIndex = 0;
    double      buyPrice  = 0.0;
    double      sellPrice = 0.0;
    double      returnPct = 0.0;  // (sellPrice - buyPrice) / buyPrice * 100
};

struct BacktestResult {
    std::string ticker;
    std::string strategyName;

    double initialCapital = 0.0;
    double finalCapital   = 0.0;

    /* ----- Score Components ----- */
    double totalReturnPct = 0.0;  // Total return percentage
    double winRate        = 0.0;  // Winning trades / Total trades (0~1)
    double maxDrawdownPct = 0.0;  // Maximum drawdown percentage (negative)
    double sharpeRatio    = 0.0;  // Annualized Sharpe ratio

    /* ----- Composite Score (0~100) ----- */
    double score = 0.0;

    /* ----- Trade History ----- */
    std::vector<Trade> trades;
};

/**
 * @brief Backtesting engine that simulates a strategy over historical data.
 *
 * Runs a strategy against StockInfo, tracks trades and porfolio equity,
 * then computes performance metrics and a composite score.
 */
class BacktestEngine {
   public:
    /**
     * @param initialCapital Starting capital for the simulation (default: $10,000).
     */
    explicit BacktestEngine(double initialCapital = 10000.0);

    /**
     * @brief Run the backtest.
     * @param strategy The investment strategy to evaluate.
     * @param data     Historical stock data.
     * @return BacktestResult with all performance metrics and trade list.
     */
    [[nodiscard]] BacktestResult run(IStrategy& strategy, const StockInfo& data);

   private:
    double initialCapital_;

    /**
     * @brief Compute composite score from individual metrics.
     *
     * Weights: TotalReturn(30%), WinRate(25%), Sharpe(25%), MDD(20%)
     */
    [[nodiscard]] static double computeScore(double totalReturnPct, double winRate, double maxDrawdownPct,
                                             double sharpeRatio);
};
