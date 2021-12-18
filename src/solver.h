#pragma once

#include <map>
#include <string>
#include <vector>

#include <armadillo>

namespace lp_pp
{
    struct SolverResult
    {
        friend struct Solver;

        double getValue() const
        {
            return value;
        };

        const std::map<std::string, double> &getVariables() const
        {
            return variables;
        };

        bool isFeasible()
        {
            return feasible;
        }

        bool isBounded()
        {
            return bounded;
        }

    private:
        size_t took;
        bool feasible;
        bool bounded;
        double value;
        std::map<std::string, double> variables;
    };

    struct Solver
    {
        Solver() = default;
        void setTableau(const arma::dmat &tableau);

        void simplex(SolverResult &res);

    private:
        size_t m, n;

        arma::dmat tableau;
        std::vector<std::string> variableNames;

        int_fast32_t pivotCol();
        int_fast32_t pivotRow(size_t pivotCol);
        void doPivot(size_t p, size_t q);
    };
}