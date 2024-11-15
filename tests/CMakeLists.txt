############################################################################
# Copyright (c) winsoft666.
# All rights reserved.
#
# This source code is licensed under the license found in the
# LICENSE file in the root directory of this source tree.
############################################################################

add_subdirectory(other)

file(GLOB TEST_SOURCE_FILES ./*.cpp ./*.h)
file(GLOB VEIGAR_SOURCE_FILES ../src/*.h ../src/*.cpp)

source_group("test" FILES ${TEST_SOURCE_FILES})

add_executable(
	test
	${TEST_SOURCE_FILES}
    ${VEIGAR_SOURCE_FILES}
)

set_target_properties(test PROPERTIES 
  OUTPUT_NAME test
  DEBUG_OUTPUT_NAME test-d)

add_dependencies(test veigar)

target_include_directories(test 
    PRIVATE "${CMAKE_SOURCE_DIR}/include"
)

target_link_libraries(test 
    PRIVATE veigar
)

# Win32 Console
if (WIN32 OR _WIN32)
	set_target_properties(test PROPERTIES LINK_FLAGS "/SUBSYSTEM:CONSOLE")
	set_target_properties(test PROPERTIES COMPILE_DEFINITIONS "_CONSOLE")
endif()

install(TARGETS test
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
	LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
	ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
	OBJECTS DESTINATION ${CMAKE_INSTALL_LIBDIR}
)