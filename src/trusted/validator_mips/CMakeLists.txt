if (LINUX AND NOT ANDROID)
	list(APPEND VALIDATOR_FLAGS -Weffc)
	list(APPEND VALIDATOR_FLAGS -Woverloaded-virtual)
	list(APPEND VALIDATOR_FLAGS -fno-rtti)
	list(APPEND VALIDATOR_FLAGS -fomit-frame-pointer)
endif()	

list(APPEND VALIDATOR_INPUTS
	"address_set.cc"
	"validator.cc"
	"gen/decode.cc"
)

string(REPLACE ";" " " VALIDATOR_FLAGS_STRING "${VALIDATOR_FLAGS}")

add_library(mips_validator_core OBJECT ${VALIDATOR_INPUTS})
set_target_properties(mips_validator_core PROPERTIES FLAGS "${VALIDATOR_FLAGS_STRING}")

add_library(ncvalidate_mips STATIC "ncvalidate.cc")
target_link_libraries(ncvalidate_mips mips_validator_core) # We don't support OPTIONAL_COVERAGE_LIBS.
set_target_properties(ncvalidate_mips PROPERTIES FLAGS "${VALIDATOR_FLAGS_STRING}")
