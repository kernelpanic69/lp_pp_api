#include "bnb_solver.h"
#include <glpk.h>
#include <stdexcept>
#include <queue>
#include <cmath>

namespace lp_pp
{
    BnBSolver::BnBSolver(glp_prob *relaxed)
    {
        if (relaxed == nullptr)
        {
            throw std::runtime_error("Initial relaxed solution empty");
        }

        if (glp_get_status(relaxed) != GLP_OPT)
        {
            throw std::runtime_error("Provided relaxed solution is not optimal");
        }

        initial = relaxed;
    }

    glp_prob *BnBSolver::doBranch()
    {
        // Initial relaxed problem
        glp_prob *opt = initial;
        double upperBound = glp_get_obj_val(opt);

        while (true)
        {
            double targetIndex = maxFracIndex(opt);

            if (targetIndex == 0)
            {
                // No fractional variables left. Cleanup and exit;
                return opt;
            }
        }
    }

    size_t BnBSolver::maxFracIndex(glp_prob *prob)
    {
        double currMax = 0;
        size_t index = 0;

        for (size_t j = 1; j <= glp_get_num_cols(prob); j++)
        {
            if (glp_get_col_kind(prob, j) == GLP_IV)
            {
                double varVal = glp_get_col_prim(prob, j);
                double frac = std::abs(varVal - std::floor(varVal));

                if (frac != 0.0)
                {
                    currMax = frac;
                    index = j;
                }
            }
        }

        return index;
    }
}