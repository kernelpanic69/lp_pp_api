//
// Created by broken_pc on 12/20/21.
//

#ifndef LP_LP_PP_H
#define LP_LP_PP_H

#include <chrono>

namespace lp_pp {
    uint64_t currentTime() {
        return std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::system_clock::now().time_since_epoch()).count();
    }
}

#endif //LP_LP_PP_H
