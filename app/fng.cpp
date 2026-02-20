#include <functional>
#include <iomanip>
#include <iostream>

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
    std::strftime(mbstr, sizeof(mbstr), "%Y-%m-%d %H:%M", std::localtime(&t));
    return mbstr;
}

void printHeader() {
    // clang-format off
    std::clog << std::left
        << std::setw(20) << "(Date)"
        << std::setw(9) << "(Score)"
        << std::setw(15) << "(Rating)"
        << "\n-"
        << std::endl;
    // clang-format on
}

int main() {
    yFinance::init();
    Defer _cleanup([] { yFinance::close(); });

    const auto data = yFinance::getFearAndGreedIndex();
    if (!data) {
        return 1;
    }

    printHeader();

    for (std::size_t i = 0; i < data->timestamps.size(); i++) {
        // clang-format off
        std::clog << std::left
            << std::setw(20) << formatTime(data->timestamps[i])
            << std::fixed << std::setprecision(2)
            << std::setw(9) << data->scores[i]
            << std::setw(15) << data->ratings[i]
            << std::endl;
        // clang-format on
    }

    return 0;
}
