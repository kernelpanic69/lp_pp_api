//
// Created by broken_pc on 12/21/21.
//

#ifndef LP_GOMORYSOLVER_H
#define LP_GOMORYSOLVER_H

#include "Solver.h"

namespace lp_pp {
    class GomorySolver : public Solver {
        std::shared_ptr<SolverResult> solve(const LpModel &model) override;

        size_t maxFracIndex(glp_prob *prob);
    };
}

#endif //LP_GOMORYSOLVER_H
