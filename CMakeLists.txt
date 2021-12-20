CMAKE_MINIMUM_REQUIRED(VERSION 3.21)

project(lp++ VERSION 0.0.1)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED true)
set(CMAKE_COMP)

add_executable(lp++
        src/api.cc
        src/bnb_solver.cc
        src/Stats.cpp
        src/Stats.h
        src/lp_pp.h src/LpModel.cpp src/LpModel.h src/SolverResult.cpp src/SolverResult.h)

target_include_directories(${PROJECT_NAME} PUBLIC include)

find_library(PISTACHE_LIBRARY pistache)
find_library(GLPK_LIBRARY glpk)
find_library(FMT_LIBRARY fmt)
find_library(Z_LIBRARY z)

target_link_libraries(${PROJECT_NAME} ${PISTACHE_LIBRARY})
target_link_libraries(${PROJECT_NAME} ${GLPK_LIBRARY})
target_link_libraries(${PROJECT_NAME} ${GLPK_LIBRARY})
target_link_libraries(${PROJECT_NAME} ${FMT_LIBRARY})
target_link_libraries(${PROJECT_NAME} ${Z_LIBRARY})