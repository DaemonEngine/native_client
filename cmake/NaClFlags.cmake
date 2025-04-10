option(USE_WERROR "Tell the compiler to make the build fail when warnings are present." OFF)

if (NOT MSVC)
	option(USE_STATIC_LIBS "Tries to use static libs where possible." OFF)
endif()

if (ARCH_armhf)
	option(USE_ARMHF_16K_PAGESIZE "Build armhf binaries with 16K PageSize." OFF)
	list(APPEND INHERITED_OPTIONS "USE_ARMHF_16K_PAGESIZE")
endif()

macro(set_ASM_flag FLAG)
	set(lang ASM)
	if (${ARGC} GREATER 1)
		set(CMAKE_${lang}_FLAGS_${ARGV1} "${CMAKE_${lang}_FLAGS_${ARGV1}} ${FLAG}")
	else()
		set(CMAKE_${lang}_FLAGS "${CMAKE_${lang}_FLAGS} ${FLAG}")
	endif()
endmacro()

macro(set_C_flag FLAG)
	set(lang C)
	if (${ARGC} GREATER 1)
		set(CMAKE_${lang}_FLAGS_${ARGV1} "${CMAKE_${lang}_FLAGS_${ARGV1}} ${FLAG}")
	else()
		set(CMAKE_${lang}_FLAGS "${CMAKE_${lang}_FLAGS} ${FLAG}")
	endif()
endmacro()

macro(set_CXX_flag FLAG)
	set(lang CXX)
	if (${ARGC} GREATER 1)
		set(CMAKE_${lang}_FLAGS_${ARGV1} "${CMAKE_${lang}_FLAGS_${ARGV1}} ${FLAG}")
	else()
		set(CMAKE_${lang}_FLAGS "${CMAKE_${lang}_FLAGS} ${FLAG}")
	endif()
endmacro()

macro(set_compiler_flag FLAG)
	foreach(lang C CXX ASM)
		if (${ARGC} GREATER 1)
			set(CMAKE_${lang}_FLAGS_${ARGV1} "${CMAKE_${lang}_FLAGS_${ARGV1}} ${FLAG}")
		else()
			set(CMAKE_${lang}_FLAGS "${CMAKE_${lang}_FLAGS} ${FLAG}")
		endif()
	endforeach()
endmacro()

macro(set_EXE_linker_flag FLAG)
	set(kind EXE)
	if (${ARGC} GREATER 1)
		set(CMAKE_${kind}_LINKER_FLAGS_${ARGV1} "${CMAKE_${kind}_LINKER_FLAGS_${ARGV1}} ${FLAG}")
	else()
		set(CMAKE_${kind}_LINKER_FLAGS "${CMAKE_${kind}_LINKER_FLAGS} ${FLAG}")
	endif()
endmacro()

macro(set_SHARED_linker_flag FLAG)
	set(kind SHARED)
	if (${ARGC} GREATER 1)
		set(CMAKE_${kind}_LINKER_FLAGS_${ARGV1} "${CMAKE_${kind}_LINKER_FLAGS_${ARGV1}} ${FLAG}")
	else()
		set(CMAKE_${kind}_LINKER_FLAGS "${CMAKE_${kind}_LINKER_FLAGS} ${FLAG}")
	endif()
endmacro()

macro(set_MODULE_linker_flag FLAG)
	set(kind MODULE)
	if (${ARGC} GREATER 1)
		set(CMAKE_${kind}_LINKER_FLAGS_${ARGV1} "${CMAKE_${kind}_LINKER_FLAGS_${ARGV1}} ${FLAG}")
	else()
		set(CMAKE_${kind}_LINKER_FLAGS "${CMAKE_${kind}_LINKER_FLAGS} ${FLAG}")
	endif()
endmacro()

macro(set_DLL_linker_flag FLAG)
	foreach(kind SHARED MODULE)
		if (${ARGC} GREATER 1)
			set(CMAKE_${kind}_LINKER_FLAGS_${ARGV1} "${CMAKE_${kind}_LINKER_FLAGS_${ARGV1}} ${FLAG}")
		else()
			set(CMAKE_${kind}_LINKER_FLAGS "${CMAKE_${kind}_LINKER_FLAGS} ${FLAG}")
		endif()
	endforeach()
endmacro()

macro(set_linker_flag FLAG)
	foreach(kind EXE SHARED MODULE)
		if (${ARGC} GREATER 1)
			set(CMAKE_${kind}_LINKER_FLAGS_${ARGV1} "${CMAKE_${kind}_LINKER_FLAGS_${ARGV1}} ${FLAG}")
		else()
			set(CMAKE_${kind}_LINKER_FLAGS "${CMAKE_${kind}_LINKER_FLAGS} ${FLAG}")
		endif()
	endforeach()
endmacro()

if (USE_STATIC_LIBS)
	set_compiler_flag("-static")
	set_linker_flag("-static")
endif()

if (USE_ARMHF_16K_PAGESIZE)
	set_linker_flag("-Wl,-z,max-page-size=16384")
endif()

#TODO: Import from SetUpClang() from (root)/SConstruct.
#TODO: This is mostly ASAN configurations.

# From MakeUnixLikeEnv() from (root)/SConstruct.
if (LINUX OR APPLE OR MINGW)
	set_C_flag("-std=gnu99")
	# -Wdeclaration-after-statement is desirable because MS studio does
	# not allow declarations after statements in a block, and since much
	# of our code is portable and primarily initially tested on Linux,
	# it'd be nice to get the build error earlier rather than later
	# (building and testing on Linux is faster).
	# TODO(nfullagar): should we consider switching to -std=c99 ?
	set_C_flag("-Wdeclaration-after-statement")
	# Require defining functions as "foo(void)" rather than
	# "foo()" because, in C (but not C++), the latter defines a
	# function with unspecified arguments rather than no
	# arguments.
	set_C_flag("-Wstrict-prototypes")

	# set_compiler_flag("-malign-double")
	set_compiler_flag("-Wall")
	set_compiler_flag("-pedantic")
	set_compiler_flag("-Wextra")
	set_compiler_flag("-Wno-long-long")
	set_compiler_flag("-Wswitch-enum")
	set_compiler_flag("-Wsign-compare")
	set_compiler_flag("-Wundef")
	set_compiler_flag("-fdiagnostics-show-option")
	set_compiler_flag("-fvisibility=hidden")
	set_compiler_flag("-fstack-protector")

	# NOTE: pthread is only neeeded for libppNaClPlugin.so and on arm
#TODO:	LIBS = ['pthread']

	add_definitions(-D__STDC_LIMIT_MACROS=1)
	add_definitions(-D__STDC_FORMAT_MACROS=1)

	if (NOT ANDROID)
		set_CXX_flag("-std=c++98")
	endif()

	if (NOT DAEMON_CXX_COMPILER_Clang_COMPATIBILITY)
		set_compiler_flag("--param ssp-buffer-size=4")
	endif()

	if (USE_WERROR)
		# It was only set on linker flag in (root)SConstruct.
		set_linker_flag("-Werror")
	endif()
endif()

# From MakeWindowsEnv() from (root)/SConstruct.
if (WIN32)
	# Windows /SAFESEH linking requires either an .sxdata section be
	# present or that @feat.00 be defined as a local, absolute symbol
	# with an odd value.
#TODO:	ASCOM = ('$ASPPCOM /E /D__ASSEMBLER__ | '
#TODO:		'$WINASM -defsym @feat.00=1 -o $TARGET'),
	add_definitions("-D_WIN32_WINNT=0x0501")
	add_definitions("-D__STDC_LIMIT_MACROS=1")
	add_definitions("-DNOMINMAX=1")
	# WIN32 is used by ppapi
	add_definitions("-DWIN32=1")
	# WIN32_LEAN_AND_MEAN tells windows.h to omit obsolete and rarely
	# used #include files. This allows use of Winsock 2.0 which otherwise
	# would conflict with Winsock 1.x included by windows.h.
	add_definitions("-DWIN32_LEAN_AND_MEAN=1")

	if (MSVC)
		# TODO(bsy) remove 4355 once cross-repo
		# NACL_ALLOW_THIS_IN_INITIALIZER_LIST changes go in.
		set_compiler_flag("/EHsc")
		set_compiler_flag("/Wx")
		set_compiler_flag("/wd4355")
		set_compiler_flag("/wd4800")

		if (ARCH_i686)
			# This linker option allows us to ensure our builds are compatible with
			# Chromium, which uses it.
#TODO: disabled until ASCOM is configured.
#TODO:			set_linker_flag("safeseh")
		endif()

		# We use the GNU assembler (gas) on Windows so that we can use the
		# same .S assembly files on all platforms.  Microsoft's assembler uses
		# a completely different syntax for x86 code.
#FIXME: Use x86_64-w64-mingw32-as.exe or x86_64-w32-mingw32-as.exe on MSVC for .S files.
	endif()
endif()

# From MakeMacEnv() from (root)/SConstruct.
if (APPLE)
	set(MAC_DEPLOYMENT_TARGET "10.6")
	set(MAC_SDK_FLAG "-mmacosx-version-min=${MAC_DEPLOYMENT_TARGET}")

	set_compiler_flag(${MAC_SDK_FLAG})
	set_linker_flag(${MAC_SDK_FLAG})

	set_compiler_flag("-fPIC")
	set_linker_flag("-fPIC")
	set_compiler_flag("-Wno-gnu")
	set_linker_flag("-stdlib=libc++")
endif()

# From SetUpLinuxEnvArm() from (root)/SConstruct.
if (LINUX AND ARCH_armhf)
	# The -rpath-link argument is needed on Ubuntu/Precise to
	# avoid linker warnings about missing ld.linux.so.3.
	# TODO(sbc): remove this once we stop supporting Precise
	# as a build environment.
	# We (Dæmon) don't support Precise.
	# set_linker_flag("-fuse-ld=lld")

	if (DAEMON_CXX_COMPILER_Clang_COMPATIBILITY)
		# If ARCH is "armhf", then the target is already set properly by the toolchain.
		# set_compiler_flag("--target=arm-linux-gnueabihf")
		# set_linker_flag("--target=arm-linux-gnueabihf")
	endif()

	set_compiler_flag("-march=armv7-a")
	set_compiler_flag("-mtune=generic-armv7-a")
	set_compiler_flag("-mfpu=neon")
	# If ARCH is "armhf", then the float ABI is already set properly by the toolchain.
	# set_compiler_flag("-mfloat-abi=hard")
endif()

# Partially from SetUpAndroidEnv() from (root)/SConstruct.
if (LINUX AND ARCH_armel)
	set_compiler_flag("-march=armv7-a")
	set_compiler_flag("-mfpu=vfp")
	set_compiler_flag("-mfloat-abi=softfp")
endif()

#TODO: Import from SetUpAndroidEnv() from (root)/SConstruct.
if (ANDROID)
#TODO:	LIBS=['stlport_shared',
#TODO:		'gcc',
#TODO:		'c',
#TODO:		'dl',
#TODO:		'm',
#TODO:	],

#TODO:	env.Append(CCFLAGS=[
#TODO:	'-I%s' % android_stlport_include,
#TODO:	CXXFLAGS=['-I%s' % android_stlport_include,
#TODO:	'-I%s' % android_ndk_include,

	# (root)/SConstruct was just setting -DANDROID without any value.
	# set_compiler_definition("-DANDROID")
	# The compiler sets it, we better trust it.
	# set_compiler_definition("-D__ANDROID__")

	set_compiler_flag("-ffunction-sections")
	set_compiler_flag("-g")
	set_compiler_flag("-fstack-protector")
	set_compiler_flag("-fno-short-enums")
	set_compiler_flag("-finline-limit=64")
	set_compiler_flag("-Wa,--noexecstack")
	# Due to bogus warnings on uintptr_t formats.
	set_compiler_flag("-Wno-format")
	
	set_CXX_flag("-fno-exceptions")

	# Copied from (root)SConsctruct, break build in Termux,
	# produces undefined symbols in std::string.
	# set_linker_flag("-nostdlib")

	set_linker_flag("-Wl,--no-undefined")
	# Don't export symbols from statically linked libraries.
	set_linker_flag("-Wl,--exclude-libs=ALL")

	# crtbegin_dynamic.o should be the last item in ldflags.
	# Already done by the toolchain, same for crtend_android.o.
	# crtbegin_so.o should be the last item in ldflags.
	# Already done by the toolchain, same for crtend_so.o.
endif()

# From SetUpLinuxEnvMips() from (root)/SConstruct.
if (LINUX AND ARCH_mipsel)
#TODO:	env.Append(LIBS=['rt', 'dl', 'pthread']
	set_compiler_flag("-march=mips32r2")
	# Because of:
	# src/trusted/service_runtime/arch/mips/nacl_switch.S: Assembler messages:
	# src/trusted/service_runtime/arch/mips/nacl_switch.S:38: Error: float register should be even, was 1
	# src/trusted/service_runtime/arch/mips/nacl_switch.S:72: Error: float register should be even, was 1
	set_compiler_flag("-mfp32")
endif()

# From MakeGenericLinuxEnv() from (root)/SConstruct.
if (LINUX)
	add_definitions(-D_POSIX_C_SOURCE=199506)
	add_definitions(-D_XOPEN_SOURCE=600)
	add_definitions(-D_GNU_SOURCE=1)
	add_definitions(-D_FORTIFY_SOURCE=2)

	if (NOT ANDROID)
		# Disabled in (root)/SConstruct.
		add_definitions(-D_LARGEFILE64_SOURCE=1)
		# Android complains about not finding -lc++_shared.
		set_linker_flag("-static-libstdc++")
	endif()

	set_linker_flag("-Wl,-z,relro")
	set_linker_flag("-Wl,-z,now")
	set_linker_flag("-Wl,-z,noexecstack")
	set_compiler_flag("-fPIE")
	set_linker_flag("-pie")
endif()

if (ARCH_i686)
	set_compiler_flag("-msse2")
endif()
