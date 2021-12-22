//
// Created by broken_pc on 12/20/21.
//

#ifndef LP_LPMODEL_H
#define LP_LPMODEL_H

#include <string>
#include <vector>
#include <memory>

#include "json.hpp"

struct glp_prob;

namespace lp_pp {

    enum OptType {
        MIN,
        MAX
    };

    enum EqType {
        LT, GT, FR, FX, UNKNOWN
    };

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

        [[nodiscard]] std::shared_ptr<nlohmann::json> to_json() const;

        [[nodiscard]] std::shared_ptr<glp_prob> get_glp_prob() const;

    private:
        std::string name;
        std::string obj_name;
        OptType type;

        size_t num_vars;
        size_t num_cons;

        std::vector<std::string> var_names;
        std::vector<double> objective;
        std::vector<bool> ints;

        std::vector<EqType> cons_types;
        std::vector<double> cons_values;
        std::vector<std::string> cons_names;
        std::vector<std::vector<double>> constraints;

        static EqType get_con_type(const std::string &t) {
            if (t == "lt") {
                return LT;
            } else if (t == "gt") {
                return GT;
            } else if (t == "fr") {
                return FR;
            } else if (t == "eq") {
                return FX;
            } else {
                return UNKNOWN;
            }
        }
    };
}

#endif //LP_LPMODEL_H
