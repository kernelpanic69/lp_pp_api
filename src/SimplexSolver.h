//
// Created by broken_pc on 12/20/21.
//

#ifndef LP_SIMPLEXSOLVER_H
#define LP_SIMPLEXSOLVER_H

#include "Solver.h"

#include <memory>

namespace lp_pp {
    class SimplexSolver : public Solver {
        std::shared_ptr<SolverResult> solve(const LpModel &model) override;

        std::shared_ptr<glp_prob> get_prob() {
            return _prob;
        }

    private:
        std::shared_ptr<glp_prob> _prob{nullptr};
    };
}

#endif //LP_SIMPLEXSOLVER_H
