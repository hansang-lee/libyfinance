#pragma once

#include <map>
#include <memory>
#include <string>
#include <vector>

#include <nlohmann/json.hpp>

#include "fng_info.hpp"
#include "fred_info.hpp"

struct MacroScores {
    double growth    = 0.0;  // 0-100 (high = strong economy)
    double inflation = 0.0;  // 0-100 (high = high inflation)
    double liquidity = 0.0;  // 0-100 (high = loose monetary policy)
    double sentiment = 0.0;  // 0-100 (high = bullish)
    double risk      = 0.0;  // 0-100 (high = risky environment)
    double composite = 0.0;  // weighted total
};

enum class Regime
{
    Expansion,    // strong growth, moderate inflation
    Overheating,  // strong growth, high inflation
    Slowdown,     // weakening growth, elevated inflation
    Recession     // weak growth, elevated risk
};

struct Allocation {
    double stocks = 0.0;
    double gold   = 0.0;
    double metals = 0.0;
    double bonds  = 0.0;
    double usd    = 0.0;
    double krw    = 0.0;
};

class MacroScorer {
   public:
    /**
     * @brief Run full macro analysis: fetch data, score, detect regime, recommend allocation.
     * @param apiKey FRED API key
     * @param configPath Path to macro_allocation.json
     */
    static bool analyze(const std::string& apiKey, const std::string& configPath);

    [[nodiscard]] static std::string regimeToString(Regime regime);

   private:
    /**
     * @brief Compute macro category scores from FRED + FNG data.
     */
    static MacroScores computeScores(const std::map<std::string, std::shared_ptr<FredSeriesInfo>>& fredData,
                                     const std::shared_ptr<FearAndGreedInfo>&                      fngData);

    /**
     * @brief Detect the current economic regime from scores.
     */
    static Regime detectRegime(const MacroScores& scores, const nlohmann::json& config);

    /**
     * @brief Look up allocation from config for given regime.
     */
    static Allocation getAllocation(Regime regime, const nlohmann::json& config);

    /**
     * @brief Helper: compute rate of change between last two values of a series.
     * @return change in percentage points (e.g., 4.0 -> 4.3 = +0.3)
     */
    static double rateOfChange(const std::shared_ptr<FredSeriesInfo>& series);

    /**
     * @brief Helper: get latest value of a series.
     */
    static double latestValue(const std::shared_ptr<FredSeriesInfo>& series);

    /**
     * @brief Clamp value to 0-100 range.
     */
    static double clamp(double value);
};
