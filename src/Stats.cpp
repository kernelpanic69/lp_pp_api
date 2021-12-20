//
// Created by broken_pc on 12/20/21.
//

#include "Stats.h"

using nlohmann::json;

json Stats::toJson() {
    json j;

    j["iterations"] = iterations;
    j["started"] = timeStarted;
    j["finished"] = timeFinished;
    j["took"] = getTimeTook();
    j["ips"] = getIPS();
    j["memory"] = memory;

    return j;
}
