add_library(ecsify
    entity_pool.cc
)
if(${ECSIFY_ENABLE_COVERAGE})
    target_compile_options(ecsify PRIVATE -coverage)
endif()
