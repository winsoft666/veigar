############################################################################
#    Veigar: Cross platform RPC library using shared memory.
#    -----------------------------------------------------------------------
#    Copyright (C), winsoft666, <winsoft666@outlook.com>.
#
############################################################################

cmake_minimum_required(VERSION 3.4 FATAL_ERROR)
project(veigar CXX)

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake;${CMAKE_MODULE_PATH};")

option(VEIGAR_BUILD_EXAMPLES "Build examples project" ON)
option(VEIGAR_BUILD_TESTS "Build tests project" ON)
option(VEIGAR_USE_STATIC_CRT "Set to ON to build with static CRT on Windows (/MT)." OFF)

set(VEIGAR_CXX_STANDARD 11 CACHE STRING "C++ version used to build veigar (Currently: Only 11+ supported)")

if(NOT ${VEIGAR_CXX_STANDARD} EQUAL 14 AND NOT ${VEIGAR_CXX_STANDARD} EQUAL 11)
  message(fatal_error "Unsupported C++ standard: ${VEIGAR_CXX_STANDARD}")
endif()

set(CMAKE_CXX_STANDARD ${RPCLIB_CXX_STANDARD})

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)


set(DEBUG_LIBNAME_SUFFIX "-d" 
	CACHE STRING "Optional suffix to append to the library name for a debug build")
mark_as_advanced(DEBUG_LIBNAME_SUFFIX)

# Unicode Support
add_definitions(-DUNICODE -D_UNICODE -DNOMINMAX)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

include_directories(./include)

add_subdirectory(src)

if(VEIGAR_BUILD_EXAMPLES)
	add_subdirectory(examples/echo)
endif()

if(VEIGAR_BUILD_TESTS)
	add_subdirectory(tests)
endif()