#include "solver.h"
#include <vector>
#include <string>
#include <stdexcept>
#include <chrono>
#include <iostream>

#include <omp.h>

namespace lp_pp
{
    using std::string;
    using std::vector;

    void Solver::simplex(SolverResult &res)
    {
        size_t iter = 0;

        auto start = std::chrono::high_resolution_clock::now();

        while (true)
        {
            iter++;

            int_fast32_t q = pivotCol();

            if (q < 0)
            {
                res.value = -tableau(m, m + n);
                std::cout << "Optimum found: " << res.getValue() << "\n";
                break;
            }

            int_fast32_t p = pivotRow(q);

            if (p < 0)
            {
                res.bounded = false;
                res.feasible = false;
                return;
            }

            doPivot(p, q);
        }

        auto end = std::chrono::high_resolution_clock::now();
        auto took = std::chrono::duration<double, std::milli>(end - start).count();

        res.took = took;

        std::cout << "Simplex took " << took << "ms"
                  << "\n";
    }

    void Solver::setTableau(const arma::dmat &tableau)
    {
        if (!tableau.empty())
        {
            m = tableau.n_rows - 1;
            n = tableau.n_cols - m - 1;

            this->tableau = tableau;
        }
    }

    int_fast32_t Solver::pivotCol()
    {
        for (size_t q = 0; q < m + n; q++)
        {
            if (tableau(m, q) > 0)
            {
                return q;
            }
        }

        return -1;
    }

    int_fast32_t Solver::pivotRow(size_t q)
    {
        int_fast32_t p = -1;

        for (size_t i = 0; i < m; i++)
        {
            if (tableau(i, q) <= 0)
            {
                continue;
            }
            else if (p < 0)
            {
                p = i;
            }
            else if (tableau(i, m + n) / tableau(i, q) < tableau(p, m + n) / tableau(p, q))
            {
                p = i;
            }
        }

        return p;
    }

    void Solver::doPivot(size_t p, size_t q)
    {
        for (size_t i = 0; i <= m; i++)
        {
            for (size_t j = 0; j <= m + n; j++)
            {
                if (i != p && j != q)
                {
                    tableau(i, j) -= tableau(p, j) * tableau(i, q) / tableau(p, q);
                }
            }
        }

        for (size_t i = 0; i <= m; i++)
        {
            if (i != p)
                tableau(i, q) = 0.0;
        }

        for (size_t j = 0; j <= m + n; j++)
        {
            if (j != q)
            {
                tableau(p, j) /= tableau(p, q);
            }
        }

        tableau(p, q) = 1.0;
    }
}