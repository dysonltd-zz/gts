# ----------------------------------------------------------------------------
#
# CMakelists for GTS (App)
#
# ----------------------------------------------------------------------------

project("gts")

cmake_minimum_required(VERSION 2.8)

# Overide with cmake -DCMAKE_BUILD_TYPE=Debug {dir}
if( NOT CMAKE_BUILD_TYPE )
   set( CMAKE_BUILD_TYPE Release )
endif()

# ----------------------------------------------------------------------------
#  Libraries
# ----------------------------------------------------------------------------

set(CMAKE_INCLUDE_DIRECTORIES_BEFORE ON)

find_package(Qt4 COMPONENTS QtCore QtGui QtOpenGL QtXml QtSvg REQUIRED )

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
                               opencv_video REQUIRED)

if(UNIX)
	find_package(Unicap REQUIRED)
endif()

add_subdirectory(lib/log4qt)

# ----------------------------------------------------------------------------
#  Files
# ----------------------------------------------------------------------------

set(GTS_INCLS src/main
              src/gui
              src/hardware
              src/misc
              src/tools
              src/tools/algorithms
              src/tools/algorithms/extrinsic
              src/tools/algorithms/intrinsic
              src/tools/core
              src/tools/core/components
              src/tools/core/mappers
              src/tools/core/tabs
              src/tools/schema
              src/tracking
              src/video
              src/video/input
              src/video/input/fake
              src/video/output
              src/workbench
              lib/log4qt/src)

# add platform specific camera apis
if(MINGW)
    set(GTS_INCLS_PLATFORM
        src/video/input/mingw)
elseif(MSVC)
    set(GTS_INCLS_PLATFORM
        src/video/input/win32)
else(UNIX)
    set(GTS_INCLS_PLATFORM
        src/video/input/unix)
endif()

# find all source files
file(GLOB_RECURSE GTS_SRCS "${PROJECT_SOURCE_DIR}/src/*.cpp")

# remove files
list(REMOVE_ITEM GTS_SRCS

    # platform specific files
    "${PROJECT_SOURCE_DIR}/src/hardware/win32/HardwareAbstraction.cpp"
    "${PROJECT_SOURCE_DIR}/src/hardware/mingw/HardwareAbstraction.cpp"
    "${PROJECT_SOURCE_DIR}/src/hardware/unix/HardwareAbstraction.cpp"

    "${PROJECT_SOURCE_DIR}/src/video/input/win32/DirectShowCameraApi.cpp"
    "${PROJECT_SOURCE_DIR}/src/video/input/mingw/MingwDSCameraApi.cpp"
    "${PROJECT_SOURCE_DIR}/src/video/input/unix/LibUnicapCameraApi.cpp"
    
    "${PROJECT_SOURCE_DIR}/src/misc/WinTime.cpp"
)

# re-add platform specific src files based on archictecture
if(MSVC)
    set(GTS_SRCS_PLATFORM
        src/hardware/win32/HardwareAbstraction.cpp
        src/video/input/win32/DirectShowCameraApi.cpp
        src/misc/WinTime.cpp)
elseif(MINGW)
    set(GTS_SRCS_PLATFORM
        src/hardware/mingw/HardwareAbstraction.cpp
        src/video/input/mingw/MingwDSCameraApi.cpp
	    src/misc/WinTime.cpp)
elseif(UNIX)
    set(GTS_SRCS_PLATFORM
        src/hardware/unix/HardwareAbstraction.cpp
        src/video/input/unix/LibUnicapCameraApi.cpp)
endif()

# find all header files files (pass to MOC even those that don't have QObject for simplicity in cmake)
file(GLOB_RECURSE GTS_HDRS ${PROJECT_SOURCE_DIR}/src/*.h)

file(GLOB_RECURSE GTS_UI_DEFS ${PROJECT_SOURCE_DIR}/src/*.ui)

set(GTS_RESOURCES src/res/resources.qrc)

include( ${QT_USE_FILE} )

include_directories( ${OpenCV2_INCLUDE_DIRS} )

include_directories( ${GTS_INCLS} ${GTS_INCLS_PLATFORM} )

if(UNIX)
	include_directories( ${UNICAP_INCLUDE_DIRS} )
endif()

# Access files produced by uic in our code
include_directories( ${CMAKE_CURRENT_BINARY_DIR} )

# Add useful macros and variables
add_definitions( ${QT_DEFINITIONS} )

QT4_WRAP_UI( GTS_UI_SRCS ${GTS_UI_DEFS} )

QT4_WRAP_CPP( GTS_MOC_SRCS ${GTS_HDRS} OPTIONS "-nn" )

QT4_ADD_RESOURCES(GTS_RESOURCES_RCC ${GTS_RESOURCES})

# ----------------------------------------------------------------------------
#  Compilation
# ----------------------------------------------------------------------------

if(MSVC)
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /Zi")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_CRT_SECURE_NO_WARNINGS")

    set_source_files_properties(${GTS_SRCS} PROPERTIES COMPILE_FLAGS "/Zc:wchar_t-")
    set_source_files_properties(${GTS_SRCS_PLATFORM} PROPERTIES COMPILE_FLAGS "/Zc:wchar_t-")

    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:windows /ENTRY:mainCRTStartup")
endif()

# GNUCXX for MinGW or UNIX
if(CMAKE_COMPILER_IS_GNUCXX)
    message(STATUS "GCC detected - Setting compiler flags")
    if(MINGW)
        set(CMAKE_CXX_FLAGS "-ggdb -Wextra -Wall -Wno-ignored-qualifiers -pedantic -std=gnu++0x")
	else()
        set(CMAKE_CXX_FLAGS "-ggdb -Wextra -Wall -Wno-ignored-qualifiers -pedantic -std=c++0x")
	endif()
endif()

# ----------------------------------------------------------------------------
#  Build & install layouts
# ----------------------------------------------------------------------------

# Save executable in the same place
set(EXECUTABLE_OUTPUT_PATH "${CMAKE_BINARY_DIR}/bin" CACHE PATH "Output directory for applications" )

if(MSVC)
    add_executable( gts WIN32 ${GTS_RESOURCES_RCC} ${GTS_UI_SRCS} ${GTS_MOC_SRCS} ${GTS_SRCS} ${GTS_SRCS_PLATFORM} )
else()
    add_executable( gts ${GTS_RESOURCES_RCC} ${GTS_UI_SRCS} ${GTS_MOC_SRCS} ${GTS_SRCS} ${GTS_SRCS_PLATFORM} )
endif()

if(MINGW)
    target_link_libraries( gts ${OpenCV2_LIBRARIES} ${QT_LIBRARIES} Log4Qt )
elseif(MSVC)
    target_link_libraries( gts ${OpenCV2_LIBRARIES} ${QT_LIBRARIES} Log4Qt strmiids.lib )
elseif(UNIX)
    target_link_libraries( gts ${OpenCV2_LIBRARIES} ${QT_LIBRARIES} ${UNICAP_LIBRARIES} Log4Qt rt )
endif()

# ----------------------------------------------------------------------------
#  Testing
# ----------------------------------------------------------------------------

if(GTS_TESTS)

    set(GTS_TESTED_UI_SRCS ${GTS_UI_SRCS})
    set(GTS_TESTED_MOC_SRCS ${GTS_MOC_SRCS})
    set(GTS_TESTED_SRCS ${GTS_SRCS})
    set(GTS_TESTED_SRCS_PLATFORM ${GTS_SRCS_PLATFORM})

    list(REMOVE_ITEM GTS_TESTED_SRCS
        "${PROJECT_SOURCE_DIR}/src/main/main.cpp")

    # Compile the GTest library
    add_subdirectory(lib/gtest-1.6.0)

    message(STATUS ${gtest_SOURCE_DIR})
    message(STATUS ${gtest_BINARY_DIR})

    # Include the Google Test library
    include_directories(${gtest_SOURCE_DIR}/include)

    # find all test files by recursively searching in test dir
    file(GLOB_RECURSE GTS_TEST_SOURCES ${PROJECT_SOURCE_DIR}/test/*.cpp)
    file(GLOB_RECURSE GTS_TEST_HEADERS ${PROJECT_SOURCE_DIR}/test/*.h )

    QT4_WRAP_CPP( GTS_TEST_MOC_SRCS ${GTS_TEST_HEADERS} OPTIONS "-nn" )

    if(MINGW)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DGTEST_HAS_PTHREAD=0")
	endif()

    add_executable( gts_test ${GTS_TEST_SOURCES} ${GTS_TEST_MOC_SRCS} ${GTS_TESTED_UI_SRCS} ${GTS_TESTED_MOC_SRCS} ${GTS_TESTED_SRCS} ${GTS_TESTED_SRCS_PLATFORM} )

    if(MINGW)
        target_link_libraries( gts_test ${OpenCV2_LIBRARIES} ${QT_LIBRARIES} Log4Qt gtest )
    elseif(MSVC)
        link_directories( ${gtest_BINARY_DIR} )
        target_link_libraries( gts_test ${OpenCV2_LIBRARIES} ${QT_LIBRARIES} Log4Qt gtest strmiids.lib )
    elseif(UNIX)
        target_link_libraries( gts_test ${OpenCV2_LIBRARIES} ${QT_LIBRARIES} ${UNICAP_LIBRARIES} Log4Qt gtest rt )
    endif()
endif()


INSTALL(TARGETS gts RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX})
# ----------------------------------------------------------------------------
# Warn in the case of in-src build
# ----------------------------------------------------------------------------
if("${CMAKE_CURRENT_SRC_DIR}" STREQUAL "${CMAKE_CURRENT_BINARY_DIR}")
  message(WARNING "The src directory is the same as binary directory. \"make clean\" may damage the src tree")
endif()
