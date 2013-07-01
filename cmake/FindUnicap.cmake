# Find the libunicap 

include(FindPackageHandleStandardArgs)

find_package(PkgConfig)
pkg_check_modules(PC_UNICAP QUIET libunicap)

set(UNICAP_DEFINITIONS ${PC_LIBUNICAP_CFLAGS_OTHER})

find_path(  UNICAP_INCLUDE_DIRS unicap.h
            HINTS ${PC_LIBUNICAP_INCLUDEDIR} ${PC_LIBUNICAP_INCLUDE_DIRS}
            PATH_SUFFIXES unicap
)

find_library( UNICAP_LIBRARIES NAMES
              unicap
              libunicap
              HINTS ${PC_LIBUNICAP_LIBDIR} ${PC_LIBUNICAP_LIBRARY_DIRS}
)

find_package_handle_standard_args( UNICAP
                                   "Could not find libunicap"
                                   UNICAP_LIBRARIES
                                   UNICAP_INCLUDE_DIRS
)
