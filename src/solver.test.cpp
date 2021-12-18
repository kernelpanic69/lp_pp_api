#include <catch2/catch.hpp>

#include "solver.h"

#include <cmath>

namespace
{
    bool approximatelyEquals(double x, double y, double epsilon = 0.00001)
    {
        return std::fabs(x - y) < epsilon;
    }
}

TEST_CASE("Solver solves simple problem")
{
    using lp_pp::Solver;
    using lp_pp::SolverResult;

    Solver solver;

    solver.setTableau({{5.0, 15.0, 1.0, 0.0, 0.0, 480.0},
                       {4.0, 4.0, 0.0, 1.0, 0.0, 160.0},
                       {35.0, 20.0, 0.0, 0.0, 1.0, 1190.0},
                       {13.0, 23.0, 0.0, 0.0, 0.0, 0.0}});

    SECTION("Simplex method")
    {
        SolverResult res;
        double expect = 800.0;
        solver.simplex(res);

        REQUIRE(approximatelyEquals(expect, res.getValue()));
    }
}