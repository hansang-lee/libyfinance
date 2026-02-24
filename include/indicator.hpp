#pragma once

#include <cstddef>
#include <vector>

namespace indicator {

/**
 * @brief Compute Simple Moving Average (SMA).
 * @param prices  Input price series.
 * @param window  Window size for the moving average.
 * @return        SMA values. Size = prices.size() - window + 1.
 *                An empty vector is returned if prices.size() < window.
 */
[[nodiscard]] inline std::vector<double> sma(const std::vector<double>& prices, std::size_t window) {
    if (window == 0 || prices.size() < window) {
        return {};
    }

    std::vector<double> result;
    result.reserve(prices.size() - window + 1);

    double sum = 0.0;
    for (std::size_t i = 0; i < window; ++i) {
        sum += prices[i];
    }
    result.push_back(sum / static_cast<double>(window));

    for (std::size_t i = window; i < prices.size(); ++i) {
        sum += prices[i] - prices[i - window];
        result.push_back(sum / static_cast<double>(window));
    }

    return result;
}

/**
 * @brief Compute Relative Strength Index (RSI).
 * @param prices  Input price series.
 * @param period  Lookback period (typically 14).
 * @return        RSI values (0~100). Size = prices.size() - period.
 *                An empty vector is returned if prices.size() <= period.
 *
 * Uses Wilder's smoothing method (exponential moving average of gains/losses).
 */
[[nodiscard]] inline std::vector<double> rsi(const std::vector<double>& prices, std::size_t period) {
    if (period == 0 || prices.size() <= period) {
        return {};
    }

    std::vector<double> result;
    result.reserve(prices.size() - period);

    // Calculate initial average gain/loss over the first 'period' changes
    double avgGain = 0.0;
    double avgLoss = 0.0;
    for (std::size_t i = 1; i <= period; ++i) {
        const double change = prices[i] - prices[i - 1];
        if (change > 0.0) {
            avgGain += change;
        } else {
            avgLoss += -change;
        }
    }
    avgGain /= static_cast<double>(period);
    avgLoss /= static_cast<double>(period);

    // First RSI value
    if (avgLoss < 1e-12) {
        result.push_back(100.0);
    } else {
        const double rs = avgGain / avgLoss;
        result.push_back(100.0 - (100.0 / (1.0 + rs)));
    }

    // Subsequent values using Wilder's smoothing
    const double smooth = static_cast<double>(period - 1) / static_cast<double>(period);
    const double inv    = 1.0 / static_cast<double>(period);

    for (std::size_t i = period + 1; i < prices.size(); ++i) {
        const double change = prices[i] - prices[i - 1];
        if (change > 0.0) {
            avgGain = avgGain * smooth + change * inv;
            avgLoss = avgLoss * smooth;
        } else {
            avgGain = avgGain * smooth;
            avgLoss = avgLoss * smooth + (-change) * inv;
        }

        if (avgLoss < 1e-12) {
            result.push_back(100.0);
        } else {
            const double rs = avgGain / avgLoss;
            result.push_back(100.0 - (100.0 / (1.0 + rs)));
        }
    }

    return result;
}

}  // namespace indicator
