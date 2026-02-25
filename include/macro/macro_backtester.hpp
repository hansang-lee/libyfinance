#pragma once

#include <map>
#include <memory>
#include <string>
#include <vector>

#include <nlohmann/json.hpp>

#include "macro_scorer.hpp"
#include "stock_info.hpp"

struct MacroBacktestPeriod {
    std::string date;
    MacroScores scores;
    Regime      regime     = Regime::Slowdown;
    Regime      prevRegime = Regime::Slowdown;
    Allocation  alloc;
    bool        allocChanged = true;
    double      equity       = 0.0;
    double      monthReturn  = 0.0;  // portfolio return this month (%)
};

struct MacroBacktestResult {
    std::string frequency;
    double      initialCapital = 0.0;
    double      finalCapital   = 0.0;
    double      totalReturnPct = 0.0;
    double      cagr           = 0.0;
    double      sharpeRatio    = 0.0;
    double      maxDrawdownPct = 0.0;
    int         rebalanceCount = 0;

    std::vector<MacroBacktestPeriod> periods;
};

class MacroBacktester {
   public:
    /**
     * @brief Run portfolio backtest with a specific rebalancing frequency.
     *
     * @param config         Loaded macro_allocation.json
     * @param fredData       Historical FRED series (monthly, aligned by index)
     * @param assetPrices    Historical monthly close prices per asset class ticker
     * @param frequency      Rebalancing frequency: "m" (monthly), "q" (quarterly), "a" (annual)
     * @param initialCapital Starting capital (default $10,000)
     * @return MacroBacktestResult with performance metrics
     */
    static MacroBacktestResult run(const nlohmann::json&                                         config,
                                   const std::map<std::string, std::shared_ptr<FredSeriesInfo>>& fredData,
                                   const std::map<std::string, std::vector<double>>&             assetReturns,
                                   const std::vector<std::string>& dates, const std::string& frequency,
                                   double initialCapital = 10000.0);

    /**
     * @brief Compute benchmark (buy-and-hold) result from a single asset's returns.
     */
    static MacroBacktestResult computeBenchmark(const std::vector<double>&      monthlyReturns,
                                                const std::vector<std::string>& dates, const std::string& ticker,
                                                double initialCapital = 10000.0);

    /**
     * @brief Print comparison table for multiple backtest results.
     */
    static void printResults(const std::vector<MacroBacktestResult>& results, const MacroBacktestResult& benchmark);

   private:
    /**
     * @brief Check if this month index is a rebalancing point for the given frequency.
     */
    static bool isRebalancePoint(size_t monthIndex, const std::string& frequency);
};
