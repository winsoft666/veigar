############################################################################
# Copyright (c) winsoft666.
# All rights reserved.
#
# This source code is licensed under the license found in the
# LICENSE file in the root directory of this source tree.
############################################################################

set (CMAKE_CXX_STANDARD 11)
set (PACKAGE_VERSION 1.2)

if (MSVC)
	add_definitions(
		-D_WIN32
	)
endif()

file(GLOB HEADER_FILES                 ../include/veigar/*.h ../include/veigar/*.hpp ../include/veigar/*.inl)
file(GLOB DETAIL_HEADER_FILES         ../include/veigar/detail/*.h ../include/veigar/detail/*.hpp ../include/veigar/detail/*.inl)

file(GLOB SOURCE_FILES                 ./*.cpp ./*.h ./*.hpp)
file(GLOB_RECURSE MSGPACK_FILES      ../include/veigar/msgpack/*.hpp ../include/veigar/msgpack/*.h)

source_group("Header Files" FILES ${HEADER_FILES})
source_group("Header Files/detail" FILES ${DETAIL_HEADER_FILES})

source_group("Source Files" FILES ${SOURCE_FILES})
source_group("MsgPack" FILES ${MSGPACK_FILES})

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

add_library(
	${PROJECT_NAME}
	STATIC
	${HEADER_FILES}
	${SOURCE_FILES}
	${DETAIL_HEADER_FILES}
	${MSGPACK_FILES}
)

target_include_directories(${PROJECT_NAME} PUBLIC
	"$<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>"
    "$<INSTALL_INTERFACE:include>"
)

# set output name prefix/suffix
set_target_properties(${PROJECT_NAME} PROPERTIES 
  OUTPUT_NAME ${PROJECT_NAME}
  DEBUG_OUTPUT_NAME ${PROJECT_NAME}${DEBUG_LIBNAME_SUFFIX})

install(TARGETS ${PROJECT_NAME}
	EXPORT veigar-targets
	RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
	LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
	ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
	OBJECTS DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

install(EXPORT veigar-targets
	FILE veigar-targets.cmake
	NAMESPACE veigar::
	DESTINATION share/veigar
)

file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/veigar-config.cmake.in"
[[include(CMakeFindDependencyMacro)
include("${CMAKE_CURRENT_LIST_DIR}/veigar-targets.cmake")
]])
configure_file("${CMAKE_CURRENT_BINARY_DIR}/veigar-config.cmake.in" "${CMAKE_CURRENT_BINARY_DIR}/veigar-config.cmake" @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/veigar-config.cmake DESTINATION share/veigar)

include(CMakePackageConfigHelpers)
write_basic_package_version_file(
	veigarConfigVersion.cmake
	VERSION ${PACKAGE_VERSION}
	COMPATIBILITY AnyNewerVersion
)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/veigarConfigVersion.cmake DESTINATION share/veigar)

install(DIRECTORY ../include/veigar 	DESTINATION include)
