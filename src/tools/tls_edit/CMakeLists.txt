cmake_minimum_required(VERSION 3.12)

project(native_client C CXX)

set(REPO_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../..")

list(APPEND CMAKE_MODULE_PATH "${REPO_DIR}/cmake")

include(Yokai/Detection)
include(NaClFlags)

if (NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE RelWithDebInfo)
endif()

include_directories("${REPO_DIR}/include-hax")
include_directories("${REPO_DIR}/include")

set(RDFA_VALIDATOR ON)
add_subdirectory("${REPO_DIR}/src/trusted/validator_ragel" rdfa_validator)

add_subdirectory("${REPO_DIR}/src/shared/platform" platform)
add_subdirectory("${REPO_DIR}/src/shared/gio" gio)

add_executable(tls_edit tls_edit.c)
target_link_libraries(tls_edit rdfa_validator platform gio)
install(TARGETS tls_edit DESTINATION bin)

