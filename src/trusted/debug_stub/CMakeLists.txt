list(APPEND DEBUG_INPUTS
	"abi.cc"
	"debug_stub.cc"
	"nacl_debug.cc"
	"packet.cc"
	"session.cc"
	"target.cc"
	"thread_common.cc"
	"transport_common.cc"
	"transport_ipc.cc"
	"util.cc"
)

if (YOKAI_TARGET_SYSTEM_WINDOWS)
	list(APPEND DEBUG_INPUTS
		"win/debug_stub_win.cc"
		"win/platform_impl.cc"
		"win/thread_impl.cc"
	)
else()
	list(APPEND DEBUG_INPUTS
		"posix/debug_stub_posix.cc"
		"posix/platform_impl.cc"
		"posix/thread_impl.cc"
	)
endif()

add_library(debug_stub STATIC ${DEBUG_INPUTS})

if (YOKAI_CXX_COMPILER_MINGW)
	target_link_libraries(debug_stub ws2_32)
endif()
