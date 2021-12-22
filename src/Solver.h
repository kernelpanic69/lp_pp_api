//
// Created by broken_pc on 12/20/21.
//

#ifndef LP_SOLVER_H
#define LP_SOLVER_H

#include "lp_pp.h"
#include "SolverResult.h"
#include "LpModel.h"

#include <memory>

namespace lp_pp {

    struct Solver {
    DECLARE_INTERFACE(Solver)

        virtual std::shared_ptr<SolverResult> solve(const LpModel &model) = 0;
    };

}
#endif //LP_SOLVER_H
