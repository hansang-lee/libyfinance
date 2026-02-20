#pragma once

#include <string>
#include <vector>

struct FearAndGreedInfo {
    /**
     * @brief
     * @example
     *  "Extreme Fear"
     *  "Fear"
     *  "Neutral"
     *  "Greed"
     *  "Extreme Greed"
     */
    std::string rating = "";

    /**
     * @brief
     * @example "2026-02-20 01:00:00"
     */
    std::string timestamp = "";

    /**
     * @brief
     * @example 47.0 [0.0, 100.0]
     */
    double score = 0.0;

    /**
     * @brief
     * @example 47.0 [0.0, 100.0]
     */
    double previousClose = 0.0;

    /**
     * @brief
     * @example 47.0 [0.0, 100.0]
     */
    double previousWeek = 0.0;

    /**
     * @brief
     * @example 47.0 [0.0, 100.0]
     */
    double previousMonth = 0.0;

    /**
     * @brief
     * @example 47.0 [0.0, 100.0]
     */
    double previousYear = 0.0;

    /* HISTORICAL DATA */

    /**
     * @brief
     * @example [1705641600, 1705728000, ...]
     */
    std::vector<int64_t> timestamps;

    /**
     * @brief
     * @example [47.0, 48.0, ...]
     */
    std::vector<double> scores;

    /**
     * @brief
     * @example ["Neutral", "Fear", ...]
     */
    std::vector<std::string> ratings;
};
