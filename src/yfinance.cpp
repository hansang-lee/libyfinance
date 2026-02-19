#include <iostream>

#include <curl/curl.h>
#include <nlohmann/json.hpp>

#include "yfinance.hpp"

void yFinance::init() {
    curl_global_init(CURL_GLOBAL_DEFAULT);
}

void yFinance::close() {
    curl_global_cleanup();
}

std::shared_ptr<StockData> yFinance::getStockInfo(const std::string& ticker, const std::string& interval,
                                                  const std::string& range) {
    const auto fetched = fetch(std::string(url_base_) + ticker + "?interval=" + interval + "&range=" + range);
    if (fetched.empty()) {
        return nullptr;
    }

    auto data = std::make_shared<StockData>();

    try {
        const auto parsed = nlohmann::json::parse(fetched);
        if (!parsed.contains("chart") || !parsed["chart"].contains("result") || parsed["chart"]["result"].is_null()) {
            return nullptr;
        }

        const auto& result = parsed["chart"]["result"][0];
        const auto& meta   = result["meta"];

        data->ticker = ticker;

        /**
         * @note CURRENCY
         * @example "USD", "KRW", etc.
         */
        if (meta.contains("currency") && !meta["currency"].is_null()) {
            data->currency = meta["currency"];
        }

        /**
         * @note EXCHANGE-NAME
         * @example "NMS", "NYE", "KSC", etc.
         */
        if (meta.contains("exchangeName") && !meta["exchangeName"].is_null()) {
            data->exchangeName = meta["exchangeName"];
        }

        /**
         * @note INSTRUMENT-TYPE
         * @example "EQUITY", "ETF", "INDEX", etc.
         */
        if (meta.contains("instrumentType")) {
            data->instrumentType = meta["instrumentType"];
        }

        /**
         * @note REGULAR-MARKET-PRICE
         * @example 264.35 (Double)
         */
        if (meta.contains("regularMarketPrice")) {
            data->regularMarketPrice = meta["regularMarketPrice"];
        }

        /**
         * @note CHART-PREVIOUS-CLOSE
         * @example 263.88 (Double)
         */
        if (meta.contains("chartPreviousClose")) {
            data->chartPreviousClose = meta["chartPreviousClose"];
        }

        /**
         * @note FIRST-TRADE-DATE
         * @example 345479400 (Unix timestamp)
         */
        if (meta.contains("firstTradeDate")) {
            data->firstTradeDate = meta["firstTradeDate"];
        }

        /**
         * @note GMT-OFFSET
         * @example -18000 (Seconds)
         */
        if (meta.contains("gmtoffset")) {
            data->gmtoffset = meta["gmtoffset"];
        }

        /**
         * @note TIMEZONE
         * @example "EST", "KST", etc.
         */
        if (meta.contains("timezone")) {
            data->timezone = meta["timezone"];
        }

        /**
         * @note TIMESTAMP
         * @example [1708324800, 1708328400, ...]
         */
        if (result.contains("timestamp")) {
            data->timestamps = result["timestamp"].get<std::vector<long long>>();
        }

        /**
         * @note QUOTE
         * @example {
         *  "open": [264.35, 264.35, 264.35, 264.35, 264.35],
         *  "high": [264.35, 264.35, 264.35, 264.35, 264.35],
         *  "low": [264.35, 264.35, 264.35, 264.35, 264.35],
         *  "close": [264.35, 264.35, 264.35, 264.35, 264.35],
         *  "volume": [264.35, 264.35, 264.35, 264.35, 264.35]
         * }
         */
        if (result.contains("indicators") && result["indicators"].contains("quote")) {
            const auto& quote = result["indicators"]["quote"][0];
            if (quote.contains("open")) {
                data->open = quote["open"].get<std::vector<double>>();
            }
            if (quote.contains("high")) {
                data->high = quote["high"].get<std::vector<double>>();
            }
            if (quote.contains("low")) {
                data->low = quote["low"].get<std::vector<double>>();
            }
            if (quote.contains("close")) {
                data->close = quote["close"].get<std::vector<double>>();
            }
            if (quote.contains("volume")) {
                data->volume = quote["volume"].get<std::vector<long long>>();
            }
        }
    } catch (const nlohmann::json::parse_error& e) {
        std::cerr << e.what() << std::endl;
    }

    return data;
}

std::string yFinance::fetch(const std::string& url) {
    CURL*    curl = nullptr;
    CURLcode res  = CURLE_OK;

    std::string buffer("");

    curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buffer);
        curl_easy_setopt(curl, CURLOPT_USERAGENT,
                         "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) "
                         "Chrome/58.0.3029.110 Safari/537.3");

        res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
        }
        curl_easy_cleanup(curl);
    }
    return buffer;
}

std::size_t yFinance::write(void* contents, std::size_t size, std::size_t nmemb, void* userp) {
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}
