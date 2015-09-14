set(CMAKE_LEGACY_CYGWIN_WIN32 0)
cmake_minimum_required(VERSION 2.6)
project(azurestorage)

enable_testing()

set(WARNINGS)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")

option(BUILD_TESTS "Build test codes" OFF)
option(BUILD_SAMPLES "Build sample codes" OFF)

# Platform (not compiler) specific settings
if(UNIX)
  find_package(Boost REQUIRED COMPONENTS log log_setup random system thread locale regex filesystem chrono date_time)
  find_package(Threads REQUIRED)
  find_package(OpenSSL REQUIRED)
  find_package(Glibmm REQUIRED)
  find_package(LibXML++ REQUIRED)
  find_package(UUID REQUIRED)
  find_package(Casablanca REQUIRED)

  if(BUILD_TESTS)
    find_package(UnitTest++ REQUIRED)
  endif()

  option(BUILD_SHARED_LIBS "Build shared Libraries." ON)
else()
  message("-- Unsupported Build Platform.")
endif()

# Compiler (not platform) specific settings
if("${CMAKE_CXX_COMPILER_ID}" MATCHES "GNU")
  message("-- Setting gcc options")

  set(WARNINGS "-Wall -Wextra -Wunused-parameter -Wcast-align -Wcast-qual -Wconversion -Wformat=2 -Winit-self -Winvalid-pch -Wmissing-format-attribute -Wmissing-include-dirs -Wpacked -Wredundant-decls -Wunreachable-code")
  set(LINUX_SUPPRESSIONS "-Wno-deprecated -Wno-unknown-pragmas -Wno-reorder -Wno-unused-function -Wno-char-subscripts -Wno-switch -Wno-unused-but-set-parameter -Wno-unused-value -Wno-unused-local-typedefs -Wno-unused-parameter")
  set(WARNINGS "${WARNINGS} ${LINUX_SUPPRESSIONS}")

  set(LD_FLAGS "${LD_FLAGS} -Wl,-z,defs")
 
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -fno-strict-aliasing")
 
  set(STRICT_CXX_FLAGS ${WARNINGS} "-Werror -pedantic")
 
  add_definitions(-DBOOST_LOG_DYN_LINK)
else()
  message("-- Unknown compiler, success is doubtful.")
endif()

# Reconfigure final output directory
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/Binaries)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/Binaries)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/Binaries)

set(AZURESTORAGE_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/includes)
set(AZURESTORAGE_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/includes ${CASABLANCA_INCLUDE_DIRS} ${Boost_INCLUDE_DIRS} ${OPENSSL_INCLUDE_DIRS} ${LibXML++_INCLUDE_DIRS} ${UUID_INCLUDE_DIRS} ${Glibmm_INCLUDE_DIRS})

set(AZURESTORAGE_LIBRARY azurestorage)
set(AZURESTORAGE_LIBRARIES ${AZURESTORAGE_LIBRARY} ${CASABLANCA_LIBRARIES} ${Boost_LIBRARIES} ${Boost_FRAMEWORK} ${OPENSSL_LIBRARIES} ${LibXML++_LIBRARIES} ${UUID_LIBRARIES} ${Glibmm_LIBRARIES})

# Set version numbers centralized
set (AZURESTORAGE_VERSION_MAJOR 2)
set (AZURESTORAGE_VERSION_MINOR 1)
set (AZURESTORAGE_VERSION_REVISION 0)

# Add sources per configuration
add_subdirectory(src)

if(BUILD_TESTS)
  set(AZURESTORAGE_LIBRARY_TEST azurestoragetest)
  add_subdirectory(tests) 
endif()

if(BUILD_SAMPLES)
  add_subdirectory(samples)
endif()
