#pragma once

#include <memory>
#include <string>
#include <string_view>
#include <vector>

struct StockData {
    std::string ticker             = "";
    std::string currency           = "";
    std::string exchangeName       = "";
    std::string instrumentType     = "";
    std::string timezone           = "";
    double      regularMarketPrice = 0.0;
    double      chartPreviousClose = 0.0;
    long long   firstTradeDate     = 0;
    int         gmtoffset          = 0;

    std::vector<long long> timestamps;
    std::vector<double>    open;
    std::vector<double>    high;
    std::vector<double>    low;
    std::vector<double>    close;
    std::vector<long long> volume;
};

class yFinance {
   public:
    static void init();
    static void close();

    yFinance()  = delete;
    ~yFinance() = delete;

    yFinance(const yFinance& other) = delete;
    yFinance(yFinance&& other)      = delete;

    yFinance& operator=(const yFinance& other) = delete;
    yFinance& operator=(yFinance&& other) = delete;

    /**
     * @brief Fetch historical stock data.
     * @param ticker Stock ticker (e.g., "AAPL")
     * @param interval Data interval (e.g., "1d", "1wk", "1mo")
     * @param range Data range (e.g., "1y", "5y", "max")
     * @return StockData containing historical time-series
     */
    [[nodiscard]] static std::shared_ptr<StockData>
    getStockInfo(const std::string& ticker, const std::string& interval = "1d", const std::string& range = "1mo");

   private:
    static constexpr std::string_view url_base_ = "https://query1.finance.yahoo.com/v8/finance/chart/";

    [[nodiscard]] static std::string fetch(const std::string& url);

    static std::size_t write(void* contents, std::size_t size, std::size_t nmemb, void* userp);
};
