if (WIN32)
	list(APPEND PQL_INPUTS
		"win/nacl_os_qualify.c"
		"win/nacl_dep_qualify.c"
	)
elseif (LINUX OR APPLE)
	list(APPEND PQL_INPUTS "posix/nacl_dep_qualify.c")

	if (LINUX)
		list(APPEND PQL_INPUTS "linux/nacl_os_qualify.c")
	elseif (APPLE)
		list(APPEND PQL_INPUTS "osx/nacl_os_qualify.c")
	endif()
endif()

if (ARCH_i686 OR ARCH_amd64)
	list(APPEND PQL_INPUTS "arch/x86/nacl_cpuallowlist.c")

	if (APPLE)
		list(APPEND VCPUID_FLAGS "-mdynamic-no-pic")
# TODO:		vcpuid_env.FilterOut(CCFLAGS = ['-fPIC'])
		list(APPEND VCPUID_FLAGS "-fno-PIC")
	elseif (LINUX)
		list(APPEND VCPUID_FLAGS "-msse3")
	endif()

	add_library(vcpuid OBJECT "arch/x86/vcpuid.c")
	list(APPEND VCPUID_LIBS vcpuid)

	if (ARCH_i686)
		list(APPEND PQL_INPUTS "arch/x86_32/nacl_dep_qualify_arch.c")
	elseif (ARCH_amd64)
		list(APPEND PQL_INPUTS "arch/x86_64/nacl_dep_qualify_arch.c")
	endif()
elseif (ARCH_armhf OR ARCH_armel)
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
elseif (ARCH_mipsel)
	list(APPEND PQL_INPUTS "arch/mips/nacl_qualify_fpu.c")
endif()

add_library(platform_qual_lib STATIC ${PQL_INPUTS})
target_link_libraries(platform_qual_lib ${VCPUID_LIBS})
