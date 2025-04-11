# Copyright (c) 2012 The Native Client Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

set(GENERATED_ROOT "${CMAKE_BINARY_DIR}/gen")
set(GENERATED "${GENERATED_ROOT}/native_client/src/trusted/service_runtime")

# ----------------------------------------------------------
# TODO(robertm): this library is too big and needs to be split up
#                for easier unit testing
list(APPEND LDR_INPUTS
	"dyn_array.c"
	"elf_util.c"
	"filename_util.cc"
	"load_file.c"
	"nacl_all_modules.c"
	"nacl_app_thread.c"
	"nacl_copy.c"
	"nacl_desc_effector_ldr.c"
	"nacl_error_gio.c"
	"nacl_error_log_hook.c"
	"nacl_globals.c"
	"nacl_resource.c"
	"nacl_signal_common.c"
	"nacl_stack_safety.c"
	"nacl_syscall_common.c"
	"nacl_syscall_hook.c"
	"nacl_syscall_list.c"
	"nacl_text.c"
	"nacl_valgrind_hooks.c"
	"sel_addrspace.c"
	"sel_ldr.c"
	"sel_ldr_filename.cc"
	"sel_ldr_standard.c"
	"sel_main_common.c"
	"sel_mem.c"
	"sel_qualify.c"
	"sel_validate_image.c"
	"sys_clock.c"
	"sys_exception.c"
	"sys_fdio.c"
	"sys_filename.c"
	"sys_futex.c"
	"sys_imc.c"
	"sys_list_mappings.c"
	"sys_memory.c"
	"sys_parallel_io.c"
	"sys_random.c"
	"thread_suspension_common.c"
	"thread_suspension_unwind.c"
)

if (ARCH_i686)
	list(APPEND LDR_INPUTS
		"arch/x86/nacl_ldt_x86.c"
		"arch/x86_32/nacl_app_32.c"
		"arch/x86_32/nacl_switch_32.S"
		"arch/x86_32/nacl_switch_all_regs_32.c"
		"arch/x86_32/nacl_switch_all_regs_asm_32.S"
		"arch/x86_32/nacl_switch_to_app_32.c"
		"arch/x86_32/nacl_syscall_32.S"
		"arch/x86_32/nacl_tls_32.c"
		"arch/x86_32/sel_addrspace_x86_32.c"
		"arch/x86_32/sel_ldr_x86_32.c"
		"arch/x86_32/sel_rt_32.c"
		"arch/x86_32/springboard.S"
		"arch/x86_32/tramp_32.S"
	)
elseif (ARCH_amd64)
	list(APPEND LDR_INPUTS
		"arch/x86/nacl_ldt_x86.c"
		"arch/x86_64/nacl_app_64.c"
		"arch/x86_64/nacl_switch_64.S"
		"arch/x86_64/nacl_switch_to_app_64.c"
		"arch/x86_64/nacl_syscall_64.S"
		"arch/x86_64/nacl_tls_64.c"
		"arch/x86_64/sel_ldr_x86_64.c"
		"arch/x86_64/sel_rt_64.c"
		"arch/x86_64/tramp_64.S"
	)

	if (WIN32)
		# We assemble the .asm assembly file with the Microsoft assembler
		# because we need to generate x86-64 Windows unwind info, which
		# the GNU assembler we use elsewhere does not support.

		list(APPEND LDR_INPUTS
			"arch/x86_64/sel_addrspace_win_x86_64.c"
			"arch/x86_64/fnstcw.S"
			"arch/x86_64/fxsaverstor.S"
		)

		if (NOT FORCE_NO_TRUSTED_BUILD)
			# This use of win64_asm_env causes an invocation of MSVC, even if sel_ldr
			# would not otherwise be actually built. So don't include it if we aren't
			# actually building sel_ldr
			add_library(nacl_switch_unwind_win OBJECT "arch/x86_64/nacl_switch_unwind_win.asm")
			list(APPEND LDR_LIBS nacl_switch_unwind_win)
		endif()
	else()
		list(APPEND LDR_INPUTS "arch/x86_64/sel_addrspace_posix_x86_64.c")

		if (USE_AMD64_ZERO_BASED_SANDBOX)
			add_definitions(-DNACL_X86_64_ZERO_BASED_SANDBOX=1)
		else()
			add_definitions(-DNACL_X86_64_ZERO_BASED_SANDBOX=0)
		endif()
	endif()
elseif (ARCH_armhf OR ARCH_armel)
	list(APPEND LDR_INPUTS
		"arch/arm/nacl_app.c"
		"arch/arm/nacl_switch_to_app_arm.c"
		"arch/arm/sel_rt.c"
		"arch/arm/nacl_tls.c"
		"arch/arm/sel_ldr_arm.c"
		"arch/arm/sel_addrspace_arm.c"
		"arch/arm/nacl_switch.S"
		"arch/arm/nacl_syscall.S"
		"arch/arm/tramp_arm.S"
	)
elseif (ARCH_mipsel)
	list(APPEND LDR_INPUTS
		"arch/mips/nacl_app.c"
		"arch/mips/nacl_switch_to_app_mips.c"
		"arch/mips/sel_rt.c"
		"arch/mips/nacl_tls.c"
		"arch/mips/sel_ldr_mips.c"
		"arch/mips/sel_addrspace_mips.c"
		"arch/mips/nacl_switch.S"
		"arch/mips/nacl_syscall.S"
		"arch/mips/tramp_mips.S"
	)
endif()

if (WIN32)
	list(APPEND LDR_INPUTS
		"win/addrspace_teardown.c"
		"win/nacl_ldt.c"
		"win/nacl_thread_nice.c"
		"win/sel_memory.c"
	)

	if (MSVC)
		list(APPEND LDR_INPUTS "win/sel_segments.c")
	else()
		list(APPEND LDR_INPUTS "posix/x86/sel_segments.c")
	endif()
elseif (APPLE)
	# Rely on the c preprocessor to discover where the mach interface definitions
	# are located.
	set(GENERATED_NACL_EXC_C "${GENERATED}/nacl_exc.c")
	set(GENERATED_NACL_EXC_H "${GENERATED}/nacl_exc.h")
	set(GENERATED_NACL_EXC_SERVER_C "${GENERATED}/nacl_exc_server.c")
	set(GENERATED_NACL_EXC_DEFS "${GENERATED}/exc.defs")

	file(WRITE "${GENERATED_NACL_EXC_C}" "#include <mach/exc.defs>")

	# It works with clang but not with default cc (which is clang).
	execute_process(
		COMMAND "clang" "${CMAKE_C_COMPILER_FLAGS}" -E "${GENERATED_NACL_EXC_C}"
		OUTPUT_FILE "${GENERATED_NACL_EXC_DEFS}"
	)

	include_directories("${GENERATED_ROOT}")

	execute_process(
		COMMAND "${PYTHON}"
			"${CMAKE_CURRENT_LIST_DIR}/osx/run_mig.py"
			"${GENERATED_NACL_EXC_DEFS}"
			"${GENERATED_NACL_EXC_H}"
			"${GENERATED_NACL_EXC_SERVER_C}"
	)

	list(APPEND LDR_INPUTS
		"${GENERATED_NACL_EXC_SERVER_C}"
		"osx/crash_filter.c"
		"osx/mach_exception_handler.c"
		"osx/mach_thread_map.c"
		"osx/nacl_ldt.c"
		"osx/nacl_thread_nice.c"
		"posix/addrspace_teardown.c"
		"posix/sel_memory.c"
		"posix/x86/sel_segments.c"
	)
elseif (LINUX)
	list(APPEND LDR_INPUTS
		"linux/nacl_bootstrap_args.c"
		"linux/nacl_thread_nice.c"
		"linux/r_debug.c"
		"linux/reserved_at_zero.c"
		"posix/addrspace_teardown.c"
		"posix/sel_memory.c"
	)

	if (ARCH_i686 OR ARCH_amd64)
		list(APPEND LDR_INPUTS
			"linux/x86/nacl_ldt.c"
			"posix/x86/sel_segments.c"
		)
	elseif (ARCH_armhf OR ARCH_armel)
		list(APPEND LDR_INPUTS "linux/arm/sel_segments.c")
	elseif (ARCH_mipsel)
		list(APPEND LDR_INPUTS "linux/mips/sel_segments.c")
	endif()
endif()

# -------------------------------------------------------------
# Add OS and architecture specific signal handling files.
#
if (WIN32)
	list(APPEND LDR_INPUTS
		"win/debug_exception_handler.c"
		"win/debug_exception_handler_standalone.c"
		"win/nacl_signal_stack.c"
		"win/thread_handle_map.c"
		"win/thread_suspension.c"
		"win/sel_addrspace_win.c"
	)

	if (ARCH_i686)
		list(APPEND LDR_INPUTS "win/nacl_signal_32.c")
	elseif (ARCH_amd64)
		list(APPEND LDR_INPUTS
			"win/exception_patch/exit_fast.S"
			"win/exception_patch/intercept.S"
			"win/exception_patch/ntdll_patch.c"
			"win/nacl_signal_64.c"
		)
	else()
		message(FATAL_ERROR "Unsupported target")
	endif()
endif()

if (LINUX)
	list(APPEND LDR_INPUTS
		"linux/thread_suspension.c"
		"posix/nacl_signal_stack.c"
		"posix/sel_addrspace_posix.c"
	)

	if (ARCH_armhf OR ARCH_armel)
		list(APPEND LDR_INPUTS "linux/nacl_signal_arm.c")
	elseif (ARCH_mipsel)
		list(APPEND LDR_INPUTS "linux/nacl_signal_mips.c")
	elseif (ARCH_i686)
		list(APPEND LDR_INPUTS "linux/nacl_signal_32.c")
	elseif (ARCH_amd64)
		list(APPEND LDR_INPUTS "linux/nacl_signal_64.c")
	else()
		message(FATAL_ERROR "Unsupported target")
	endif()

	add_library(nacl_signal STATIC "linux/nacl_signal.c")
	list(APPEND LDR_LIBS nacl_signal)
#TODO:	nacl_signal_env = env.Clone()
	if (ARCH_i686)
		# nacl_signal.c needs to be compiled without the stack protector
		# on i386.
		# See https://code.google.com/p/nativeclient/issues/detail?id=3581.
#TODO:		nacl_signal_env.FilterOut(CCFLAGS=['-fstack-protector', '-fstack-protector-all'])
		set_target_properties(nacl_signal PROPERTIES COMPILE_FLAGS "-fno-stack-protector")
	endif()
endif()

if (APPLE)
	list(APPEND LDR_INPUTS
		"osx/thread_suspension.c"
		"posix/nacl_signal_stack.c"
		"posix/sel_addrspace_posix.c"
	)

	if (ARCH_i686)
		list(APPEND LDR_INPUTS "osx/nacl_signal_32.c")
	elseif (ARCH_amd64)
		list(APPEND LDR_INPUTS "osx/nacl_signal_64.c")
	else()
		message(FATAL_ERROR "Unsupported target")
	endif()
endif()

if (WIN32)
	list(APPEND LDR_INPUTS "win/vm_hole.c")
else()
	list(APPEND LDR_INPUTS "generic/vm_hole.c")
endif()

#TODO: syscall_gen_flags = '-a ${ARCHITECTURE} -s ${TARGET_SUBARCH}'

#TODO: env.Append(SYSCALL_GEN_FLAGS=syscall_gen_flags)

add_library(sel OBJECT ${LDR_INPUTS})
list(APPEND LDR_LIBS sel)

add_library(sel_main OBJECT "sel_main.c")
list(APPEND LDR_LIBS sel_main)

# add_library(sel_main_chrome OBJECT "sel_main_chrome.c")

add_library(env_cleanser OBJECT "env_cleanser.c")
list(APPEND LDR_LIBS env_cleanser)

add_library(nacl_error_code OBJECT "nacl_error_code.c")
list(APPEND LDR_LIBS nacl_error_code)

list(APPEND LDR_LIBS debug_stub) # not listed in src/trusted/service_runtime/build.scons
list(APPEND LDR_LIBS nrd_xfer)
list(APPEND LDR_LIBS nacl_perf_counter)
list(APPEND LDR_LIBS nacl_base)
list(APPEND LDR_LIBS imc)
list(APPEND LDR_LIBS nacl_fault_inject)
list(APPEND LDR_LIBS nacl_interval)
list(APPEND LDR_LIBS platform)
list(APPEND LDR_LIBS gio) # not listed in src/trusted/service_runtime/build.scons
list(APPEND LDR_LIBS platform_qual_lib)
list(APPEND LDR_LIBS validators)

if (ARCH_i686 OR ARCH_amd64)
	list(APPEND LDR_LIBS dfa_validate_caller)
elseif (ARCH_armhf OR ARCH_armel)
	list(APPEND LDR_LIBS ncvalidate_arm_v2)
	list(APPEND LDR_LIBS arm_validator_reporters)
elseif (ARCH_mipsel)
	list(APPEND LDR_LIBS ncvalidate_mips)
endif()

if (ARCH_i686 OR ARCH_amd64)
	list(APPEND LDR_LIBS nccopy) # not listed in src/trusted/service_runtime/build.scons
endif()

list(APPEND LDR_LIBS cpu_features) # not listed in src/trusted/service_runtime/build.scons

if (WIN32)
	set(MMAP_TEST_CHECK_CC "win/mmap_test_check.cc")
elseif (APPLE)
	set(MMAP_TEST_CHECK_CC "osx/mmap_test_check.cc")
elseif (LINUX)
	set(MMAP_TEST_CHECK_CC "linux/mmap_test_check.cc")
else()
	message(FATAL_ERROR "Unsupported host OS")
endif()

add_library(sel_test "${MMAP_TEST_CHECK_CC}")
list(APPEND LDR_LIBS sel_test)

# NOTE(robertm): these extra libs were orignially only added to the
#                sel_ldr binary
# TODO(robertm): see who really needs them and remove
if (WIN32)
# FIXME: Unused for now.
	set(LIBS
		ws2_32
		kernel32
		advapi32
		winmm
		# TODO(gregoryd): ntdll.lib is required for sem_get_value implementation but
		# it is available in Windows DDK only. The DDK is not
		# in third_party, but we might need to add it if we want to use it.
		# ntdll
	)
endif()

if (NOT COVERAGE_ENABLED OR NOT WIN32)
#TODO:	sel_main_objs = [env.ComponentObject('nacl_test_injection_main.c')]
	add_library(nacl_test_injection_main OBJECT "nacl_test_injection_main.c")
	list(APPEND LDR_LIBS nacl_test_injection_main)

	if (LINUX)
		# FIXME: librt was added by src/shared/imc/build.scons for Linux but not Android.
		# FIXME: libdl was added by (root)/SConstruct for Linux on armhf and Android on armel.
		# get_plugin_dirname.cc has a dependency on dladdr
		list(APPEND LDR_LIBS "-ldl")
		list(APPEND LDR_LIBS "-lrt")
		list(APPEND LDR_LIBS "-pthread")
	endif()

	add_executable(sel_ldr)
	target_link_libraries(sel_ldr ${LDR_LIBS})

	if (APPLE)
		# This target exists only to check that the service_runtime code
		# can successfully be linked into a Mac OS X dynamic library.  Our
		# assembly code needs to be PIC-friendly and linkable in this
		# context, because it is linked into a dynamic library inside
		# Chromium, and OS X is strict about TEXTRELs.  Without this, the
		# standalone build won't catch some mistakes that can break the
		# Chromium build.  Linking a dylib here works because -fPIC is the
		# default for all C code on OS X.
#TODO:		dylib_env = env.Clone()
#TODO:		dylib_env.Append(LINKFLAGS=['-bundle'])
#TODO:		dylib_env.ComponentProgram('dummy_sel_ldr.dylib', sel_main_objs, EXTRA_LIBS=['sel_main'])
	endif()

  # NOTE: we do not have segments on ARM
endif()

if (LINUX AND ARCH_amd64)
#TODO:	sel_ldr_seccomp_node = env.ComponentProgram('sel_ldr_seccomp',
#TODO:		['sel_ldr_seccomp_main.c'],
#TODO:		EXTRA_LIBS=['sel_main',
#TODO:			'seccomp_bpf'])
#TODO:	env.SDKInstallBin('sel_ldr_seccomp', sel_ldr_seccomp_node)
endif()
