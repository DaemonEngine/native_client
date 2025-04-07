cmake_minimum_required(VERSION 3.12)

project(native_client C CXX ASM)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}")

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

include(DaemonPlatform/Platform)
include(NaClFlags)

if (NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE RelWithDebInfo)
endif()

option(BUILD_NACL_LOADER "Build the sel_ldr program." ON)

if (LINUX)
	option(BUILD_NACL_HELPER_BOOTSTRAP "Build the nacl_helper_bootstrap program on platforms requiring it." ON)
endif()

# TODO(arbenson): remove this once binutils bug is fixed (see
# src/trusted/service_runtime/arch/x86_64/sel_addrspace_posix_x86_64.c)
if (BUILD_NACL_LOADER AND ARCH_amd64 AND NOT WIN32)
	option(USE_AMD64_ZERO_BASED_SANDBOX "Allow the zero-based sandbox model to run insecurely." OFF)
	list(APPEND INHERITED_OPTIONS "USE_AMD64_ZERO_BASED_SANDBOX")
endif()

if (BUILD_NACL_LOADER AND WIN32)
	option(FORCE_NO_TRUSTED_BUILD "Prevent use of trusted toolchain." OFF)
endif()

if (BUILD_NACL_LOADER AND WIN32)
	set(REQUIRE_MASM ON)
endif()

if (BUILD_NACL_LOADER AND APPLE)
	set(REQUIRE_PYTHON ON)
endif()

if (BUILD_NACL_HELPER_BOOTSTRAP)
	set(REQUIRE_PYTHON ON)
endif()

if (REQUIRE_PYTHON)
	if (NOT PYTHON)
		find_program(PYTHON NAMES "python3" DOC "Path to the python3 executable." REQUIRED)
	endif()
endif()

if (REQUIRE_MASM)
	if (NOT MSVC AND NOT CMAKE_ASM_MASM_COMPILER)
		include_directories("${CMAKE_CURRENT_SOURCE_DIR}/include-hax/fake_masm")

		find_program(JWASM NAMES "jwasm" DOC "Path to the JWasm executable." REQUIRED)

		set(CMAKE_ASM_MASM_COMPILER "${JWASM}")

		if (ARCH_i686)
			list(APPEND CMAKE_ASM_MASM_FLAGS "-coff")
		elseif(ARCH_amd64)
			list(APPEND CMAKE_ASM_MASM_FLAGS "-win64")
		endif()
	endif()

	enable_language(ASM_MASM)
endif()

include_directories("include-hax")

if (ARCH_i686)
	set(ARCH_SUFFIX "_x86_32")
elseif (ARCH_amd64)
	set(ARCH_SUFFIX "_X86_64")
elseif (ARCH_armhf OR ARCH_armel)
	set(ARCH_SUFFIX "_arm")
elseif (ARCH_mipsel)
	set(ARCH_SUFFIX "_mips32")
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

	if (ARCH_i686 OR ARCH_amd64)
		add_subdirectory(src/trusted/validator_x86)
		add_subdirectory(src/trusted/validator_ragel)
	elseif (ARCH_armhf OR ARCH_armel)
		add_subdirectory(src/trusted/validator_arm)
	elseif (ARCH_mipsel)
		add_subdirectory(src/trusted/validator_mips)
	endif()
endif()

if (BUILD_NACL_LOADER)
	add_subdirectory(src/trusted/service_runtime)
endif()

if (BUILD_NACL_HELPER_BOOTSTRAP)
	option(BUILD_NACL_HELPER_BOOTSTRAP_WITH_CLANG "Build the nacl_helper_bootstrap program with Clang." ON)
endif()

if (BUILD_NACL_HELPER_BOOTSTRAP)
	if (DAEMON_C_COMPILER_Clang_COMPATIBILITY)
		set(BUILD_BOOTSTRAP_DIRECTLY ON)
	elseif (NOT BUILD_NACL_HELPER_BOOTSTRAP_WITH_CLANG)
		set(BUILD_BOOTSTRAP_DIRECTLY ON)
	endif()

	if (BUILD_BOOTSTRAP_DIRECTLY)
		add_subdirectory(src/trusted/service_runtime/linux)
	else()
		if (CMAKE_GENERATOR MATCHES "Visual Studio")
			set(BOOTSTRAP_GENERATOR "NMake Makefiles")
		else()
			set(BOOTSTRAP_GENERATOR "${CMAKE_GENERATOR}")
		endif()

		find_program(CLANG_C_COMPILER NAMES "clang" DOC "Path to the Clang C compiler." REQUIRED)
		find_program(CLANG_CXX_COMPILER NAMES "clang++" DOC "Path to the Clang C++ compiler." REQUIRED)

		if (NOT CLANG_TARGET)
			execute_process(COMMAND "${CMAKE_CXX_COMPILER}" -dumpmachine
				OUTPUT_VARIABLE CLANG_TARGET
				OUTPUT_STRIP_TRAILING_WHITESPACE
				# CMake 3.18.4 from Debian Buster doesn't support that option:
				# COMMAND_ERROR_IS_FATAL ANY
			)

			if (NOT CLANG_TARGET)
				message(FATAL_ERROR "Failed to read the compiler target.")
			endif()
		endif()

		foreach(inherited_option ${INHERITED_OPTIONS})
			list(APPEND INHERITED_OPTION_ARGS "-D${inherited_option}=${${inherited_option}}")
		endforeach()

		set(BOOTSTRAP_BUILD_DIR "${CMAKE_CURRENT_BINARY_DIR}/nacl_helper_bootstrap-build")

		include(ExternalProject)

		ExternalProject_Add(nacl_helper_bootstrap-project
			SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}"
			BINARY_DIR "${BOOTSTRAP_BUILD_DIR}"
			CMAKE_GENERATOR "${BOOTSTRAP_GENERATOR}"
			CMAKE_ARGS
				"-DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}"
				"-DCMAKE_C_COMPILER=${CLANG_C_COMPILER}"
				"-DCMAKE_CXX_COMPILER=${CLANG_CXX_COMPILER}"
				"-DCMAKE_C_FLAGS=--target=${CLANG_TARGET}"
				"-DCMAKE_CXX_FLAGS=--target=${CLANG_TARGET}"
				-DBUILD_NACL_LOADER=OFF
				-DBUILD_NACL_HELPER_BOOTSTRAP=ON
				${INHERITED_OPTION_ARGS}
			# CMake 3.18.4 from Debian Buster doesn't support that option:
			# INSTALL_BYPRODUCTS
			INSTALL_COMMAND echo
		)

		add_custom_target(nacl_helper_bootstrap ALL
			COMMAND "${CMAKE_COMMAND}" -E copy "${BOOTSTRAP_BUILD_DIR}/nacl_helper_bootstrap"
				"${CMAKE_CURRENT_BINARY_DIR}/nacl_helper_bootstrap"
			BYPRODUCTS
				"${CMAKE_CURRENT_BINARY_DIR}/nacl_helper_bootstrap"
			DEPENDS nacl_helper_bootstrap-project
		)
	endif()
endif()
