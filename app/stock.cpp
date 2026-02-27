#include <ctime>
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
        << std::setw(12) << "(Open)"
        << std::setw(12) << "(High)"
        << std::setw(12) << "(Low)"
        << std::setw(12) << "(Close)"
        << std::setw(15) << "(Volume)"
        << "\n-"
        << std::endl;
    // clang-format on
}

int main(int argc, char* argv[]) {
    yFinance::init();
    Defer _cleanup([] { yFinance::close(); });

    const auto TICKER   = ((argc > 1) ? argv[1] : "^IXIC");
    const auto START    = ((argc > 2) ? argv[2] : "2026-01-01");
    const auto END      = ((argc > 3) ? argv[3] : "2026-02-01");
    const auto INTERVAL = ((argc > 4) ? argv[4] : "1d");

    const auto data = yFinance::getStockInfo(TICKER, START, END, INTERVAL);
    if (!data) {
        return 1;
    }

    printHeader();

    for (std::size_t i = 0; i < data->timestamps.size(); i++) {
        // clang-format off
        std::clog << std::left
            << std::setw(20) << formatTime(data->timestamps[i])
            << std::fixed << std::setprecision(2)
            << std::setw(12) << data->open[i]
            << std::setw(12) << data->high[i]
            << std::setw(12) << data->low[i]
            << std::setw(12) << data->close[i]
            << std::setw(15) << data->volume[i]
            << std::endl;
        // clang-format on
    }

    return 0;
}
