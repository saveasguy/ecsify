find_package(GTest REQUIRED)
if(NOT GTest_FOUND OR NOT ${GTest_FOUND})
    message(FATAL_ERROR "Couldn't find GTest library. You can disable testing by passing -DECSIFY_ENABLE_TESTING=OFF.")
endif()

enable_testing()

add_executable(ecsify_tests
    data_pool_tests.cc
    entity_pool_tests.cc
    world_tests.cc
)
if(${ECSIFY_ENABLE_COVERAGE})
    target_compile_options(ecsify_tests PRIVATE -coverage)
    target_link_options(ecsify_tests PRIVATE -coverage)
endif()
target_link_libraries(ecsify_tests PRIVATE
    gtest::gtest
)
add_test(
    NAME ecsify_tests
    COMMAND "ecsify_tests --gtest_output=json:${CMAKE_CURRENT_BINARY_DIR}/ecsify_tests.json"
)
