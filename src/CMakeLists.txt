############################################################################
# Copyright (c) winsoft666.
# All rights reserved.
#
# This source code is licensed under the license found in the
# LICENSE file in the root directory of this source tree.
############################################################################

file(GLOB HEADER_FILES                 ../include/veigar/*.h ../include/veigar/*.hpp ../include/veigar/*.inl)
file(GLOB DETAIL_HEADER_FILES         ../include/veigar/detail/*.h ../include/veigar/detail/*.hpp ../include/veigar/detail/*.inl)

file(GLOB SOURCE_FILES                 ./*.cpp ./*.h ./*.hpp)
file(GLOB_RECURSE MSGPACK_FILES      ../include/veigar/msgpack/*.hpp ../include/veigar/msgpack/*.h)

source_group("Header Files" FILES ${HEADER_FILES})
source_group("Header Files/detail" FILES ${DETAIL_HEADER_FILES})

source_group("Source Files" FILES ${SOURCE_FILES})
source_group("MsgPack" FILES ${MSGPACK_FILES})

add_library(
	veigar
	STATIC
	${HEADER_FILES}
	${SOURCE_FILES}
	${DETAIL_HEADER_FILES}
	${MSGPACK_FILES}
)

set_target_properties(veigar PROPERTIES 
  OUTPUT_NAME veigar
  DEBUG_OUTPUT_NAME veigar-d)

target_include_directories(veigar
	PUBLIC $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
	PUBLIC $<INSTALL_INTERFACE:../include>
)

target_compile_definitions(veigar PRIVATE UNICODE _UNICODE NOMINMAX)
if(WIN32)
	target_compile_definitions(veigar PRIVATE _WIN32)
endif()

if(VEIGAR_ENABLE_RUN_TIME_RECORD)
	target_compile_definitions(veigar PRIVATE VEIGAR_ENABLE_RUN_TIME_RECORD)
endif()

target_include_directories(veigar PUBLIC
	"$<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>"
    "$<INSTALL_INTERFACE:include>"
)

install(TARGETS veigar
	EXPORT veigarConfig
	RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
	LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
	ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
	OBJECTS DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

install(DIRECTORY ../include/veigar DESTINATION include)

install(EXPORT veigarConfig
	FILE veigarConfig.cmake
	NAMESPACE veigar::
	DESTINATION share/veigar
)

include(CMakePackageConfigHelpers)
write_basic_package_version_file(
	veigarConfigVersion.cmake
	COMPATIBILITY AnyNewerVersion
)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/veigarConfigVersion.cmake DESTINATION share/veigar)
