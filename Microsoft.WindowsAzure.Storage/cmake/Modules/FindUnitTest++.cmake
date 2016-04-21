# find UnitTest++
#
# exports:
#
#   UnitTest++_FOUND
#   UnitTest++_INCLUDE_DIRS
#   UnitTest++_LIBRARIES
#

include(FindPkgConfig)
include(FindPackageHandleStandardArgs)

# Use pkg-config to get hints about paths
pkg_check_modules(UnitTest++_PKGCONF QUIET unittest++)

# Include dir
find_path(UnitTest++_INCLUDE_DIR
  NAMES UnitTest++.h
  PATHS
  	${UnitTest++_PKGCONF_INCLUDE_DIRS}
  	${CMAKE_SOURCE_DIR}/tests/UnitTest++/src
	/usr/local/include
  PATH_SUFFIXES
	unittest++
	UnitTest++
)

# Library
find_library(UnitTest++_LIBRARY
  NAMES unittest++ UnitTest++
  PATHS 
  	${UnitTest++_PKGCONF_LIBRARY_DIRS}
  	${CMAKE_SOURCE_DIR}/tests/UnitTest++
	/usr/local/lib
)

set(UnitTest++_PROCESS_LIBS UnitTest++_LIBRARY)
set(UnitTest++_PROCESS_INCLUDES UnitTest++_INCLUDE_DIR)
libfind_process(UnitTest++)
