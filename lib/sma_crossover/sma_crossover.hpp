#pragma once

#include <cstddef>
#include <string>
#include <vector>

#include "strategy/istrategy.hpp"

/**
 * @brief SMA Crossover strategy.
 *
 * Generates BUY when short SMA crosses above long SMA (golden cross),
 * and SELL when short SMA crosses below long SMA (death cross).
 */
class SmaCrossover: public IStrategy {
   public:
    /**
     * @param shortWindow Short-term SMA window (default: 20 days).
     * @param longWindow  Long-term SMA window (default: 50 days).
     */
    explicit SmaCrossover(std::size_t shortWindow = 20, std::size_t longWindow = 50);

    [[nodiscard]] std::string name() const override;

    void init(const StockInfo& data) override;

    [[nodiscard]] std::size_t warmupPeriod() const override;

    [[nodiscard]] Signal evaluate(const StockInfo& data, std::size_t index) override;

   private:
    std::size_t shortWindow_;
    std::size_t longWindow_;

    // Cached SMA values aligned to data indices.
    // shortSma_[i] and longSma_[i] correspond to data index (longWindow_ - 1 + i).
    std::vector<double> shortSma_;
    std::vector<double> longSma_;
};
