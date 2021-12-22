//
// Created by broken_pc on 12/20/21.
//

#include "LpModel.h"
#include "lp_pp.h"

#include <memory>

#include <fmt/format.h>
#include <glpk.h>
#include <json.hpp>

using std::string;
using std::vector;
using std::runtime_error;
using std::make_shared;
using std::shared_ptr;
using std::make_unique;

using fmt::format;

using nlohmann::json;
namespace lp_pp {
    void LpModel::from_json(const nlohmann::json &json) {
        name = json.at("name").get<string>();
        const auto &c_type = json.at("type").get<string>();

        if (c_type == "max") {
            type = MAX;
        } else if (c_type == "min") {
            type = MIN;
        } else {
            throw runtime_error(R"(Unknown optimization type expected ("max"|"min"))");
        }

        obj_name = json.at("objName").get<string>();

        var_names = json.at("variables").get<vector<string>>();
        num_vars = var_names.size();

        for (size_t i = 0; i < var_names.size(); i++) {
            const auto &var_name = var_names[i];

            if (var_name.empty() || var_name.size() > 32) {
                throw runtime_error(format("Invalid variable name at: {}", i));
            }
        }

        objective = json.at("objective").get<vector<double>>();

        if (objective.size() != var_names.size()) {
            throw runtime_error("Objective coefficients size differs from amount of variables");
        }

        if (json.contains("ints")) {
            auto t_ints = json.at("ints").get<vector<int>>();

            if (t_ints.size() != num_vars) {
                throw runtime_error("Integer definitions size differs from amount of variables");
            }

            for (const auto &i: t_ints) {
                if (i != 0) {
                    ints.push_back(true);
                } else {
                    ints.push_back(false);
                }
            }
        }

        cons_names = json.at("consNames").get<vector<string>>();

        for (size_t i = 0; i < cons_names.size(); i++) {
            const auto &con_name = cons_names[i];

            if (con_name.empty() || con_name.size() > 32) {
                throw runtime_error(format("Invalid constraint name value at: {}", i));
            }
        }

        num_cons = cons_names.size();

        cons_values = json.at("consVals").get<vector<double>>();

        if (cons_values.size() != num_cons) {
            throw runtime_error("Constraints values size differs from amount of constraints");
        }

        const auto &c_types = json.at("consTypes").get<vector<string>>();

        if (c_types.size() != num_cons) {
            throw runtime_error("Constraints types size differs from amount of constraints");
        }

        for (size_t i = 0; i < num_cons; i++) {
            const auto &t = c_types[i];

            EqType eq = get_con_type(t);
            if (eq == UNKNOWN) {
                throw runtime_error(format("Invalid value for constraint type at: {}", i));
            }

            cons_types.push_back(eq);
        }

        constraints = json.at("constraints").get<vector<vector<double>>>();
        if (constraints.size() != num_cons) {
            throw runtime_error("Constraints coefficients row count differs from amount of constraints");
        }

        for (const auto &con: constraints) {
            if (con.size() != num_vars) {
                throw runtime_error("Constraint coefficients column size differs from amount of variables");
            }
        }
    }

    std::shared_ptr<json> LpModel::to_json() const {
        const json j = {
                {"name",        name},
                {"type",        type},
                {"objName",     obj_name},
                {"variables",   var_names},
                {"ints",        ints},
                {"objective",   objective},
                {"consNames",   cons_names},
                {"consValues",  cons_values},
                {"consTypes",   cons_types},
                {"constraints", constraints}
        };

        return make_shared<json>(j);
    }

    shared_ptr<glp_prob> LpModel::get_glp_prob() const {
        shared_ptr<glp_prob> prob(glp_create_prob(), glp_delete_prob);

        glp_set_prob_name(prob.get(), name.c_str());
        glp_set_obj_name(prob.get(), obj_name.c_str());

        glp_set_obj_dir(prob.get(), type == MAX ? 1 : -1);

        glp_add_cols(prob.get(), static_cast<int>(num_vars));

        for (int j = 1; j <= num_vars; j++) {
            glp_set_col_name(prob.get(), j, var_names[j - 1].c_str());
            glp_set_obj_coef(prob.get(), j, objective[j - 1]);
            glp_set_col_kind(prob.get(), j, ints[j] ? GLP_IV : GLP_CV);
            glp_set_col_bnds(prob.get(), j, GLP_LO, 0, 0);
        }

        glp_add_rows(prob.get(), static_cast<int>(num_cons));

        size_t total = num_cons * num_vars;

        auto ia = make_unique<int[]>(total + 1);
        auto ja = make_unique<int[]>(total + 1);
        auto ra = make_unique<double[]>(total + 1);

        size_t index = 1;

        for (int i = 1; i <= num_cons; i++) {
            glp_set_row_name(prob.get(), i, cons_names[i - 1].c_str());

            const auto &c_type = cons_types[i - 1];
            const auto c_bnd = cons_values[i - 1];

            int g_type = GLP_FR;
            double ub = 0;
            double lb = 0;

            if (c_type == LT) {
                g_type = GLP_UP;
                ub = c_bnd;
            } else if (c_type == GT) {
                g_type = GLP_LO;
                lb = c_bnd;
            } else {
                g_type = GLP_FX;
                lb = c_bnd;
            }

            glp_set_row_bnds(prob.get(), i, g_type, lb, ub);

            for (int j = 1; j <= num_vars; j++) {
                ia[index++] = i;
                ja[index] = j;
                ra[index] = constraints[i - 1][j - 1];
            }

            glp_load_matrix(prob.get(), static_cast<int>(total), ia.get(), ja.get(), ra.get());
        }

        return prob;
    }
}