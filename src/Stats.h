//
// Created by broken_pc on 12/20/21.
//

#ifndef LP_STATS_H
#define LP_STATS_H

#include <cinttypes>
#include <chrono>

#include "json.hpp"

class Stats {

public:
    [[nodiscard]] uint64_t getTimeTook() const {
        return timeFinished - timeStarted;
    }

    [[nodiscard]] uint64_t getIPS() const {
        return iterations / getTimeTook();
    }

    nlohmann::json toJson();

    uint64_t timeStarted{};
    uint64_t timeFinished{};
    uint64_t iterations{};
    uint64_t memory{};
};


#endif //LP_STATS_H
