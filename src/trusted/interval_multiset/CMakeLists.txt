list(APPEND NACL_INTERVAL_LIB_INPUTS
	"nacl_interval_multiset_delete.c"
	"nacl_interval_multiset_factory.c"
	"nacl_interval_list.c"
	"nacl_interval_range_tree.c"
)

add_library(nacl_interval ${NACL_INTERVAL_LIB_INPUTS})
