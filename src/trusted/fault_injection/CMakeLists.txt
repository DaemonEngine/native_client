list(APPEND NACL_FI_LIB_INPUTS
	"fault_injection.c"
	"test_injection.c"
)

add_library(nacl_fault_inject STATIC ${NACL_FI_LIB_INPUTS})
target_link_libraries(nacl_fault_inject platform)
