list(APPEND NACL_IMC_INPUTS "nacl_imc_common.cc")

if (YOKAI_TARGET_SYSTEM_WINDOWS)
	list(APPEND NACL_IMC_INPUTS
		"win/nacl_imc.cc"
		"win/nacl_shm.cc"
	)
else()
	list(APPEND NACL_IMC_INPUTS "posix/nacl_imc_posix.cc")

	if (YOKAI_TARGET_SYSTEM_LINUX_COMPATIBILITY)
		list(APPEND NACL_IMC_INPUTS "linux/nacl_imc.cc")
	elseif (YOKAI_TARGET_SYSTEM_MACOS)
		list(APPEND NACL_IMC_INPUTS "osx/nacl_imc.cc")
	endif()
endif()

add_library(imc STATIC ${NACL_IMC_INPUTS})
