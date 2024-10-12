############################################################################
# Copyright (c) winsoft666.
# All rights reserved.
#
# This source code is licensed under the license found in the
# LICENSE file in the root directory of this source tree.
############################################################################

add_executable(echo main.cpp)

set_target_properties(echo PROPERTIES 
  OUTPUT_NAME echo
  DEBUG_OUTPUT_NAME echo-d)

# Win32 Console
if (WIN32 OR _WIN32)
	set_target_properties(${EXE_NAME} PROPERTIES LINK_FLAGS "/SUBSYSTEM:CONSOLE")
	set_target_properties(${EXE_NAME} PROPERTIES COMPILE_DEFINITIONS "_CONSOLE")
endif()

add_dependencies(echo veigar)

target_include_directories(echo 
    PRIVATE "${CMAKE_SOURCE_DIR}/include"
)

target_link_libraries(echo 
    PRIVATE veigar
)

install(TARGETS echo
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
	LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
	ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
	OBJECTS DESTINATION ${CMAKE_INSTALL_LIBDIR}
)