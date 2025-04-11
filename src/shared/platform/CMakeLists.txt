if (WIN32)
	list(APPEND PLATFORM_INPUTS
		"win/aligned_malloc.c"
		"win/condition_variable.cc"
		"win/lock.cc"
		"win/nacl_clock.c"
		"win/nacl_error.c"
		"win/nacl_exit.c"
		"win/nacl_fast_mutex.c"
		"win/nacl_find_addrsp.c"
		"win/nacl_host_desc.c"
		"win/nacl_host_dir.c"
		"win/lock_impl_win.cc"
		"win/nacl_secure_random.c"
		"win/nacl_semaphore.c"
		"win/nacl_sync_win.cc"
		"win/nacl_threads.c"
		"win/nacl_timestamp.c"
		"win/port_win.c"
		"win/xlate_system_error.c"
	)

	if (MSVC)
		list(APPEND PLATFORM_INPUTS "win/nacl_time.c")
	else()
		list(APPEND PLATFORM_INPUTS "posix/nacl_time.c")
	endif()

	if (MSVC)
		list(APPEND PLATFORM_FLAGS "/D_CRT_RAND_S")
		list(APPEND PLATFORM_FLAGS "/D_UNICODE")
		list(APPEND PLATFORM_FLAGS "/DUNICODE")
	endif()
elseif (LINUX)
	list(APPEND PLATFORM_INPUTS
		"linux/nacl_clock.c"
		"linux/nacl_host_dir.c"
		"linux/nacl_semaphore.c"
	)

#TODO:	kernel_version = list(map(int, platform.release().split('.', 2)[:2]))
#TODO:	if kernel_version < [3, 0]:
#TODO:		cputime_test_enabled = False
elseif (APPLE)
	list(APPEND PLATFORM_INPUTS
		"osx/nacl_clock.c"
		"osx/nacl_host_dir.c"
		"osx/nacl_semaphore.c"
	)
endif()

# It was doing: if env.Bit('posix'):
if (LINUX OR APPLE)
	list(APPEND PLATFORM_INPUTS
		"posix/aligned_malloc.c"
		"posix/condition_variable.c"
		"posix/lock.c"
		"posix/nacl_error.c"
		"posix/nacl_exit.c"
		"posix/nacl_fast_mutex.c"
		"posix/nacl_file_lock.c" # only used on Linux, but built/tested on Posix
		"posix/nacl_find_addrsp.c"
		"posix/nacl_host_desc.c"
		"posix/nacl_secure_random.c"
		"posix/nacl_thread_id.c"
		"posix/nacl_threads.c"
		"posix/nacl_time.c"
		"posix/nacl_timestamp.c"
	)
endif()

list(APPEND PLATFORM_INPUTS
	"nacl_check.c"
	"nacl_global_secure_random.c"
	"nacl_host_desc_common.c"
	"nacl_interruptible_condvar.c"
	"nacl_interruptible_mutex.c"
	"nacl_log.c"
	"nacl_secure_random_common.c"
	"nacl_sync_checked.c"
	"nacl_time_common.c"
	"platform_init.c"
	"refcount_base.cc"
)

add_library(platform STATIC ${PLATFORM_INPUTS})
string(REPLACE ";" " " PLATFORM_FLAGS_STRING "${PLATFORM_FLAGS}")
set_target_properties(platform PROPERTIES COMPILE_FLAGS "${PLATFORM_FLAGS}")
