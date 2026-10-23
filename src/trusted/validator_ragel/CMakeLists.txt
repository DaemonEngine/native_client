add_definitions("-DVALIDATOR_EXPORT=DLLEXPORT")

# RDFA_VALIDATOR is set to ON when building tls_edit.

if (YOKAI_TARGET_ARCH_I686 OR RDFA_VALIDATOR)
	add_library(validator32 OBJECT "gen/validator_x86_32.c")
	list(APPEND VALIDATOR_LIBS validator32)
elseif (YOKAI_TARGET_ARCH_AMD64 OR RDFA_VALIDATOR)
	add_library(validator64 OBJECT "gen/validator_x86_64.c")
	list(APPEND VALIDATOR_LIBS validator64)
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

if (RDFA_VALIDATOR)
	add_library(rdfa_validator)
	target_link_libraries(rdfa_validator ${FEATURES_LIBS} ${VALIDATOR_LIBS})
endif()

add_library(dfa_validate_caller${VALIDATOR_ARCH_SUFFIX} STATIC ${DFA_VALIDATE_CALLER_INPUTS})
target_link_libraries(dfa_validate_caller${VALIDATOR_ARCH_SUFFIX} ${FEATURES_LIBS} ${VALIDATOR_LIBS})

add_library(dfa_validate_caller ALIAS dfa_validate_caller${VALIDATOR_ARCH_SUFFIX})
