//
// Created by kernelpanic on 12/20/21.
//

#include "SolverResult.h"

using std::make_shared;
using std::shared_ptr;

using nlohmann::json;

std::shared_ptr<nlohmann::json> lp_pp::SolverResult::to_json() {
    json j = {{"objective", obj_value},
              {"variables", variables},
              {"stats",     stats.toJson()}};

    return make_shared<json>(j);
}
