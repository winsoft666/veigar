############################################################################
# Copyright (c) winsoft666.
# All rights reserved.
#
# This source code is licensed under the license found in the
# LICENSE file in the root directory of this source tree.
############################################################################

cmake_minimum_required(VERSION 3.4 FATAL_ERROR)
project(veigar CXX)

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake;${CMAKE_MODULE_PATH};")

option(VEIGAR_BUILD_EXAMPLES "Build examples project" ON)
option(VEIGAR_BUILD_TESTS "Build tests project" ON)
option(VEIGAR_USE_STATIC_CRT "Set to ON to build with static CRT on Windows (/MT)." OFF)

set(VEIGAR_CXX_STANDARD 11 CACHE STRING "C++ version used to build veigar (Currently: Only 11+ supported)")

if(${VEIGAR_CXX_STANDARD} LESS 11)
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
	add_subdirectory(examples/performance-test)
endif()

if(VEIGAR_BUILD_TESTS)
	add_subdirectory(tests)
endif()