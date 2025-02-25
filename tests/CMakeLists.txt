find_package(GTest REQUIRED)
if(NOT GTest_FOUND OR NOT ${GTest_FOUND})
    message(FATAL_ERROR "Couldn't find GTest library. You can disable testing by passing -DECSIFY_ENABLE_TESTING=OFF.")
endif()

add_executable(world_tests world_tests.cc)
target_link_libraries(world_tests gtest::gtest)

enable_testing()
add_test(NAME world_tests COMMAND ${CMAKE_CURRENT_BINARY_DIR}/world_tests)
