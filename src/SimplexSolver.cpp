//
// Created by broken_pc on 12/20/21.
//

#include "SimplexSolver.h"
#include "lp_pp.h"

#include <memory>

#include <glpk.h>

using std::make_shared;
using std::shared_ptr;

std::shared_ptr<lp_pp::SolverResult> lp_pp::SimplexSolver::solve(const lp_pp::LpModel &model) {
    auto prob = model.get_glp_prob().get();

    Stats stats;

    stats.timeStarted = currentTime();
    glp_simplex(prob, nullptr);
    stats.timeFinished = currentTime();
    _prob.reset(prob, glp_delete_prob);
    glp_mem_usage(nullptr, nullptr, &stats.memory, nullptr);

    double obj = glp_get_obj_val(prob);

    auto res = make_shared<SolverResult>();

    res->setObjective(obj);

}
