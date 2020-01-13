option(PL_SAT_SOLVER "PL_SAT_SOLVER" OFF)
if(PL_SAT_SOLVER OR BUILD_ALL_PLUGINS)
    file(GLOB_RECURSE SAT_SOLVER_INC ${CMAKE_CURRENT_SOURCE_DIR}/include/*.h)
    file(GLOB_RECURSE SAT_SOLVER_SRC ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
    file(GLOB_RECURSE SAT_SOLVER_PYTHON_SRC ${CMAKE_CURRENT_SOURCE_DIR}/python/*.cpp)
    
    find_package(Z3 REQUIRED)
    if(Z3_FOUND)
        message(STATUS "Found following z3 Libraries")
        message(STATUS "    Z3_INCLUDE_DIR: ${Z3_INCLUDE_DIR}")
        message(STATUS "    Z3_LIBRARY: ${Z3_LIBRARY}")
    else()
        message(STATUS "Could not find z3")
    endif(Z3_FOUND)

    hal_add_plugin(sat_solver
                   SHARED
                   HEADER ${SAT_SOLVER_INC}
                   SOURCES ${SAT_SOLVER_SRC} ${SAT_SOLVER_PYTHON_SRC}
                   INCLUDES PUBLIC $<BUILD_INTERFACE:${Z3_INCLUDE_DIRS}>
                   LINK_LIBRARIES PUBLIC ${Z3_LIBRARIES}
                   )
endif()
