#include "rsi_strategy.hpp"

#include "indicator.hpp"

RsiStrategy::RsiStrategy(std::size_t period, double oversold, double overbought)
    : period_(period)
    , oversold_(oversold)
    , overbought_(overbought) {}

std::string RsiStrategy::name() const {
    return "RSI (" + std::to_string(period_) + ", " + std::to_string(static_cast<int>(oversold_)) + "/"
         + std::to_string(static_cast<int>(overbought_)) + ")";
}

void RsiStrategy::init(const StockInfo& data) {
    rsi_ = indicator::rsi(data.close, period_);
}

std::size_t RsiStrategy::warmupPeriod() const {
    return period_;
}

Signal RsiStrategy::evaluate(const StockInfo& /* data */, std::size_t index) {
    if (index <= period_) {
        return Signal::HOLD;
    }

    const auto rsiIdx = index - period_ - 1;
    if (rsiIdx >= rsi_.size()) {
        return Signal::HOLD;
    }

    const double val = rsi_[rsiIdx];

    // Oversold → BUY
    if (val <= oversold_) {
        return Signal::BUY;
    }

    // Overbought → SELL
    if (val >= overbought_) {
        return Signal::SELL;
    }

    return Signal::HOLD;
}
