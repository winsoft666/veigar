############################################################################
# Copyright (c) winsoft666.
# All rights reserved.
#
# This source code is licensed under the license found in the
# LICENSE file in the root directory of this source tree.
############################################################################

cmake_minimum_required(VERSION 3.16 FATAL_ERROR)
project(veigar 
	VERSION 1.5
	LANGUAGES CXX
)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

option(VEIGAR_BUILD_EXAMPLES "Build examples project" ON)
option(VEIGAR_BUILD_TESTS "Build test project" ON)
option(VEIGAR_BUILD_SHARED_LIBS "Build shared libraries" OFF)
option(VEIGAR_USE_STATIC_CRT "Set to ON to build with static CRT on Windows (/MT)." OFF)

set(VEIGAR_CXX_STANDARD 11 CACHE STRING "C++ version used to build veigar (Currently: Only 11+ supported)")

if(${VEIGAR_CXX_STANDARD} LESS 11)
  message(fatal_error "Unsupported C++ standard: ${VEIGAR_CXX_STANDARD}")
endif()

set(CMAKE_CXX_STANDARD ${VEIGAR_CXX_STANDARD})

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

if (MSVC AND VEIGAR_USE_STATIC_CRT)
	set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

add_subdirectory(src)

if(VEIGAR_BUILD_EXAMPLES)
	add_subdirectory(examples/echo)
	add_subdirectory(examples/performance-test)
endif()

if(VEIGAR_BUILD_TESTS)
	add_subdirectory(tests)
endif()