#pragma once

#include <string>
#include <vector>

struct FredSeriesInfo {
    /**
     * @brief FRED series identifier
     * @example "UNRATE", "FEDFUNDS"
     */
    std::string seriesId = "";

    /**
     * @brief Observation dates
     * @example ["2024-01-01", "2024-02-01", ...]
     */
    std::vector<std::string> dates;

    /**
     * @brief Observation values
     * @example [3.7, 3.9, ...]
     */
    std::vector<double> values;
};
