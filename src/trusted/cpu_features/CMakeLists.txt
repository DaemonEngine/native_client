if (YOKAI_TARGET_ARCH_I686 OR YOKAI_TARGET_ARCH_AMD64)
	list(APPEND CPU_FEATURES_INPUTS
		"arch/x86/cpu_x86.c"
		"arch/x86/cpu_xgetbv.S"
	)
elseif (YOKAI_TARGET_ARCH_ARMHF OR YOKAI_TARGET_ARCH_ARMEL)
	# For unkown reasons src/trusted/cpu_features/build.scons was building that on every architecture.
	list(APPEND CPU_FEATURES_INPUTS "arch/arm/cpu_arm.c")
elseif (YOKAI_TARGET_ARCH_MIPSEL)
	# For unkown reasons src/trusted/cpu_features/build.scons was building that on every architecture.
	list(APPEND CPU_FEATURES_INPUTS "arch/mips/cpu_mips.c")
endif()

add_library(cpu_features STATIC ${CPU_FEATURES_INPUTS})
