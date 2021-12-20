//
// Created by broken_pc on 12/20/21.
//

#ifndef LP_LPMODEL_H
#define LP_LPMODEL_H

#include <string>
#include <vector>

#include "json.hpp"
#include <glpk.h>

namespace lp_pp {
    struct Variable {
        const size_t index;
        const std::string name;
        const bool is_integer;
        const double obj_value;
    };

    class LpModel {
    public:
        LpModel() = delete;

        void from_json(const nlohmann::json &json);

        std::shared_ptr<nlohmann::json> to_json();

        std::shared_ptr<glp_prob> get_glp_prob();

    private:
        std::string name;
        std::string obj_name;
        std::string type;

        size_t num_vars;
        size_t num_cons;

        std::vector<std::string> var_names;
        std::vector<double> objective;
        std::vector<bool> ints;

        std::vector<std::string> cons_types;
        std::vector<double> cons_values;
        std::vector<std::string> cons_names;
        std::vector<std::vector<double>> constraints;

        static bool is_valid_con_type(const std::string &t) {
            return t == "lt" || t == "gt" || t == "fr" || t == "eq";
        }
    };
}

#endif //LP_LPMODEL_H
