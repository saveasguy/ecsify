cmake_minimum_required(VERSION 3.10)
project(ecsify)

include_directories(include)
add_subdirectory(src)

option(ECSIFY_BUILD_EXAMPLES "Build examples" ON)
option(ECSIFY_BUILD_BENCHMARKS "Build benchmarks" ON)
option(ECSIFY_ENABLE_TESTING "Build and enable tests" ON)
option(ECSIFY_ENABLE_COVERAGE "Build with coverage runtimes" OFF)

if(${ECSIFY_BUILD_EXAMPLES})
    add_subdirectory(examples)
endif()

if(${ECSIFY_ENABLE_TESTING})
    add_subdirectory(tests)
endif()

if(${ECSIFY_BUILD_BENCHMARKS})
    add_subdirectory(benchmarks)
endif()
