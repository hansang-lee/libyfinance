#pragma once

#include <cstddef>
#include <string>
#include <vector>

#include "strategy/istrategy.hpp"

/**
 * @brief RSI (Relative Strength Index) strategy.
 *
 * Generates BUY when RSI drops below the oversold threshold (default: 30),
 * and SELL when RSI rises above the overbought threshold (default: 70).
 */
class RsiStrategy: public IStrategy {
   public:
    /**
     * @param period     RSI lookback period (default: 14 days).
     * @param oversold   RSI threshold for BUY signal (default: 30).
     * @param overbought RSI threshold for SELL signal (default: 70).
     */
    explicit RsiStrategy(std::size_t period = 14, double oversold = 30.0, double overbought = 70.0);

    [[nodiscard]] std::string name() const override;

    void init(const StockInfo& data) override;

    [[nodiscard]] std::size_t warmupPeriod() const override;

    [[nodiscard]] Signal evaluate(const StockInfo& data, std::size_t index) override;

   private:
    std::size_t period_;
    double      oversold_;
    double      overbought_;

    // Cached RSI values. rsi_[i] corresponds to data index (period_ + i).
    std::vector<double> rsi_;
};
