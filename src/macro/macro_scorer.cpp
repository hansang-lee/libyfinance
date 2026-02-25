#include <algorithm>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <iostream>

#include <nlohmann/json.hpp>

#include "macro_scorer.hpp"
#include "yfinance.hpp"

double MacroScorer::clamp(double value) {
    return std::max(0.0, std::min(100.0, value));
}

double MacroScorer::rateOfChange(const std::shared_ptr<FredSeriesInfo>& series) {
    if (!series || series->values.size() < 2) {
        return 0.0;
    }
    const auto n = series->values.size();
    return series->values[n - 1] - series->values[n - 2];
}

double MacroScorer::latestValue(const std::shared_ptr<FredSeriesInfo>& series) {
    if (!series || series->values.empty()) {
        return 0.0;
    }
    return series->values.back();
}

MacroScores MacroScorer::computeScores(const std::map<std::string, std::shared_ptr<FredSeriesInfo>>& data,
                                       const std::shared_ptr<FearAndGreedInfo>&                      fng) {
    MacroScores scores;

    /* ===== GROWTH SCORE =====
     * UNRATE:  lower & declining = stronger economy → higher score
     * PAYEMS:  increasing = stronger economy → higher score
     * INDPRO:  increasing = stronger economy → higher score
     */
    {
        double s = 50.0;

        if (data.count("UNRATE")) {
            double roc = rateOfChange(data.at("UNRATE"));
            double lvl = latestValue(data.at("UNRATE"));
            // Negative change in unemployment is good (+10 per -0.1pp)
            s += (-roc) * 100.0;
            // Low unemployment level is good (below 4% = bonus, above 6% = penalty)
            s += (5.0 - lvl) * 5.0;
        }

        if (data.count("PAYEMS")) {
            double roc = rateOfChange(data.at("PAYEMS"));
            // Positive job growth is good (normalized, ~200k/mo is healthy)
            s += (roc / 200.0) * 10.0;
        }

        if (data.count("INDPRO")) {
            double roc = rateOfChange(data.at("INDPRO"));
            // Positive industrial production growth is good
            s += roc * 5.0;
        }

        scores.growth = clamp(s);
    }

    /* ===== INFLATION SCORE =====
     * Higher score = higher inflation pressure
     * CPIAUCSL/CPILFESL/PCEPI: month-over-month change
     */
    {
        double s = 50.0;

        if (data.count("CPIAUCSL")) {
            double roc    = rateOfChange(data.at("CPIAUCSL"));
            double pctChg = (roc / latestValue(data.at("CPIAUCSL"))) * 100.0 * 12.0;  // annualized
            // 2% = neutral, above = inflationary
            s += (pctChg - 2.0) * 10.0;
        }

        if (data.count("CPILFESL")) {
            double roc    = rateOfChange(data.at("CPILFESL"));
            double pctChg = (roc / latestValue(data.at("CPILFESL"))) * 100.0 * 12.0;
            s += (pctChg - 2.0) * 8.0;
        }

        if (data.count("PCEPI")) {
            double roc    = rateOfChange(data.at("PCEPI"));
            double pctChg = (roc / latestValue(data.at("PCEPI"))) * 100.0 * 12.0;
            s += (pctChg - 2.0) * 7.0;
        }

        scores.inflation = clamp(s);
    }

    /* ===== LIQUIDITY SCORE =====
     * Higher score = more liquidity (easier monetary conditions)
     * M2REAL: growing = more liquidity
     * WM2NS:  growing = more liquidity
     * FEDFUNDS: lower = more liquidity
     */
    {
        double s = 50.0;

        if (data.count("M2REAL")) {
            double roc    = rateOfChange(data.at("M2REAL"));
            double pctChg = (roc / latestValue(data.at("M2REAL"))) * 100.0;
            s += pctChg * 30.0;
        }

        if (data.count("WM2NS")) {
            double roc    = rateOfChange(data.at("WM2NS"));
            double pctChg = (roc / latestValue(data.at("WM2NS"))) * 100.0;
            s += pctChg * 20.0;
        }

        if (data.count("FEDFUNDS")) {
            double rate = latestValue(data.at("FEDFUNDS"));
            // Lower rates = more liquidity (0% = max, 6%+ = min)
            s += (3.0 - rate) * 5.0;
        }

        scores.liquidity = clamp(s);
    }

    /* ===== SENTIMENT SCORE =====
     * FNG:     0-100 directly
     * UMCSENT: typically 50-110, normalize to 0-100
     */
    {
        double s = 50.0;
        int    n = 0;

        if (fng) {
            s = fng->score;
            n++;
        }

        if (data.count("UMCSENT")) {
            double val   = latestValue(data.at("UMCSENT"));
            double normd = clamp((val - 50.0) * (100.0 / 60.0));
            if (n > 0) {
                s = (s + normd) / 2.0;
            } else {
                s = normd;
            }
        }

        scores.sentiment = clamp(s);
    }

    /* ===== RISK SCORE =====
     * Higher score = more risk in the environment
     * T10Y2Y:        negative (inverted) = recession signal = high risk
     * BAMLH0A0HYM2:  higher spread = more credit risk
     */
    {
        double s = 50.0;

        if (data.count("T10Y2Y")) {
            double spread = latestValue(data.at("T10Y2Y"));
            // Negative spread = inverted yield curve = high risk
            // +2.0 = very safe (-20), -0.5 = dangerous (+25)
            s += (-spread) * 10.0;
        }

        if (data.count("BAMLH0A0HYM2")) {
            double spread = latestValue(data.at("BAMLH0A0HYM2"));
            // Normal ~3-4%, above 5% = elevated risk
            s += (spread - 4.0) * 8.0;
        }

        scores.risk = clamp(s);
    }

    return scores;
}

Regime MacroScorer::detectRegime(const MacroScores& scores, const nlohmann::json& config) {
    const auto& thresholds = config["regime_thresholds"];

    // Overheating: decent growth but high inflation
    if (scores.growth >= thresholds["overheating"].value("composite_min", 45.0)
        && scores.inflation >= thresholds["overheating"].value("inflation_min", 65.0)) {
        return Regime::Overheating;
    }

    // Expansion: high composite, moderate inflation
    if (scores.growth >= thresholds["expansion"].value("composite_min", 60.0)
        && scores.inflation < thresholds["expansion"].value("inflation_max", 65.0)) {
        return Regime::Expansion;
    }

    // Recession: low growth or high risk
    if (scores.growth < thresholds["slowdown"].value("composite_min", 25.0) || scores.risk >= 70.0) {
        return Regime::Recession;
    }

    // Default: Slowdown
    return Regime::Slowdown;
}

Allocation MacroScorer::getAllocation(Regime regime, const nlohmann::json& config) {
    std::string regimeKey;
    switch (regime) {
    case Regime::Expansion:
        regimeKey = "expansion";
        break;
    case Regime::Overheating:
        regimeKey = "overheating";
        break;
    case Regime::Slowdown:
        regimeKey = "slowdown";
        break;
    case Regime::Recession:
        regimeKey = "recession";
        break;
    }

    Allocation alloc;
    if (config.contains("allocation") && config["allocation"].contains(regimeKey)) {
        const auto& a = config["allocation"][regimeKey];
        alloc.stocks  = a.value("stocks", 0.0);
        alloc.gold    = a.value("gold", 0.0);
        alloc.metals  = a.value("metals", 0.0);
        alloc.bonds   = a.value("bonds", 0.0);
        alloc.usd     = a.value("usd", 0.0);
        alloc.krw     = a.value("krw", 0.0);
    }

    return alloc;
}

std::string MacroScorer::regimeToString(Regime regime) {
    switch (regime) {
    case Regime::Expansion:
        return "EXPANSION";
    case Regime::Overheating:
        return "OVERHEATING";
    case Regime::Slowdown:
        return "SLOWDOWN";
    case Regime::Recession:
        return "RECESSION";
    }
    return "UNKNOWN";
}

bool MacroScorer::analyze(const std::string& apiKey, const std::string& configPath) {
    /* Load config */
    nlohmann::json config;
    {
        std::ifstream f(configPath);
        if (!f.is_open()) {
            std::cerr << "Error: Cannot open config file: " << configPath << std::endl;
            return false;
        }
        try {
            f >> config;
        } catch (const nlohmann::json::parse_error& e) {
            std::cerr << "Config parse error: " << e.what() << std::endl;
            return false;
        }
    }

    /* Fetch FRED data */
    // clang-format off
    const std::vector<std::string> seriesIds = {
        "UNRATE", "PAYEMS", "INDPRO",                     // Growth
        "CPIAUCSL", "CPILFESL", "PCEPI",                  // Inflation
        "M2REAL", "WM2NS", "FEDFUNDS",                    // Liquidity
        "UMCSENT",                                         // Sentiment
        "T10Y2Y", "BAMLH0A0HYM2"                          // Risk
    };
    // clang-format on

    std::cerr << "Fetching FRED data..." << std::endl;

    std::map<std::string, std::shared_ptr<FredSeriesInfo>> fredData;
    for (const auto& id : seriesIds) {
        auto result = yFinance::getFredSeries(id, apiKey, "", "", "m");
        if (result && !result->values.empty()) {
            fredData[id] = result;
            std::cerr << "  [OK] " << id << " (" << result->values.size() << " observations)" << std::endl;
        } else {
            std::cerr << "  [WARN] " << id << " - no data" << std::endl;
        }
    }

    /* Fetch FNG data */
    std::cerr << "Fetching Fear & Greed Index..." << std::endl;
    auto fngData = yFinance::getFearAndGreedIndex();
    if (fngData) {
        std::cerr << "  [OK] FNG score: " << fngData->score << " (" << fngData->rating << ")" << std::endl;
    } else {
        std::cerr << "  [WARN] FNG - no data" << std::endl;
    }

    /* Compute scores */
    auto scores = computeScores(fredData, fngData);

    /* Apply weights */
    if (config.contains("scoring_weights")) {
        const auto& w    = config["scoring_weights"];
        scores.composite = scores.growth * w.value("growth", 0.25)
                         + (100.0 - scores.inflation) * w.value("inflation", 0.20)
                         + scores.liquidity * w.value("liquidity", 0.20) + scores.sentiment * w.value("sentiment", 0.15)
                         + (100.0 - scores.risk) * w.value("risk", 0.20);
    }

    /* Detect regime */
    auto regime = detectRegime(scores, config);

    /* Get allocation */
    auto alloc = getAllocation(regime, config);

    /* Print results */
    // clang-format off
    std::clog << std::endl;
    std::clog << "=== Macro Regime Analysis ===" << std::endl;
    std::clog << std::endl;
    std::clog << std::left << std::setw(20) << "Category"
              << std::right << std::setw(10) << "Score" << std::endl;
    std::clog << std::string(30, '-') << std::endl;
    std::clog << std::left << std::setw(20) << "Growth"
              << std::right << std::fixed << std::setprecision(1) << std::setw(10) << scores.growth << std::endl;
    std::clog << std::left << std::setw(20) << "Inflation"
              << std::right << std::setw(10) << scores.inflation << std::endl;
    std::clog << std::left << std::setw(20) << "Liquidity"
              << std::right << std::setw(10) << scores.liquidity << std::endl;
    std::clog << std::left << std::setw(20) << "Sentiment"
              << std::right << std::setw(10) << scores.sentiment << std::endl;
    std::clog << std::left << std::setw(20) << "Risk"
              << std::right << std::setw(10) << scores.risk << std::endl;
    std::clog << std::string(30, '-') << std::endl;
    std::clog << std::left << std::setw(20) << "Composite"
              << std::right << std::setw(10) << scores.composite << std::endl;
    std::clog << std::endl;
    std::clog << "Current Regime: " << regimeToString(regime) << std::endl;

    std::clog << std::endl;
    std::clog << "=== Recommended Allocation ===" << std::endl;
    std::clog << std::endl;
    std::clog << std::left << std::setw(20) << "Asset"
              << std::right << std::setw(10) << "Weight" << std::endl;
    std::clog << std::string(30, '-') << std::endl;
    std::clog << std::left << std::setw(20) << "Stocks"
              << std::right << std::setw(9) << static_cast<int>(alloc.stocks) << "%" << std::endl;
    std::clog << std::left << std::setw(20) << "Gold"
              << std::right << std::setw(9) << static_cast<int>(alloc.gold) << "%" << std::endl;
    std::clog << std::left << std::setw(20) << "Metals (Ag/Cu)"
              << std::right << std::setw(9) << static_cast<int>(alloc.metals) << "%" << std::endl;
    std::clog << std::left << std::setw(20) << "Bonds (US Treasury)"
              << std::right << std::setw(9) << static_cast<int>(alloc.bonds) << "%" << std::endl;
    std::clog << std::left << std::setw(20) << "USD"
              << std::right << std::setw(9) << static_cast<int>(alloc.usd) << "%" << std::endl;
    std::clog << std::left << std::setw(20) << "KRW"
              << std::right << std::setw(9) << static_cast<int>(alloc.krw) << "%" << std::endl;
    // clang-format on

    return true;
}
