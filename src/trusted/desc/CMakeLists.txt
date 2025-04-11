add_library(nrd OBJECT "nrd_xfer.c")
list(APPEND NRD_XFER_LIBS nrd)

if (NOT WIN32)
	# Use those flags only with the nrd target.
	list(APPEND NRD_FLAGS "-fno-strict-aliasing") # This was only a C flag in build.scons
	list(APPEND NRD_FLAGS "-Wno-missing-field-initializers")
	string(REPLACE ";" " " NRD_FLAGS_STRING "${NRD_FLAGS}")
	set_target_properties(nrd PROPERTIES COMPILE_FLAGS "${NRD_FLAGS_STRING}")
endif()

list(APPEND NRD_LIB_INPUTS
	"nacl_desc_base.c"
	"nacl_desc_cond.c"
	"nacl_desc_custom.c"
	"nacl_desc_dir.c"
	"nacl_desc_effector_trusted_mem.c"
	"nacl_desc_imc.c"
	"nacl_desc_imc_shm.c"
	"nacl_desc_invalid.c"
	"nacl_desc_io.c"
	"nacl_desc_mutex.c"
	"nacl_desc_null.c"
	"nacl_desc_quota.c"
	"nacl_desc_quota_interface.c"
	"nacl_desc_semaphore.c"
	"nacl_desc_sync_socket.c"
	"nrd_all_modules.c"
)

if (WIN32)
	list(APPEND NRD_LIB_INPUTS "win/nacl_desc.c")
else()
	list(APPEND NRD_LIB_INPUTS "posix/nacl_desc.c")
endif()

if (APPLE)
	list(APPEND NRD_LIB_INPUTS "osx/nacl_desc_imc_shm_mach.c")
endif()

if (WIN32)
	list(APPEND NRD_LIB_INPUTS
		"nacl_desc_conn_cap.c"
		"nacl_desc_imc_bound_desc.c"
		"nacl_makeboundsock.c"
	)
else()
	list(APPEND NRD_LIB_INPUTS
		"posix/nacl_desc_conn_cap.c"
		"posix/nacl_desc_imc_bound_desc.c"
		"posix/nacl_makeboundsock.c"
	)
endif()

add_library(nrd_xfer STATIC ${NRD_LIB_INPUTS})
target_link_libraries(nrd_xfer ${NRD_XFER_LIBS} platform nacl_base)
