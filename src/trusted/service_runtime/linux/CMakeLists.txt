#TODO:	bootstrap_env = env.Clone()
#TODO:	bootstrap_env.Replace(CLANG_OPTS='')
#TODO:	bootstrap_env.FilterOut(CCFLAGS=['-fstack-protector', '-fPIC', '-fPIE', '-pedantic', '$COVERAGE_CCFLAGS'],
list(APPEND BOOTSTRAP_FLAGS "-fno-pic")
list(APPEND BOOTSTRAP_FLAGS "-fno-PIC")
list(APPEND BOOTSTRAP_FLAGS "-fno-pie")
# Assumed to be what is expected, as -fno-pie was passed twice in src/trusted/service_runtime/build.scons.
list(APPEND BOOTSTRAP_FLAGS "-fno-PIE")
list(APPEND BOOTSTRAP_FLAGS "-fno-stack-protector")
# Was only a C flag in src/trusted/service_runtime/build.scons.
list(APPEND BOOTSTRAP_FLAGS "-Wdeclaration-after-statement")

if (ANDROID)
	# It is is known to break the Android build, making bits/fortify/unistd.h
	# unable to find SSIZE_MAX (this may be an NDK bug).
	remove_definitions(-D_FORTIFY_SOURCE=2)
endif()

# Using optimizations when building with GCC breaks the program on armhf.
if (NOT DAEMON_CXX_COMPILER_Clang_COMPATIBILITY)
	list(APPEND BOOTSTRAP_FLAGS "-O0")
endif()

# The src/trusted/service_runtime/build.scons file was only setting them on
# Clang but GCC accepts them as well.
if (ON) # DAEMON_CXX_COMPILER_Clang_COMPATIBILITY)
	list(APPEND BOOTSTRAP_FLAGS "-ffreestanding")
	# TODO(bbudge) Remove -Qunused-arguments when Clang supports -fno-pic.
	# list(APPEND BOOTSTRAP_FLAGS "-Qunused-arguments")
	list(APPEND BOOTSTRAP_FLAGS "-Wno-builtin-macro-redefined")

	# FIXME: It doesn't work; CMake puts it in flags that is passed after the
	# definitions and then it unsets it after overwriting it, not before.
	# add_definitions("-U__STDC_HOSTED__")

	add_definitions("-D__STDC_HOSTED__=1")
endif()

set(COMPILER_OVERRIDE "${CMAKE_CXX_COMPILER}")

if (ARCH_amd64)
	set(LD_EMUL "elf_x86_64")

	if (USE_AMD64_ZERO_BASED_SANDBOX)
		# For the zero-based 64-bit sandbox, we want to reserve 44GB of address
		# space: 4GB for the program plus 40GB of guard pages. Due to a binutils
		# bug (see http://sourceware.org/bugzilla/show_bug.cgi?id=13400), the
		# amount of address space that the linker can pre-reserve is capped
		# at 4GB. For proper reservation, GNU ld version 2.22 or higher
		# needs to be used.
		#
		# Without the bug fix, trying to reserve 44GB will result in
		# pre-reserving the entire capped space of 4GB. This tricks the run-time
		# into thinking that we can mmap up to 44GB. This is unsafe as it can
		# overwrite the run-time program itself and/or other programs. Because
		# of this, we only reserve 4GB.
		#
		# TODO(arbenson): remove these comments and reserve 44GB once the
		# necessary ld version becomes standard.
		set(RESERVE_TOP "0x100000000")
		# The reserve_top value gets interpreted as a pointer in
		# linux/nacl_bootstrap.c. By default, mcmodel is set to small,
		# which restricts code and data to the first 2GB.  With
		# mcmodel set to small or medium, the reserve_top value is
		# truncated, which produces an error. With mcmodel set to large,
		# there is no restriction on the code and data, so we can
		# safely set reserve_top to 0x100000000.
		list(APPEND BOOTSTRAP_FLAGS "-mcmodel=large")
	else()
		set(RESERVE_TOP "0x0")
	endif()
elseif (ARCH_i686)
	set(LD_EMUL "elf_i386")
	set(RESERVE_TOP "0x40000000")
elseif (ARCH_armhf OR ARCH_armel)
	set(LD_EMUL "armelf_linux_eabi")
	set(RESERVE_TOP "0x40002000")

	if (DAEMON_CXX_COMPILER_Clang_COMPATIBILITY)
		if (ANDROID)
			if (ARCH_armel)
				set(COMPILER_OVERRIDE "arm-linux-androideabi-g++")
			endif()
		elseif (LINUX)
			if (ARCH_armhf)
				set(COMPILER_OVERRIDE "arm-linux-gnueabihf-g++")
			elseif (ARCH_armel)
				set(COMPILER_OVERRIDE "arm-linux-gnueabi-g++")
			endif()
		endif()
	endif()
elseif (ARCH_mipsel)
	set(LD_EMUL "elf32ltsmip")
	set(RESERVE_TOP "0x40008000")
endif()

string(REPLACE ";" " " BOOTSTRAP_FLAGS_STRING "${BOOTSTRAP_FLAGS}")

add_library(nacl_bootstrap OBJECT "nacl_bootstrap.c")
set_target_properties(nacl_bootstrap PROPERTIES COMPILE_FLAGS ${BOOTSTRAP_FLAGS_STRING})

if (USE_ARMHF_16K_PAGESIZE)
	set(MAX_PAGE_SIZE 0x4000)
else()
	set(MAX_PAGE_SIZE 0x1000)
endif()

add_custom_target(nacl_bootstrap_raw
	COMMAND env "${PYTHON}"
		"${CMAKE_CURRENT_LIST_DIR}/ld_bfd.py"
		--compiler "${COMPILER_OVERRIDE}"
		-m "${LD_EMUL}"
		--build-id
		-static
		-z "max-page-size=${MAX_PAGE_SIZE}"
		--defsym RESERVE_TOP="${RESERVE_TOP}"
		--script "${CMAKE_CURRENT_LIST_DIR}/nacl_bootstrap.x"
		-o "${CMAKE_CURRENT_BINARY_DIR}/nacl_bootstrap_raw"
		"${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/nacl_bootstrap.dir/nacl_bootstrap.c.o"
	DEPENDS nacl_bootstrap
	BYPRODUCTS "${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/nacl_bootstrap.dir/nacl_bootstrap.c.o"
)

add_custom_target(nacl_helper_bootstrap ALL
	COMMAND "${PYTHON}"
		"${CMAKE_CURRENT_LIST_DIR}/nacl_bootstrap_munge_phdr.py"
		"${CMAKE_BINARY_DIR}/src/trusted/service_runtime/linux/nacl_bootstrap_raw"
		"${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/nacl_helper_bootstrap"
	DEPENDS nacl_bootstrap_raw
	BYPRODUCTS "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/nacl_helper_bootstrap"
)
