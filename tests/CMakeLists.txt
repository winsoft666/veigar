#################################################################################
#    Veigar: Cross platform RPC library using shared memory.
#    ---------------------------------------------------------------------------
#    Copyright (C) 2022~2023 winsoft666 <winsoft666@outlook.com>.
#
#    This program is free software: you can redistribute it and/or modify
#    it under the terms of the GNU General Public License as published by
#    the Free Software Foundation, either version 3 of the License, or
#    (at your option) any later version.
#
#    This program is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU General Public License for more details.
#
#    You should have received a copy of the GNU General Public License
#    along with this program.  If not, see <http://www.gnu.org/licenses/>.
#################################################################################

set (CMAKE_CXX_STANDARD 14)

set(EXE_NAME test)

file(GLOB TEST_SOURCE_FILES ./*.cpp ./*.h)
file(GLOB VEIGAR_SOURCE_FILES ../src/*.h ../src/*.cpp)

source_group("test" FILES ${TEST_SOURCE_FILES})

include_directories(../include)

if (MSVC AND VEIGAR_USE_STATIC_CRT)
    set(CompilerFlags
        CMAKE_CXX_FLAGS
        CMAKE_CXX_FLAGS_DEBUG
        CMAKE_CXX_FLAGS_RELEASE
        CMAKE_C_FLAGS
        CMAKE_C_FLAGS_DEBUG
        CMAKE_C_FLAGS_RELEASE
        )
    foreach(CompilerFlag ${CompilerFlags})
        string(REPLACE "/MD" "/MT" ${CompilerFlag} "${${CompilerFlag}}")
    endforeach()
endif()

add_executable(
	${EXE_NAME}
	${TEST_SOURCE_FILES}
    ${VEIGAR_SOURCE_FILES})

add_dependencies(${EXE_NAME} ${PROJECT_NAME})
target_link_libraries(${EXE_NAME} ${PROJECT_NAME})

# Win32 Console
if (WIN32 OR _WIN32)
	set_target_properties(${EXE_NAME} PROPERTIES LINK_FLAGS "/SUBSYSTEM:CONSOLE")
	set_target_properties(${EXE_NAME} PROPERTIES COMPILE_DEFINITIONS "_CONSOLE")
else()
   target_link_libraries(${EXE_NAME} pthread)
endif()
