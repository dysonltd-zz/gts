# ----------------------------------------------------------------------------
# Top Level CMakeLists.txt
#
# Steps:
# 1. $ mkdir build
# 2. $ cd build
# 3.
# Unix:
#   $ cmake -DOpenCV_ROOT_DIR="<path-to-opencv>" ../
#
# Windows:
#   $ cmake.exe -G"MinGW Makefiles"
#                  -DOpenCV_ROOT_DIR="C:\Software\OpenCV-2.X.0\opencv"
#                  ..
#
# To compile help add -DGTS_HELP=ON
# To compile tests add -DGTS_TESTS=ON
#
# ----------------------------------------------------------------------------

project("GTS")

# Options
option(GTS_HELP "Build Help" ON)
option(GTS_TESTS "Build all GTS tests." OFF)

cmake_minimum_required(VERSION 2.8)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake/")
set(CMAKE_INCLUDE_DIRECTORIES_BEFORE ON)

SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
SET(CMAKE_INSTALL_RPATH "${PROJECT_SOURCE_DIR}/build/app/lib/log4qt")

add_subdirectory("app")

if(GTS_HELP)
    add_subdirectory("help")
endif()
