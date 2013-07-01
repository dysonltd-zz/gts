# ----------------------------------------------------------------------------
#  
# CMakelists for GTS (Help)
#
# ----------------------------------------------------------------------------

project("gts_help")

cmake_minimum_required(VERSION 2.8)

set(CMAKE_INCLUDE_DIRECTORIES_BEFORE ON)

find_program(QT_QCOLLECTIONGENERATOR_EXECUTABLE qcollectiongenerator REQUIRED)

file(GLOB HELP_SOURCE ./* ./gts_userguide_files/*)

if(QT_QCOLLECTIONGENERATOR_EXECUTABLE)

    add_custom_command(OUTPUT gts_userguide.qch.tmp
                       COMMAND qhelpgenerator gts_userguide.qhp -o gts_userguide.qch.tmp
                       COMMAND ${CMAKE_COMMAND} -E copy gts_userguide.qch.tmp gts_userguide.qch
		               WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/doc
                       DEPENDS ${HELP_SOURCE}
                       COMMENT "copying .qch.temp to qch") # hack to allow for make -j N

    add_custom_target(gts_userguide_qch ALL DEPENDS gts_userguide.qch.tmp)
	
    add_custom_command(OUTPUT gts_userguide.qhc.tmp
                       COMMAND qcollectiongenerator gts_userguide.qhcp -o gts_userguide.qhc.tmp
                       COMMAND ${CMAKE_COMMAND} -E copy gts_userguide.qhc.tmp gts_userguide.qhc
		               WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/doc
                       DEPENDS ${HELP_SOURCE}
                       COMMENT "copying .qhc.temp to qhc")

    add_custom_target(gts_userguide_qhc ALL DEPENDS gts_userguide.qhc.tmp)

    add_custom_target(help ALL DEPENDS gts_userguide_qch gts_userguide_qhc)

endif()

install(FILES ${PROJECT_SOURCE_DIR}/doc/gts_userguide.qch ${PROJECT_SOURCE_DIR}/doc/gts_userguide.qhc DESTINATION ${CMAKE_INSTALL_PREFIX})

# ----------------------------------------------------------------------------
# Warn in the case of in-src build
# ----------------------------------------------------------------------------
if("${CMAKE_CURRENT_SRC_DIR}" STREQUAL "${CMAKE_CURRENT_BINARY_DIR}")
  message(WARNING "The src directory is the same as binary directory. \"make clean\" may damage the src tree")
endif()
