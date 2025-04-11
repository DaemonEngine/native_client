# set(NCFILEUTILS "ncfileutils${ARCH_SUFFIX}")
# add_library(${NCFILEUTILS} OBJECT "ncfileutil.c")

add_library(validation_cache OBJECT "validation_cache.c")

add_library(validators STATIC "validator_init.c")
target_link_libraries(validators validation_cache)
