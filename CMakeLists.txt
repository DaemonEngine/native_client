cmake_minimum_required(VERSION 3.12)

if (COMPILER_TARGET)
	set(CMAKE_C_COMPILER_TARGET "${COMPILER_TARGET}")
	set(CMAKE_CXX_COMPILER_TARGET "${COMPILER_TARGET}")
	set(CMAKE_ASM_COMPILER_TARGET "${COMPILER_TARGET}")
endif()

project(native_client C CXX ASM)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}")

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

include(Yokai/Detection)
include(NaClFlags)

if (NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE RelWithDebInfo)
endif()

option(BUILD_NACL_LOADER "Build the sel_ldr program." ON)

if (YOKAI_TARGET_SYSTEM_LINUX_COMPATIBILITY)
	option(BUILD_NACL_HELPER_BOOTSTRAP "Build the nacl_helper_bootstrap program on platforms requiring it." ON)
endif()

# TODO(arbenson): remove this once binutils bug is fixed (see
# src/trusted/service_runtime/arch/x86_64/sel_addrspace_posix_x86_64.c)
if (BUILD_NACL_LOADER AND YOKAI_TARGET_ARCH_AMD64 AND NOT YOKAI_TARGET_SYSTEM_WINDOWS)
	option(USE_AMD64_ZERO_BASED_SANDBOX "Allow the zero-based sandbox model to run insecurely." OFF)
endif()

if (BUILD_NACL_LOADER AND YOKAI_TARGET_SYSTEM_WINDOWS)
	option(FORCE_NO_TRUSTED_BUILD "Prevent use of trusted toolchain." OFF)
endif()

if (BUILD_NACL_LOADER AND YOKAI_TARGET_SYSTEM_WINDOWS)
	set(REQUIRE_MASM ON)
endif()

if (BUILD_NACL_LOADER AND YOKAI_TARGET_SYSTEM_MACOS)
	set(REQUIRE_PYTHON ON)
endif()

if (BUILD_NACL_HELPER_BOOTSTRAP)
	set(REQUIRE_PYTHON ON)
endif()

if (REQUIRE_PYTHON)
	if (NOT PYTHON)
		find_program(PATH_PYTHON NAMES "python3" DOC "Path to the python3 executable." REQUIRED)
	endif()
endif()

if (REQUIRE_MASM)
	if (YOKAI_CXX_COMPILER_MINGW)
		if (NOT CMAKE_ASM_MASM_COMPILER)
			find_program(PATH_JWASM NAMES "jwasm" DOC "Path to the JWasm executable." REQUIRED)

			set(CMAKE_ASM_MASM_COMPILER "${PATH_JWASM}")
		endif()

		include_directories("${CMAKE_CURRENT_SOURCE_DIR}/include-hax/fake_masm")

		if (YOKAI_TARGET_ARCH_I686)
			list(APPEND CMAKE_ASM_MASM_FLAGS "-coff")
		elseif(YOKAI_TARGET_ARCH_AMD64)
			list(APPEND CMAKE_ASM_MASM_FLAGS "-win64")
		endif()
	endif()

	enable_language(ASM_MASM)
endif()

include_directories("include-hax")

if (YOKAI_TARGET_ARCH_I686)
	set(VALIDATOR_ARCH_SUFFIX "_x86_32")
elseif (YOKAI_TARGET_ARCH_AMD64)
	set(VALIDATOR_ARCH_SUFFIX "_x86_64")
elseif (YOKAI_TARGET_ARCH_ARMHF OR YOKAI_TARGET_ARCH_ARMEL)
	set(VALIDATOR_ARCH_SUFFIX "_arm")
elseif (YOKAI_TARGET_ARCH_MIPSEL)
	set(VALIDATOR_ARCH_SUFFIX "_mips32")
endif()

if (BUILD_NACL_LOADER)
	add_subdirectory(src/shared/gio)
	add_subdirectory(src/shared/imc)
	add_subdirectory(src/shared/platform)
	add_subdirectory(src/trusted/cpu_features)
	add_subdirectory(src/trusted/debug_stub)
	add_subdirectory(src/trusted/desc)
	add_subdirectory(src/trusted/fault_injection)
	add_subdirectory(src/trusted/interval_multiset)
	add_subdirectory(src/trusted/nacl_base)
	add_subdirectory(src/trusted/perf_counter)
	add_subdirectory(src/trusted/platform_qualify)
	add_subdirectory(src/trusted/validator)

	if (YOKAI_TARGET_ARCH_I686 OR YOKAI_TARGET_ARCH_AMD64)
		add_subdirectory(src/trusted/validator_x86)
		add_subdirectory(src/trusted/validator_ragel)
	elseif (YOKAI_TARGET_ARCH_ARMHF OR YOKAI_TARGET_ARCH_ARMEL)
		add_subdirectory(src/trusted/validator_arm)
	elseif (YOKAI_TARGET_ARCH_MIPSEL)
		add_subdirectory(src/trusted/validator_mips)
	endif()
endif()

if (BUILD_NACL_LOADER)
	add_subdirectory(src/trusted/service_runtime)
endif()

if (BUILD_NACL_HELPER_BOOTSTRAP)
	add_subdirectory(src/trusted/service_runtime/linux)
endif()
