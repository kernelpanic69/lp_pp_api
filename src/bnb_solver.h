#include <glpk.h>
#include <cstdint>

namespace lp_pp
{
    class BnBSolver
    {
    public:
        BnBSolver(glp_prob *relaxed);

    private:
        glp_prob *initial;
        glp_prob *doBranch();
        size_t maxFracIndex(glp_prob *prob);
    };
}