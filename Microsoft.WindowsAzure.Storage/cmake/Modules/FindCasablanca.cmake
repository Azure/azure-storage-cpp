# FindCasablanca package
#
# Tries to find the Casablanca (C++ REST SDK) library
#

find_package(PkgConfig)

include(LibFindMacros)

if(WIN32)
  find_package(cpprestsdk)
  
  if(cpprestsdk_FOUND)
    set(CASABLANCA_LIBRARY cpprestsdk::cpprest)
    set(CASABLANCA_PROCESS_LIBS CASABLANCA_LIBRARY)
    set(CASABLANCA_PROCESS_INCLUDES CASABLANCA_INCLUDE_DIR)
    libfind_process(CASABLANCA)
    return()
  endif()
else()
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
endif()

# Library
find_library(CASABLANCA_LIBRARY
  NAMES 
    cpprest
    cpprest_2_9.lib
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

