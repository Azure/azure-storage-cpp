# FindCasablanca package
#
# Tries to find the Casablanca (C++ REST SDK) library
#

find_package(PkgConfig)

include(LibFindMacros)

# Include dir
find_path(CASABLANCA_INCLUDE_DIR
  NAMES
    cpprest/http_client.h
  PATHS 
    ${CASABLANCA_PKGCONF_INCLUDE_DIRS}
    ${CASABLANCA_DIR}
    $ENV{CASABLANCA_DIR}
    /usr/local/include
    /usr/include
    ../../casablanca
  PATH_SUFFIXES 
    Release/include
    include
)

# Library
find_library(CASABLANCA_LIBRARY
  NAMES 
    cpprest
  PATHS 
    ${CASABLANCA_PKGCONF_LIBRARY_DIRS}
    ${CASABLANCA_DIR}
    ${CASABLANCA_DIR}
    $ENV{CASABLANCA_DIR}
    /usr/local
    /usr
    ../../casablanca
  PATH_SUFFIXES
    lib
    Release/build.release/Binaries/
    build.release/Binaries/
)

set(CASABLANCA_PROCESS_LIBS CASABLANCA_LIBRARY)
set(CASABLANCA_PROCESS_INCLUDES CASABLANCA_INCLUDE_DIR)
libfind_process(CASABLANCA)

