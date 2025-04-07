if (ARCH_i686 OR ARCH_amd64)
	list(APPEND CPU_FEATURES_INPUTS
		"arch/x86/cpu_x86.c"
		"arch/x86/cpu_xgetbv.S"
	)
elseif (ARCH_armhf OR ARCH_armel)
	# For unkown reasons src/trusted/cpu_features/build.scons was building that on every architecture.
	list(APPEND CPU_FEATURES_INPUTS "arch/arm/cpu_arm.c")
elseif (ARCH_mipsel)
	# For unkown reasons src/trusted/cpu_features/build.scons was building that on every architecture.
	list(APPEND CPU_FEATURES_INPUTS "arch/mips/cpu_mips.c")
endif()

add_library(cpu_features STATIC ${CPU_FEATURES_INPUTS})
