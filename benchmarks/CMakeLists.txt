
find_package(benchmark REQUIRED)
if(NOT benchmark_FOUND OR NOT ${GTest_FOUND})
    message(FATAL_ERROR "Couldn't find benchmark library.")
endif()

add_executable(ecsify_benchmarks
    data_pool_benchmarks.cc
    entity_pool_benchmarks.cc
)

target_link_libraries(ecsify_benchmarks PRIVATE
    benchmark::benchmark_main
)
