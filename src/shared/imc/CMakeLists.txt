list(APPEND NACL_IMC_INPUTS "nacl_imc_common.cc")

if (DAEMON_SYSTEM_Windows)
	list(APPEND NACL_IMC_INPUTS
		"win/nacl_imc.cc"
		"win/nacl_shm.cc"
	)
else()
	list(APPEND NACL_IMC_INPUTS "posix/nacl_imc_posix.cc")

	if (DAEMON_SYSTEM_Linux_COMPATIBILITY)
		list(APPEND NACL_IMC_INPUTS "linux/nacl_imc.cc")
	elseif (DAEMON_SYSTEM_macOS)
		list(APPEND NACL_IMC_INPUTS "osx/nacl_imc.cc")
	endif()
endif()

add_library(imc STATIC ${NACL_IMC_INPUTS})
