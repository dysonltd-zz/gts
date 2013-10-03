# ----------------------------------------------------------------------------
# Top Level CMakeLists.txt
#
# Steps:
# 1. $ mkdir build
# 2. $ cd build
# 3.
#
# Unix:
#   $ cmake -DOpenCV_ROOT_DIR="<path-to-opencv>" ../
#
# Windows:
#   $ cmake.exe -G"MinGW Makefiles" -DOpenCV_ROOT_DIR="<path-to-opencv>" ..
#   or
#   $ cmake.exe -G"NMake Makefiles" -DOpenCV_ROOT_DIR="<path-to-opencv>" ..
#
# Options:
# > Compile help -DGTS_HELP=ON (ON by default)
# > Compile tests -DGTS_TESTS=ON (OFF by default)
#
# ----------------------------------------------------------------------------

project("GTS")

# Options
option(GTS_HELP "Build Help" ON)
option(GTS_TESTS "Build all GTS tests." OFF)

cmake_minimum_required(VERSION 2.8)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake/")
set(CMAKE_INCLUDE_DIRECTORIES_BEFORE ON)

set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
set(CMAKE_INSTALL_RPATH "${PROJECT_SOURCE_DIR}/build/app/lib/log4qt")

add_subdirectory("app")

if(GTS_HELP)
    add_subdirectory("help")
endif()
