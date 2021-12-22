//
// Created by broken_pc on 12/21/21.
//

#include "GomorySolver.h"
#include <glpk.h>

namespace lp_pp {
    std::shared_ptr<SolverResult> GomorySolver::solve(const lp_pp::LpModel &model) {
// Initial relaxed problem
        glp_prob *opt = model.get_glp_prob().get();
        double upperBound = glp_get_obj_val(opt);
        double lowerBound = 0;

        // find lower bound (Evaluate objective with rounded down variables)
        for (size_t j = 1; j <= glp_get_num_cols(opt); j++) {
            double coef = glp_get_obj_coef(opt, j);
            double var = glp_get_col_prim(opt, j);

            lowerBound += coef * std::floor(var);
        }

        while (true) {
            // find variable with biggest fractional part
            double targetIndex = maxFracIndex(opt);

            if (targetIndex == 0) {
                // No fractional variables left. Cleanup and exit;
//                return opt;
            } else {
                // Initialize branches
                glp_prob *left = glp_create_prob();
                glp_prob *right = glp_create_prob();

                // Copy current optimum to branches
                glp_copy_prob(left, opt, GLP_ON);
                glp_copy_prob(right, opt, GLP_ON);

                // get target variable value
                double varVal = glp_get_col_prim(opt, targetIndex);

                // Evaluate constraints values
                double lBnd = std::floor(varVal);
                double rBnd = std::ceil(varVal);

                // add new constraints to nodes
                glp_add_rows(left, 1);
                glp_set_row_bnds(left, glp_get_num_rows(left), GLP_UP, 0, lBnd);

                const size_t numCols = glp_get_num_cols(left);

                glp_add_rows(right, 1);
                glp_set_row_bnds(right, glp_get_num_rows(right), GLP_LO, rBnd, 0);

                double *v = new double[numCols + 1];
                int *ind = new int[numCols + 1];

                for (size_t j = 0; j <= numCols; j++) {
                    v[j] = (j == targetIndex) ? 1 : 0;
                    ind[j] = j;
                }

                glp_set_mat_row(left, glp_get_num_rows(left), numCols, ind, v);
                glp_set_mat_row(left, glp_get_num_rows(right), numCols, ind, v);

                // solve nodes with new constraints
                if (glp_simplex(right, NULL)) {
                    // error cleanup and exit
                    glp_delete_prob(left);
                    glp_delete_prob(right);
                    return nullptr;
                }

                double leftVal = glp_get_obj_val(left);

                double rightVal = glp_get_obj_val(right);

                if (rightVal > leftVal) {
                    upperBound = rightVal;
                } else {
                    upperBound = leftVal;
                }
            }
        }


    }

    size_t GomorySolver::maxFracIndex(glp_prob *prob) {
        double currMax = 0;
        size_t index = 0;

        for (size_t j = 1; j <= glp_get_num_cols(prob); j++) {
            if (glp_get_col_kind(prob, j) == GLP_IV) {
                double varVal = glp_get_col_prim(prob, j);
                double frac = std::abs(varVal - std::floor(varVal));

                if (std::abs(frac) > 0.00000001) {
                    if (frac > currMax) {
                        currMax = frac;
                        index = j;
                    }
                }
            }
        }

        return index;
    }
}