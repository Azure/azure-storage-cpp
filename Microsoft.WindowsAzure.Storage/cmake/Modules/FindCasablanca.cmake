# FindCasablanca package
#
# Tries to find the Casablanca (C++ REST SDK) library
#

find_package(PkgConfig)

include(LibFindMacros)

# Include dir
find_path(CASABLANCA_INCLUDE_DIR
  NAMES cpprest/http_client.h
  PATHS 
    ${CASABLANCA_PKGCONF_INCLUDE_DIRS}
    ${CASABLANCA_DIR}
    $ENV{CASABLANCA_DIR}
    /usr/local/include
    /usr/include
  PATH_SUFFIXES Release/include include
)

# Library
find_library(CASABLANCA_LIBRARY
  NAMES libcpprest.so
  PATHS 
    ${CASABLANCA_PKGCONF_LIBRARY_DIRS}
    ${CASABLANCA_DIR}
    ${CASABLANCA_DIR}/lib
    $ENV{CASABLANCA_DIR}/lib
    $ENV{CASABLANCA_DIR}
    /usr/local/lib
    /usr/lib
  PATH_SUFFIXES Release/build.release/Binaries/ build.release/Binaries/
)

set(CASABLANCA_PROCESS_LIBS CASABLANCA_LIBRARY)
set(CASABLANCA_PROCESS_INCLUDES CASABLANCA_INCLUDE_DIR)
libfind_process(CASABLANCA)

