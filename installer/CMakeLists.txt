# ----------------------------------------------------------------------------
# 
# CMake file for GTS (Win_Installer)
#
# ----------------------------------------------------------------------------

project("gts_win_installer")

cmake_minimum_required(VERSION 2.8)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/../cmake/")

set( CMAKE_INCLUDE_DIRECTORIES_BEFORE ON )

find_package(Qt4 COMPONENTS QtCore QtGui QtOpenGL QtXml REQUIRED )

find_package(OpenCV COMPONENTS opencv_calib3d
                               opencv_contrib
                               opencv_core
							   opencv_features2d
							   opencv_ffmpeg
							   opencv_flann
							   opencv_gpu
							   opencv_highgui
							   opencv_imgproc
							   opencv_legacy
							   opencv_ml
							   opencv_objdetect
							   opencv_ts
							   opencv_video REQUIRED )


if(NOT QT_ASSISTANT_EXECUTABLE)
    if(MSVC)
        find_program(QT_ASSISTANT_EXECUTABLE assistant)
    else()
        find_program(QT_ASSISTANT_EXECUTABLE assistant.exe)
    endif()
endif()

if(MINGW)
    get_filename_component( MINGW_PATH ${CMAKE_CXX_COMPILER} PATH )

    set( CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS ${MINGW_PATH}/mingwm10.dll
	                                       ${MINGW_PATH}/libgcc_s_dw2-1.dll
										   ${MINGW_PATH}/libstdc++-6.dll )
endif()

include( ${QT_USE_FILE} )

# Add useful macros and variables
add_definitions( ${QT_DEFINITIONS} )

#include( InstallRequiredSystemLibraries )

# ----------------------------------------------------------------------------
#  Build & install layouts
# ----------------------------------------------------------------------------

if(MSVC)
    install( PROGRAMS "${CMAKE_CURRENT_SOURCE_DIR}/../build/bin/gts.exe"
             DESTINATION bin
		     COMPONENT Application )
elseif(MINGW)
    install( PROGRAMS "${CMAKE_CURRENT_SOURCE_DIR}/../build/bin/gts"
             DESTINATION bin
		     COMPONENT Application )
endif()

install( PROGRAMS "${CMAKE_CURRENT_SOURCE_DIR}/../LICENSE"
				  "${CMAKE_CURRENT_SOURCE_DIR}/../OPENSOURCE_LICENSES"
                  "${CMAKE_CURRENT_SOURCE_DIR}/../help/doc/gts_userguide.qch"
                  "${CMAKE_CURRENT_SOURCE_DIR}/../help/doc/gts_userguide.qhc"
         DESTINATION doc
		 COMPONENT Application )

install( FILES "${QT_LIBRARY_DIR}/../bin/QtCore4.dll"
               "${QT_LIBRARY_DIR}/../bin/QtGui4.dll"
               "${QT_LIBRARY_DIR}/../bin/QtXml4.dll"
               "${QT_LIBRARY_DIR}/../bin/QtHelp4.dll"
               "${QT_LIBRARY_DIR}/../bin/QtSql4.dll"
               "${QT_LIBRARY_DIR}/../bin/QtSvg4.dll"
               "${QT_LIBRARY_DIR}/../bin/QtNetwork4.dll"
               "${QT_LIBRARY_DIR}/../bin/QtCLucene4.dll"
               "${QT_LIBRARY_DIR}/../bin/QtWebkit4.dll"
         DESTINATION bin
         COMPONENT Qt )

install( FILES "${QT_LIBRARY_DIR}/../plugins/sqldrivers/qsqlite4.dll"
               "${QT_LIBRARY_DIR}/../plugins/sqldrivers/qsqlodbc4.dll"
               "${QT_LIBRARY_DIR}/../plugins/sqldrivers/qsqlpsql4.dll"
         DESTINATION bin/sqldrivers
         COMPONENT Qt )

install( FILES "${QT_LIBRARY_DIR}/../plugins/imageformats/qjpeg4.dll"
         DESTINATION bin/imageformats
         COMPONENT Qt )

install( PROGRAMS ${QT_ASSISTANT_EXECUTABLE}
         DESTINATION bin
		 COMPONENT Qt )

if(MSVC)
    install( FILES "${OpenCV_ROOT_DIR}/build/x86/vc10/bin/opencv_calib3d246.dll"
                   "${OpenCV_ROOT_DIR}/build/x86/vc10/bin/opencv_contrib246.dll"
                   "${OpenCV_ROOT_DIR}/build/x86/vc10/bin/opencv_core246.dll"
                   "${OpenCV_ROOT_DIR}/build/x86/vc10/bin/opencv_features2d246.dll"
                   "${OpenCV_ROOT_DIR}/build/x86/vc10/bin/opencv_flann246.dll"
                   "${OpenCV_ROOT_DIR}/build/x86/vc10/bin/opencv_gpu246.dll"
                   "${OpenCV_ROOT_DIR}/build/x86/vc10/bin/opencv_highgui246.dll"
                   "${OpenCV_ROOT_DIR}/build/x86/vc10/bin/opencv_imgproc246.dll"
                   "${OpenCV_ROOT_DIR}/build/x86/vc10/bin/opencv_legacy246.dll"
                   "${OpenCV_ROOT_DIR}/build/x86/vc10/bin/opencv_ml246.dll"
                   "${OpenCV_ROOT_DIR}/build/x86/vc10/bin/opencv_objdetect246.dll"
                   "${OpenCV_ROOT_DIR}/build/x86/vc10/bin/opencv_ts246.dll"
                   "${OpenCV_ROOT_DIR}/build/x86/vc10/bin/opencv_video246.dll"
             DESTINATION bin
             COMPONENT OpenCV )
elseif(MINGW)
    install( FILES "${OpenCV_ROOT_DIR}/build/x86/vc10/bin/libopencv_calib3d246.dll"
                   "${OpenCV_ROOT_DIR}/build/x86/vc10/bin/libopencv_contrib246.dll"
                   "${OpenCV_ROOT_DIR}/build/x86/vc10/bin/libopencv_core246.dll"
                   "${OpenCV_ROOT_DIR}/build/x86/vc10/bin/libopencv_features2d246.dll"
                   "${OpenCV_ROOT_DIR}/build/x86/vc10/bin/libopencv_flann246.dll"
                   "${OpenCV_ROOT_DIR}/build/x86/vc10/bin/libopencv_gpu246.dll"
                   "${OpenCV_ROOT_DIR}/build/x86/vc10/bin/libopencv_highgui246.dll"
                   "${OpenCV_ROOT_DIR}/build/x86/vc10/bin/libopencv_imgproc246.dll"
                   "${OpenCV_ROOT_DIR}/build/x86/vc10/bin/libopencv_legacy246.dll"
                   "${OpenCV_ROOT_DIR}/build/x86/vc10/bin/libopencv_ml246.dll"
                   "${OpenCV_ROOT_DIR}/build/x86/vc10/bin/libopencv_objdetect246.dll"
                   "${OpenCV_ROOT_DIR}/build/x86/vc10/bin/libopencv_ts246.dll"
                   "${OpenCV_ROOT_DIR}/build/x86/vc10/bin/libopencv_video246.dll"
             DESTINATION bin
             COMPONENT OpenCV )
endif()

if(MSVC)
    install( PROGRAMS "${CMAKE_CURRENT_SOURCE_DIR}/files/DOWNLOADS"
             DESTINATION ext
		     COMPONENT Other )
endif()

set( CPACK_GENERATOR "NSIS" )

set( CPACK_PACKAGE_NAME "GTS" )
set( CPACK_PACKAGE_VENDOR "Ground Truth System" )
set( CPACK_PACKAGE_DESCRIPTION_SUMMARY "GTS - Ground Truth System" )
set( CPACK_PACKAGE_VERSION_MAJOR "0" )
set( CPACK_PACKAGE_VERSION_MINOR "3" )
set( CPACK_PACKAGE_VERSION_PATCH "6" )
set( CPACK_PACKAGE_VERSION "${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}" )
set( CPACK_PACKAGE_INSTALL_DIRECTORY "GTS" )

set( CPACK_NSIS_MUI_ICON "${CMAKE_CURRENT_SOURCE_DIR}/icon/gts.ico" )
set( CPACK_NSIS_MUI_UNIICON "${CMAKE_CURRENT_SOURCE_DIR}/icon/gts.ico" )

set( CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/../LICENSE" )

include( CPack )

# ----------------------------------------------------------------------------
# Warn in the case of in-src build
# ----------------------------------------------------------------------------
if("${CMAKE_CURRENT_SRC_DIR}" STREQUAL "${CMAKE_CURRENT_BINARY_DIR}")
  message(WARNING "The src directory is the same as binary directory. \"make clean\" may damage the src tree")
endif()
