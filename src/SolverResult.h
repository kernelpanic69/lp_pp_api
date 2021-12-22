//
// Created by broken_pc on 12/20/21.
//

#ifndef LP_SOLVERRESULT_H
#define LP_SOLVERRESULT_H

#include "Stats.h"

#include <map>
#include <string>

#include "json.hpp"

namespace lp_pp {
    class SolverResult {
    public:
        std::shared_ptr<nlohmann::json> to_json();

        void setObjective(double value) {
            obj_value = value;
        }

        void add_variable(const std::string &name, double value) {
            variables[name] = value;
        }

    private:
        std::string error;
        std::map<std::string, double> variables;
        double obj_value{};
        Stats stats;
    };
}

#endif //LP_SOLVERRESULT_H
