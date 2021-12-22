#pragma clang diagnostic push
#pragma ide diagnostic ignored "bugprone-macro-parentheses"
//
// Created by broken_pc on 12/20/21.
//

#ifndef LP_LP_PP_H
#define LP_LP_PP_H

#include <chrono>

namespace lp_pp {
    uint64_t currentTime();

#define DECLARE_INTERFACE(ClassName) \
   public :                                                                  \
      virtual ~ClassName() {}                                                \
   protected :                                                               \
      ClassName() {}                                                         \
      ClassName(const ClassName & ) {}                                       \
      ClassName & operator = (const ClassName & ) { return *this ; }         \
      ClassName(ClassName && ) noexcept {}                                   \
      ClassName & operator = (ClassName && ) noexcept { return *this ; }     \
   private :
}

#endif //LP_LP_PP_H

#pragma clang diagnostic pop