#include <catch2/catch.hpp>

#include "model.h"

TEST_CASE("Testing model")
{
    using namespace lp_pp;

    Variable ale{"Ale", Variable::Type::CONTINUOUS, 13.0};
    Variable beer{"Beer", Variable::Type::CONTINUOUS, 23.0};

    Constraint corn{"Corn", Constraint::BoundType::UP, 480.0};
    Constraint hops{"Hops", Constraint::BoundType::UP, 160.0};
    Constraint malt{"Malt", Constraint::BoundType::UP, 1190.0};

    corn.setCoefs({{ale, 5.0}, {beer, 15.0}});
    hops.setCoefs({{ale, 4.0}, {beer, 4.0}});
    malt.setCoefs({{ale, 35.0}, {beer, 20.0}});

    Model model{{ale, beer}, {corn, hops, malt}};
}