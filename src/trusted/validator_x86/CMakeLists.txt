list(APPEND NCCOPY_INPUTS
	"nccopycode.c"
	"nccopycode_stores.S"
)

add_library(nccopy STATIC ${NCCOPY_INPUTS})
