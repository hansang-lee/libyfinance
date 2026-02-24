#include "sma_crossover.hpp"

#include <algorithm>

#include "indicator.hpp"

SmaCrossover::SmaCrossover(std::size_t shortWindow, std::size_t longWindow)
    : shortWindow_(shortWindow)
    , longWindow_(longWindow) {}

std::string SmaCrossover::name() const {
    return "SMA Crossover (" + std::to_string(shortWindow_) + "/" + std::to_string(longWindow_) + ")";
}

void SmaCrossover::init(const StockInfo& data) {
    const auto& prices = data.close;

    auto rawShort = indicator::sma(prices, shortWindow_);
    auto rawLong  = indicator::sma(prices, longWindow_);

    // rawShort starts at index (shortWindow_ - 1) of original data.
    // rawLong  starts at index (longWindow_  - 1) of original data.
    // We align both to start at index (longWindow_ - 1).
    // rawShort needs to be trimmed from the front by (longWindow_ - shortWindow_) elements.

    const std::size_t offset = longWindow_ - shortWindow_;
    if (offset < rawShort.size()) {
        shortSma_.assign(rawShort.begin() + static_cast<long>(offset), rawShort.end());
    }
    longSma_ = std::move(rawLong);

    // Ensure both are the same length
    const auto minLen = std::min(shortSma_.size(), longSma_.size());
    shortSma_.resize(minLen);
    longSma_.resize(minLen);
}

std::size_t SmaCrossover::warmupPeriod() const {
    // Need at least longWindow_ data points, plus 1 extra for crossover detection
    return longWindow_;
}

Signal SmaCrossover::evaluate(const StockInfo& /* data */, std::size_t index) {
    // Map data index to our aligned SMA index
    if (index < longWindow_) {
        return Signal::HOLD;
    }

    const auto smaIdx = index - longWindow_;

    // Need at least 2 points for crossover detection
    if (smaIdx == 0 || smaIdx >= shortSma_.size()) {
        return Signal::HOLD;
    }

    const double prevShort = shortSma_[smaIdx - 1];
    const double prevLong  = longSma_[smaIdx - 1];
    const double currShort = shortSma_[smaIdx];
    const double currLong  = longSma_[smaIdx];

    // Golden cross: short SMA crosses above long SMA
    if (prevShort <= prevLong && currShort > currLong) {
        return Signal::BUY;
    }

    // Death cross: short SMA crosses below long SMA
    if (prevShort >= prevLong && currShort < currLong) {
        return Signal::SELL;
    }

    return Signal::HOLD;
}
