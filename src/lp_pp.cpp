//
// Created by broken_pc on 12/20/21.
//

#include "lp_pp.h"

#include "chrono"

uint64_t lp_pp::currentTime() {
    return std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();
}