# find libxml++
#
# exports:
#
#   LibXML++_FOUND
#   LibXML++_INCLUDE_DIRS
#   LibXML++_LIBRARIES
#

include(LibFindMacros)

# Use pkg-config to get hints about paths
libfind_pkg_check_modules(LibXML++_PKGCONF QUIET libxml++-2.6)

# Include dir
find_path(LibXML++_INCLUDE_DIR
  NAMES libxml++/libxml++.h
  PATHS
    ${LibXML++_PKGCONF_INCLUDE_DIRS}
    ${LibXML++_ROOT_DIR}
    /usr
  PATH_SUFFIXES
    include/libxml++-2.6
)

# Config Include dir
find_path(LibXML++Config_INCLUDE_DIR
  NAMES libxml++config.h
  PATHS
    ${LibXML++_PKGCONF_INCLUDE_DIRS}
    ${LibXML++_ROOT_DIR}
    /usr
  PATH_SUFFIXES
    lib/libxml++-2.6/include
)

# Finally the library itself
find_library(LibXML++_LIBRARY
  NAMES xml++ xml++-2.6
  PATHS
    ${LibXML++_PKGCONF_LIBRARY_DIRS}
    ${LibXML++_ROOT_DIR}
    /usr
  PATH_SUFFIXES
    lib
)

FIND_PACKAGE_HANDLE_STANDARD_ARGS(LibXML++ DEFAULT_MSG LibXML++_LIBRARY LibXML++_INCLUDE_DIR)

if(LibXML++_INCLUDE_DIR AND LibXML++_LIBRARY)
  set(LibXML++_LIBRARIES ${LibXML++_LIBRARY} ${LibXML++_PKGCONF_LIBRARIES})
  set(LibXML++_INCLUDE_DIRS ${LibXML++_INCLUDE_DIR} ${LibXML++Config_INCLUDE_DIR} ${LibXML++_PKGCONF_INCLUDE_DIRS})
  set(LibXML++_FOUND yes)
else()
  set(LibXML++_LIBRARIES)
  set(LibXML++_INCLUDE_DIRS)
  set(LibXML++_FOUND no)
endif()
