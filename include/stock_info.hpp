#pragma once

#include <string>
#include <vector>

struct StockInfo {
    /**
     * @brief
     * @example "AAPL", "GOOGL", etc.
     */
    std::string ticker = "";

    /**
     * @brief
     * @example "USD", "KRW", etc.
     */
    std::string currency = "";

    /**
     * @brief
     * @example "NASDAQ", "NYSE", etc.
     */
    std::string exchangeName = "";

    /**
     * @brief
     * @example "COMMON_STOCK", "ETF", etc.
     */
    std::string instrumentType = "";

    /**
     * @brief
     * @example "America/New_York", "Asia/Seoul", etc.
     */
    std::string timezone = "";

    /**
     * @brief
     * @example 100.0
     */
    double regularMarketPrice = 0.0;

    /**
     * @brief
     * @example 100.0
     */
    double chartPreviousClose = 0.0;

    /**
     * @brief
     * @example 100.0
     */
    int64_t firstTradeDate = 0;

    /**
     * @brief
     * @example 100.0
     */
    int32_t gmtoffset = 0;

    /* HISTORICAL DATA */

    /**
     * @brief
     * @example [1705641600, 1705728000, ...]
     */
    std::vector<int64_t> timestamps;

    /**
     * @brief
     * @example [100.0, 101.0, ...]
     */
    std::vector<double> open;

    /**
     * @brief
     * @example [100.0, 101.0, ...]
     */
    std::vector<double> high;

    /**
     * @brief
     * @example [100.0, 101.0, ...]
     */
    std::vector<double> low;

    /**
     * @brief
     * @example [100.0, 101.0, ...]
     */
    std::vector<double> close;

    /**
     * @brief
     * @example [100.0, 101.0, ...]
     */
    std::vector<int64_t> volume;
};
