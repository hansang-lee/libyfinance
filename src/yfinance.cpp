#include <ctime>
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

static time_t parseDateToTimestamp(const std::string& date) {
    std::tm tm = {};
    if (strptime(date.c_str(), "%Y-%m-%d", &tm) == nullptr) {
        return 0;
    }
    tm.tm_isdst = 0;
    return timegm(&tm);
}

std::shared_ptr<StockInfo> yFinance::getStockInfo(const std::string& ticker, const std::string& interval,
                                                  const std::string& range) {
    const auto fetched = fetch(std::string(url_base_) + ticker + "?interval=" + interval + "&range=" + range);
    if (fetched.empty()) {
        return nullptr;
    }

    const auto data = std::make_shared<StockInfo>();
    if (!data) {
        return nullptr;
    }

    try {
        const auto parsed = nlohmann::json::parse(fetched);
        if (!parsed.contains("chart") || !parsed["chart"].contains("result") || parsed["chart"]["result"].is_null()) {
            return nullptr;
        }

        const auto& result = parsed["chart"]["result"][0];
        const auto& meta   = result["meta"];

        data->ticker = ticker;

        if (meta.contains("currency")) {
            data->currency = meta["currency"];
        }

        if (meta.contains("exchangeName")) {
            data->exchangeName = meta["exchangeName"];
        }

        if (meta.contains("instrumentType")) {
            data->instrumentType = meta["instrumentType"];
        }

        if (meta.contains("regularMarketPrice")) {
            data->regularMarketPrice = meta["regularMarketPrice"];
        }

        if (meta.contains("chartPreviousClose")) {
            data->chartPreviousClose = meta["chartPreviousClose"];
        }

        if (meta.contains("firstTradeDate")) {
            data->firstTradeDate = meta["firstTradeDate"];
        }

        if (meta.contains("gmtoffset")) {
            data->gmtoffset = meta["gmtoffset"];
        }

        if (meta.contains("timezone")) {
            data->timezone = meta["timezone"];
        }

        if (result.contains("timestamp")) {
            data->timestamps = result["timestamp"].get<std::vector<int64_t>>();
        }

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
                data->volume = quote["volume"].get<std::vector<int64_t>>();
            }
        }
    } catch (const nlohmann::json::parse_error& e) {
        std::cerr << "JSON parse error: " << e.what() << std::endl;
    }

    return data;
}

std::shared_ptr<StockInfo> yFinance::getStockInfo(const std::string& ticker, const std::string& startDate,
                                                  const std::string& endDate, const std::string& interval) {
    auto p1 = parseDateToTimestamp(startDate);
    auto p2 = parseDateToTimestamp(endDate);

    if (p1 == -1 || p2 == -1) {
        return nullptr;
    }

    p2 += 86400;

    const std::string url = std::string(url_base_) + ticker + "?period1=" + std::to_string(p1)
                          + "&period2=" + std::to_string(p2) + "&interval=" + interval;

    const auto fetched = fetch(url);
    if (fetched.empty()) {
        return nullptr;
    }

    const auto data = std::make_shared<StockInfo>();
    if (!data) {
        return nullptr;
    }

    try {
        const auto parsed = nlohmann::json::parse(fetched);
        if (!parsed.contains("chart") || !parsed["chart"].contains("result") || parsed["chart"]["result"].is_null()) {
            return nullptr;
        }

        const auto& result = parsed["chart"]["result"][0];
        const auto& meta   = result["meta"];

        data->ticker = ticker;

        if (meta.contains("currency")) {
            data->currency = meta["currency"];
        }

        if (meta.contains("exchangeName")) {
            data->exchangeName = meta["exchangeName"];
        }

        if (meta.contains("instrumentType")) {
            data->instrumentType = meta["instrumentType"];
        }

        if (meta.contains("regularMarketPrice")) {
            data->regularMarketPrice = meta["regularMarketPrice"];
        }

        if (meta.contains("chartPreviousClose")) {
            data->chartPreviousClose = meta["chartPreviousClose"];
        }

        if (meta.contains("firstTradeDate")) {
            data->firstTradeDate = meta["firstTradeDate"];
        }

        if (meta.contains("gmtoffset")) {
            data->gmtoffset = meta["gmtoffset"];
        }

        if (meta.contains("timezone")) {
            data->timezone = meta["timezone"];
        }

        if (result.contains("timestamp")) {
            data->timestamps = result["timestamp"].get<std::vector<int64_t>>();
        }

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
                data->volume = quote["volume"].get<std::vector<int64_t>>();
            }
        }
    } catch (const nlohmann::json::parse_error& e) {
        std::cerr << "JSON parse error: " << e.what() << std::endl;
    }

    return data;
}

std::shared_ptr<FearAndGreedInfo> yFinance::getFearAndGreedIndex() {
    const auto fetched = fetch(std::string(cnn_url_base_), true);
    if (fetched.empty()) {
        return nullptr;
    }

    const auto data = std::make_shared<FearAndGreedInfo>();
    if (!data) {
        return nullptr;
    }

    try {
        const auto parsed = nlohmann::json::parse(fetched);
        if (!parsed.contains("fear_and_greed")) {
            return nullptr;
        }

        const auto& fng = parsed["fear_and_greed"];

        data->score         = fng.value("score", 0.0);
        data->rating        = fng.value("rating", "");
        data->timestamp     = fng.value("timestamp", "");
        data->previousClose = fng.value("previous_close", 0.0);
        data->previousWeek  = fng.value("previous_1_week", 0.0);
        data->previousMonth = fng.value("previous_1_month", 0.0);
        data->previousYear  = fng.value("previous_1_year", 0.0);

        if (parsed.contains("fear_and_greed_historical") && parsed["fear_and_greed_historical"].contains("data")) {
            for (const auto& item : parsed["fear_and_greed_historical"]["data"]) {
                data->timestamps.push_back(static_cast<int64_t>(item.value("x", 0.0) / 1000.0));
                data->scores.push_back(item.value("y", 0.0));
                data->ratings.push_back(item.value("rating", ""));
            }
        }
    } catch (const nlohmann::json::parse_error& e) {
        std::cerr << "JSON parse error: " << e.what() << std::endl;
        return nullptr;
    }

    return data;
}

std::shared_ptr<FredSeriesInfo> yFinance::getFredSeries(const std::string& seriesId, const std::string& apiKey,
                                                        const std::string& observationStart,
                                                        const std::string& observationEnd,
                                                        const std::string& frequency) {
    std::string baseUrl =
        std::string(fred_url_base_) + "?series_id=" + seriesId + "&api_key=" + apiKey + "&file_type=json";

    if (!observationStart.empty()) {
        baseUrl += "&observation_start=" + observationStart;
    }
    if (!observationEnd.empty()) {
        baseUrl += "&observation_end=" + observationEnd;
    }

    std::string url = baseUrl;
    if (!frequency.empty()) {
        url += "&frequency=" + frequency;
    }

    auto fetched = fetch(url);
    if (fetched.empty()) {
        return nullptr;
    }

    const auto data = std::make_shared<FredSeriesInfo>();
    if (!data) {
        return nullptr;
    }

    try {
        auto parsed = nlohmann::json::parse(fetched);

        /* Retry without frequency if the series doesn't support it */
        if (parsed.contains("error_code") && !frequency.empty()) {
            const auto msg = parsed.value("error_message", "");
            if (msg.find("frequency") != std::string::npos) {
                fetched = fetch(baseUrl);
                if (fetched.empty()) {
                    return nullptr;
                }
                parsed = nlohmann::json::parse(fetched);
            }
        }

        if (parsed.contains("error_code")) {
            std::cerr << "FRED API error: " << parsed.value("error_message", "Unknown error") << std::endl;
            return nullptr;
        }

        if (!parsed.contains("observations")) {
            return nullptr;
        }

        data->seriesId = seriesId;

        for (const auto& obs : parsed["observations"]) {
            const auto dateStr  = obs.value("date", "");
            const auto valueStr = obs.value("value", "");

            /* skip missing data marked as "." */
            if (valueStr == "." || valueStr.empty()) {
                continue;
            }

            data->dates.push_back(dateStr);
            data->values.push_back(std::stod(valueStr));
        }
    } catch (const nlohmann::json::parse_error& e) {
        std::cerr << "JSON parse error: " << e.what() << std::endl;
        return nullptr;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return nullptr;
    }

    return data;
}

std::string yFinance::fetch(const std::string& url, bool is_cnn) {
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
                         "Chrome/120.0.0.0 Safari/537.36");

        if (is_cnn) {
            struct curl_slist* headers = nullptr;
            headers = curl_slist_append(headers, "Referer: https://www.cnn.com/markets/fear-and-greed");
            headers = curl_slist_append(headers, "Accept: application/json");
            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

            res = curl_easy_perform(curl);
            curl_slist_free_all(headers);
        } else {
            res = curl_easy_perform(curl);
        }

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
