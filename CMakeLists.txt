project(ecsify)
cmake_minimum_required(VERSION 3.0)

include_directories(include)

if(NOT ECSIFY_BUILD_EXAMPLES)
    set(ECSIFY_BUILD_EXAMPLES ON)
endif()
if(${ECSIFY_BUILD_EXAMPLES})
    add_subdirectory(examples)
endif()

if(NOT ECSIFY_ENABLE_TESTING)
    set(ECSIFY_ENABLE_TESTING ON)
endif()
if(${ECSIFY_ENABLE_TESTING})
    add_subdirectory(tests)
endif()
