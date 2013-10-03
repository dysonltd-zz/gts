# ----------------------------------------------------------------------------
#  CMake file for Log4Qt
#
#    From the off-tree build directory, invoke:
#      $ cmake.exe -G"MinGW Makefiles"
#                  ..
#
# ----------------------------------------------------------------------------

project("Log4Qt")

cmake_minimum_required(VERSION 2.8)

find_package(Qt4 COMPONENTS QtCore REQUIRED)

include(${QT_USE_FILE})

#ADD_DEFINITIONS(-DCUTELOGGER_LIBRARY)

include_directories(BEFORE include)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/src/")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/src/log4qt/")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/src/log4qt/helpers/")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/src/log4qt/spi/")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/src/log4qt/varia/")

set(SOURCES src/log4qt/appenderskeleton.cpp
            src/log4qt/basicconfigurator.cpp
            src/log4qt/consoleappender.cpp
            src/log4qt/dailyrollingfileappender.cpp
            src/log4qt/fileappender.cpp
            src/log4qt/hierarchy.cpp
            src/log4qt/layout.cpp
            src/log4qt/level.cpp
            src/log4qt/log4qt.cpp
            src/log4qt/logger.cpp
            src/log4qt/loggerrepository.cpp
            src/log4qt/loggingevent.cpp
            src/log4qt/logmanager.cpp
            src/log4qt/mdc.cpp
            src/log4qt/ndc.cpp
            src/log4qt/patternlayout.cpp
            src/log4qt/propertyconfigurator.cpp
            src/log4qt/rollingfileappender.cpp
            src/log4qt/simplelayout.cpp
            src/log4qt/ttcclayout.cpp
            src/log4qt/writerappender.cpp

            src/log4qt/helpers/classlogger.cpp
            src/log4qt/helpers/configuratorhelper.cpp
            src/log4qt/helpers/datetime.cpp
            src/log4qt/helpers/factory.cpp
            src/log4qt/helpers/initialisationhelper.cpp
            src/log4qt/helpers/logerror.cpp
            src/log4qt/helpers/logobject.cpp
            src/log4qt/helpers/logobjectptr.cpp
            src/log4qt/helpers/optionconverter.cpp
            src/log4qt/helpers/patternformatter.cpp
            src/log4qt/helpers/properties.cpp

            src/log4qt/spi/filter.cpp

            src/log4qt/varia/debugappender.cpp
            src/log4qt/varia/denyallfilter.cpp
            src/log4qt/varia/levelmatchfilter.cpp
            src/log4qt/varia/levelrangefilter.cpp
            src/log4qt/varia/listappender.cpp
            src/log4qt/varia/nullappender.cpp
            src/log4qt/varia/stringmatchfilter.cpp)

set(INCLUDES src/log4qt/appender.h
             src/log4qt/appenderskeleton.h
             src/log4qt/basicconfigurator.h
             src/log4qt/consoleappender.h
             src/log4qt/dailyrollingfileappender.h
             src/log4qt/fileappender.h
             src/log4qt/hierarchy.h
             src/log4qt/layout.h
             src/log4qt/level.h
             src/log4qt/log4qt.h
             src/log4qt/logger.h
             src/log4qt/loggerrepository.h
             src/log4qt/loggingevent.h
             src/log4qt/logmanager.h
             src/log4qt/mdc.h
             src/log4qt/ndc.h
             src/log4qt/patternlayout.h
             src/log4qt/propertyconfigurator.h
             src/log4qt/rollingfileappender.h
             src/log4qt/simplelayout.h
             src/log4qt/ttcclayout.h
             src/log4qt/writerappender.h

             src/log4qt/helpers/classlogger.h
             src/log4qt/helpers/configuratorhelper.h
             src/log4qt/helpers/datetime.h
             src/log4qt/helpers/factory.h
             src/log4qt/helpers/initialisationhelper.h
             src/log4qt/helpers/logerror.h
             src/log4qt/helpers/logobject.h
             src/log4qt/helpers/logobjectptr.h
             src/log4qt/helpers/optionconverter.h
             src/log4qt/helpers/patternformatter.h
             src/log4qt/helpers/properties.h

             src/log4qt/spi/filter.h

             src/log4qt/varia/debugappender.h
             src/log4qt/varia/denyallfilter.h
             src/log4qt/varia/levelmatchfilter.h
             src/log4qt/varia/levelrangefilter.h
             src/log4qt/varia/listappender.h
             src/log4qt/varia/nullappender.h
             src/log4qt/varia/stringmatchfilter.h)

set(MOCHEADERS src/log4qt/appender.h
               src/log4qt/appenderskeleton.h
               src/log4qt/consoleappender.h
               src/log4qt/dailyrollingfileappender.h
               src/log4qt/fileappender.h
               src/log4qt/layout.h
               src/log4qt/logger.h
               src/log4qt/patternlayout.h
               src/log4qt/rollingfileappender.h
               src/log4qt/simplelayout.h
               src/log4qt/ttcclayout.h
               src/log4qt/writerappender.h

               src/log4qt/helpers/configuratorhelper.h
               src/log4qt/helpers/logobject.h

               src/log4qt/spi/filter.h

               src/log4qt/varia/debugappender.h
               src/log4qt/varia/denyallfilter.h
               src/log4qt/varia/levelmatchfilter.h
               src/log4qt/varia/levelrangefilter.h
               src/log4qt/varia/listappender.h
               src/log4qt/varia/nullappender.h
               src/log4qt/varia/stringmatchfilter.h)

QT4_WRAP_CPP(MOCS ${MOCHEADERS})

# OutputDebugAppender is only for Windows systems
#IF(WIN32)
#  SET(SOURCES ${SOURCES} src/OutputDebugAppender.cpp)
#  SET(INCLUDES ${INCLUDES} include/OutputDebugAppender.h)
#ENDIF(WIN32)

set(LIBRARY_TARGET Log4Qt)

add_library(${LIBRARY_TARGET} STATIC ${SOURCES} ${MOCS} ${INCLUDES})

target_link_libraries(${LIBRARY_TARGET} ${QT_LIBRARIES})
