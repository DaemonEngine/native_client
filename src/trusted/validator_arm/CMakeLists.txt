# Android compiler cannot compile NDK C++ headers with these.

if (LINUX AND NOT ANDROID)
	# TODO(cbiffle): give everyone else these warnings too.
	# TODO(karl): Why does -fno-rtti cause linking issues.
	# CXXFLAGS='-Weffc++ -Woverloaded-virtual -fno-rtti -fomit-frame-pointer',
	list(APPEND VALIDATOR_FLAGS -Weffc)
	list(APPEND VALIDATOR_FLAGS -Woverloaded-virtual)
	list(APPEND VALIDATOR_FLAGS -fomit-frame-pointer)
endif()

list(APPEND GEN_TEST_LIST
	# Generated table file names.
	"gen/arm32_decode_advanced_simd_data_processing_instructions_tests.cc"
	"gen/arm32_decode_advanced_simd_element_or_structure_load_store_instructions_tests.cc"
	"gen/arm32_decode_ARMv7_tests.cc"
	"gen/arm32_decode_branch_branch_with_link_and_block_data_transfer_tests.cc"
	"gen/arm32_decode_coprocessor_instructions_and_supervisor_call_tests.cc"
	"gen/arm32_decode_data_processing_and_miscellaneous_instructions_tests.cc"
	"gen/arm32_decode_data_processing_immediate_tests.cc"
	"gen/arm32_decode_data_processing_register_shifted_register_tests.cc"
	"gen/arm32_decode_data_processing_register_tests.cc"
	"gen/arm32_decode_extension_register_load_store_instructions_tests.cc"
	"gen/arm32_decode_extra_load_store_instructions_tests.cc"
	"gen/arm32_decode_floating_point_data_processing_instructions_tests.cc"
	"gen/arm32_decode_halfword_multiply_and_multiply_accumulate_tests.cc"
	"gen/arm32_decode_load_store_word_and_unsigned_byte_tests.cc"
	"gen/arm32_decode_media_instructions_tests.cc"
	"gen/arm32_decode_memory_hints_advanced_simd_instructions_and_miscellaneous_instructions_tests.cc"
	"gen/arm32_decode_miscellaneous_instructions_tests.cc"
	"gen/arm32_decode_msr_immediate_and_hints_tests.cc"
	"gen/arm32_decode_multiply_and_multiply_accumulate_tests.cc"
	"gen/arm32_decode_other_floating_point_data_processing_instructions_tests.cc"
	"gen/arm32_decode_packing_unpacking_saturation_and_reversal_tests.cc"
	"gen/arm32_decode_parallel_addition_and_subtraction_signed_tests.cc"
	"gen/arm32_decode_parallel_addition_and_subtraction_unsigned_tests.cc"
	"gen/arm32_decode_saturating_addition_and_subtraction_tests.cc"
	"gen/arm32_decode_signed_multiply_signed_and_unsigned_divide_tests.cc"
	"gen/arm32_decode_simd_dp_1imm_tests.cc"
	"gen/arm32_decode_simd_dp_2misc_tests.cc"
	"gen/arm32_decode_simd_dp_2scalar_tests.cc"
	"gen/arm32_decode_simd_dp_2shift_tests.cc"
	"gen/arm32_decode_simd_dp_3diff_tests.cc"
	"gen/arm32_decode_simd_dp_3same_tests.cc"
	"gen/arm32_decode_synchronization_primitives_tests.cc"
	"gen/arm32_decode_transfer_between_arm_core_and_extension_register_8_16_and_32_bit_tests.cc"
	"gen/arm32_decode_transfer_between_arm_core_and_extension_registers_64_bit_tests.cc"
	"gen/arm32_decode_unconditional_instructions_tests.cc"
)

list(APPEND GEN_ACTUAL_LIST
	"gen/arm32_decode_actuals_1.cc"
	"gen/arm32_decode_actuals_1.h"
	"gen/arm32_decode_actuals_2.cc"
	"gen/arm32_decode_actuals_2.h"
)

list(APPEND GEN_BASELINE_LIST
	"gen/arm32_decode_baselines_1.cc"
	"gen/arm32_decode_baselines_1.h"
	"gen/arm32_decode_baselines_2.cc"
	"gen/arm32_decode_baselines_2.h"
	"gen/arm32_decode_baselines_3.cc"
	"gen/arm32_decode_baselines_3.h"
)

list(APPEND GEN_OTHER_LIST
	"gen/arm32_decode.cc"
	"gen/arm32_decode.h"
	"gen/arm32_decode_actuals.h"
	"gen/arm32_decode_baselines.h"
#	"gen/arm32_decode_named.cc" # Why?
	"gen/arm32_decode_named_classes.h"
	"gen/arm32_decode_named_bases.h"
	"gen/arm32_decode_named_decoder.h"
	${GEN_ACTUAL_LIST}
	${GEN_BASELINE_LIST}
)

list(APPEND VALIDATOR_INPUTS
	"address_set.cc"
	"inst_classes.cc"
	"model.cc"
	"arm_helpers.cc"
	"validator.cc"
)

string(REPLACE ";" " " VALIDATOR_FLAGS_STRING "${VALIDATOR_FLAGS}")

add_library(arm_validator_core OBJECT ${VALIDATOR_INPUTS} ${GEN_OTHER_LIST})
set_target_properties(arm_validator_core PROPERTIES FLAGS "${VALIDATOR_FLAGS_STRING}")

add_library(ncvalidate_arm_v2 STATIC "ncvalidate.cc")
target_link_libraries(ncvalidate_arm_v2 arm_validator_core) # We don't support OPTIONAL_COVERAGE_LIBS.
set_target_properties(ncvalidate_arm_v2 PROPERTIES FLAGS "${VALIDATOR_FLAGS_STRING}")

add_library(arm_validator_reporters STATIC "problem_reporter.cc")
set_target_properties(arm_validator_reporters PROPERTIES FLAGS "${VALIDATOR_FLAGS_STRING}")
