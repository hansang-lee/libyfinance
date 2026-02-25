#include <cstdlib>
#include <functional>
#include <iostream>
#include <string>

#include <unistd.h>

#include "macro_scorer.hpp"
#include "yfinance.hpp"

/**
 * @brief Resolve a path relative to the executable's directory.
 */
static std::string resolveFromExe(const std::string& relativePath) {
    char    buf[4096];
    ssize_t len = readlink("/proc/self/exe", buf, sizeof(buf) - 1);
    if (len <= 0) {
        return relativePath;
    }
    buf[len] = '\0';
    std::string exePath(buf);
    for (int i = 0; i < 4; ++i) {
        auto pos = exePath.rfind('/');
        if (pos == std::string::npos) {
            return relativePath;
        }
        exePath = exePath.substr(0, pos);
    }
    return exePath + "/" + relativePath;
}

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

int main(int argc, char* argv[]) {
    const char* apiKey = std::getenv("FRED_API_KEY");
    if (!apiKey || std::string(apiKey).empty()) {
        std::cerr << "Error: FRED_API_KEY environment variable is not set.\n"
                  << "Get your free API key at: https://fred.stlouisfed.org/docs/api/api_key.html\n"
                  << "Usage: export FRED_API_KEY=<your_key>" << std::endl;
        return 1;
    }

    std::string configPath = resolveFromExe("config/macro_allocation.json");
    if (argc > 1) {
        configPath = argv[1];
    }

    yFinance::init();
    Defer _cleanup([] { yFinance::close(); });

    if (!MacroScorer::analyze(apiKey, configPath)) {
        std::cerr << "Analysis failed." << std::endl;
        return 1;
    }

    return 0;
}
