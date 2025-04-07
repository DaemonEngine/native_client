if (YOKAI_TARGET_SYSTEM_WINDOWS)
	list(APPEND PQL_INPUTS
		"win/nacl_os_qualify.c"
		"win/nacl_dep_qualify.c"
	)
elseif (YOKAI_TARGET_SYSTEM_LINUX_COMPATIBILITY OR YOKAI_TARGET_SYSTEM_MACOS)
	list(APPEND PQL_INPUTS "posix/nacl_dep_qualify.c")

	if (YOKAI_TARGET_SYSTEM_LINUX_COMPATIBILITY)
		list(APPEND PQL_INPUTS "linux/nacl_os_qualify.c")
	elseif (YOKAI_TARGET_SYSTEM_MACOS)
		list(APPEND PQL_INPUTS "osx/nacl_os_qualify.c")
	endif()
endif()

if (YOKAI_TARGET_ARCH_I686 OR YOKAI_TARGET_ARCH_AMD64)
	list(APPEND PQL_INPUTS "arch/x86/nacl_cpuallowlist.c")

	if (YOKAI_TARGET_SYSTEM_MACOS)
		list(APPEND VCPUID_FLAGS "-mdynamic-no-pic")
# TODO:		vcpuid_env.FilterOut(CCFLAGS = ['-fPIC'])
		list(APPEND VCPUID_FLAGS "-fno-PIC")
	elseif (YOKAI_TARGET_SYSTEM_LINUX_COMPATIBILITY)
		list(APPEND VCPUID_FLAGS "-msse3")
	endif()

	add_library(vcpuid OBJECT "arch/x86/vcpuid.c")
	list(APPEND VCPUID_LIBS vcpuid)

	if (YOKAI_TARGET_ARCH_I686)
		list(APPEND PQL_INPUTS "arch/x86_32/nacl_dep_qualify_arch.c")
	elseif (YOKAI_TARGET_ARCH_AMD64)
		list(APPEND PQL_INPUTS "arch/x86_64/nacl_dep_qualify_arch.c")
	endif()
elseif (YOKAI_TARGET_ARCH_ARMHF OR YOKAI_TARGET_ARCH_ARMEL)
	list(APPEND PQL_INPUTS
		# Compile ARM primitives for checking XN functionality
		"arch/arm/nacl_dep_qualify_arch.c"
		# Compile ARM primitives for checking VFP/vector features.
		"arch/arm/nacl_qualify_fpu.c"
		# Compile ARM primitives for checking sandboxing features.
		"arch/arm/nacl_qualify_sandbox_instrs.c"
		# Compile ARM primitives for checking alignment features.
		"arch/arm/nacl_qualify_unaligned.c"
	)
elseif (YOKAI_TARGET_ARCH_MIPSEL)
	list(APPEND PQL_INPUTS "arch/mips/nacl_qualify_fpu.c")
endif()

add_library(platform_qual_lib STATIC ${PQL_INPUTS})
target_link_libraries(platform_qual_lib ${VCPUID_LIBS})
