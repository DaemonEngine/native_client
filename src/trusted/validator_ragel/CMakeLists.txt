add_definitions("-DVALIDATOR_EXPORT=DLLEXPORT")

# src/trusted/validator_ragel/build.scons built both for both bitness
# because it was needed for the rdfa_validator library which looks to
# only be used for running some benchmark.
if (YOKAI_TARGET_ARCH_I686)
	set(VALIDATOR_LIB validator32)
	add_library(${VALIDATOR_LIB} OBJECT "gen/validator_x86_32.c")
elseif (YOKAI_TARGET_ARCH_AMD64)
	set(VALIDATOR_LIB validator64)
	add_library(${VALIDATOR_LIB} OBJECT "gen/validator_x86_64.c")
endif()

add_library(validator_features_all OBJECT "validator_features_all.c")
list(APPEND FEATURES_LIBS validator_features_all)
add_library(validator_features_validator OBJECT "validator_features_validator.c")
list(APPEND FEATURES_LIBS validator_features_validator)

if (YOKAI_TARGET_ARCH_I686)
	set(DFA_VALIDATE_CALLER_INPUTS
		"dfa_validate_32.c"
		"dfa_validate_common.c"
	)
elseif (YOKAI_TARGET_ARCH_AMD64)
	set(DFA_VALIDATE_CALLER_INPUTS
		"dfa_validate_64.c"
		"dfa_validate_common.c"
	)
endif()

add_library(dfa_validate_caller${VALIDATOR_ARCH_SUFFIX} STATIC ${DFA_VALIDATE_CALLER_INPUTS})
target_link_libraries(dfa_validate_caller${VALIDATOR_ARCH_SUFFIX} ${FEATURES_LIBS} ${VALIDATOR_LIB})

add_library(dfa_validate_caller ALIAS dfa_validate_caller${VALIDATOR_ARCH_SUFFIX})
