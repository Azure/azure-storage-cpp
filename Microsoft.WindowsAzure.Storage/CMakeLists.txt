set(CMAKE_LEGACY_CYGWIN_WIN32 0)

if(WIN32)
  cmake_minimum_required(VERSION 3.8)
else()
  cmake_minimum_required(VERSION 2.6)
endif()

project(azurestorage)

enable_testing()

set(WARNINGS)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_LIST_DIR}/cmake/Modules/")

option(BUILD_TESTS "Build test codes" OFF)
option(BUILD_SAMPLES "Build sample codes" OFF)

# Platform (not compiler) specific settings
if(UNIX)
  find_package(Boost REQUIRED COMPONENTS log log_setup random system thread locale regex filesystem chrono date_time)
  find_package(Threads REQUIRED)
  if(APPLE AND NOT OPENSSL_ROOT_DIR)
    # Prefer a homebrew version of OpenSSL over the one in /usr/lib
    file(GLOB OPENSSL_ROOT_DIR /usr/local/Cellar/openssl/*)

  # Prefer the latest (make the latest one first)
    list(REVERSE OPENSSL_ROOT_DIR)


  if(NOT GETTEXT_LIB_DIR)
    message(WARNING "No GETTEXT_LIB_DIR specified, assuming: /usr/local/opt/gettext/lib")
    set(GETTEXT_LIB_DIR "/usr/local/opt/gettext/lib")
  endif()
  # If we didn't find it where homebrew would put it, and it hasn't been specified, then we have to throw an error
  if(NOT IS_DIRECTORY "${GETTEXT_LIB_DIR}")
    message(ERROR "We couldn't find your gettext lib directory (${GETTEXT_LIB_DIR}). Please re-run cmake with -DGETTEXT_LIB_DIR=<your gettext lib dir>. This is usually where libintl.a and libintl.dylib reside.")
  endif()

  # if we actually have a GETTEXT_LIB_DIR we add the linker flag for it
  set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -L${GETTEXT_LIB_DIR}")
  endif()

  set(_OPENSSL_VERSION "")
  find_package(OpenSSL 1.0.0 REQUIRED)


  find_package(UUID REQUIRED)
  find_package(Casablanca REQUIRED)
  find_package(LibXML2 REQUIRED)

  if(BUILD_TESTS)
    find_package(UnitTest++ REQUIRED)
  endif()


elseif(WIN32)
  message("-- Setting WIN32 options")
  find_package(Casablanca REQUIRED)
  add_definitions(-DUNICODE -D_UNICODE -D_WIN32)
else()
  message("-- Unsupported Build Platform.")
endif()

if(WIN32 OR UNIX)
  option(BUILD_SHARED_LIBS "Build shared Libraries." ON)
  option(WASTORE_INSTALL_HEADERS "Install header files." ON)

  file(GLOB WAS_HEADERS includes/was/*.h)
  install(FILES ${WAS_HEADERS} DESTINATION include/was)
  file(GLOB WASCORE_HEADERS includes/wascore/*.h)
  install(FILES ${WASCORE_HEADERS} DESTINATION include/wascore)
  file(GLOB WASCORE_DATA includes/wascore/*.dat)
  install(FILES ${WASCORE_DATA} DESTINATION include/wascore)
endif()

# Compiler (not platform) specific settings
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
  message("-- Setting gcc options")

  set(WARNINGS "-Wall -Wextra -Wunused-parameter -Wcast-align -Wcast-qual -Wconversion -Wformat=2 -Winit-self -Winvalid-pch -Wmissing-format-attribute -Wmissing-include-dirs -Wpacked -Wredundant-decls -Wunreachable-code")
  set(LINUX_SUPPRESSIONS "-Wno-deprecated -Wno-unknown-pragmas -Wno-reorder -Wno-unused-function -Wno-char-subscripts -Wno-switch -Wno-unused-but-set-parameter -Wno-unused-value -Wno-unused-local-typedefs -Wno-unused-parameter")
  set(WARNINGS "${WARNINGS} ${LINUX_SUPPRESSIONS}")

  set(LD_FLAGS "${LD_FLAGS} -Wl,-z,defs")

  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -fno-strict-aliasing")

  set(STRICT_CXX_FLAGS ${WARNINGS} "-Werror -pedantic")

  if (BUILD_SHARED_LIBS)
    add_definitions(-DBOOST_LOG_DYN_LINK)
  endif()
  add_definitions(-D_TURN_OFF_PLATFORM_STRING)
elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  message("-- Setting clang options")

  set(WARNINGS "-Wall -Wextra -Wcast-qual -Wconversion -Wformat=2 -Winit-self -Winvalid-pch -Wmissing-format-attribute -Wmissing-include-dirs -Wpacked -Wredundant-decls")
  set(OSX_SUPPRESSIONS "-Wno-overloaded-virtual -Wno-sign-conversion -Wno-deprecated -Wno-unknown-pragmas -Wno-reorder -Wno-char-subscripts -Wno-switch -Wno-unused-parameter -Wno-unused-variable -Wno-deprecated -Wno-unused-value -Wno-unknown-warning-option -Wno-return-type-c-linkage -Wno-unused-function -Wno-sign-compare -Wno-shorten-64-to-32 -Wno-reorder -Wno-unused-local-typedefs")
  set(WARNINGS "${WARNINGS} ${OSX_SUPPRESSIONS}")

  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++ -Wno-return-type-c-linkage -Wno-unneeded-internal-declaration")
  set(CMAKE_XCODE_ATTRIBUTE_CLANG_CXX_LIBRARY "libc++")
  set(CMAKE_XCODE_ATTRIBUTE_CLANG_CXX_LANGUAGE_STANDARD "c++11")

  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -fno-strict-aliasing")
  if (BUILD_SHARED_LIBS)
    add_definitions(-DBOOST_LOG_DYN_LINK)
  endif()
  add_definitions(-D_TURN_OFF_PLATFORM_STRING)
elseif(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
  message("-- Setting MSVC options")
  add_compile_options(/bigobj)
  add_compile_options(/MP)
  if(BUILD_SHARED_LIBS)
    add_definitions(-DWASTORAGE_DLL -D_USRDLL)
  else()
    add_definitions(-D_NO_WASTORAGE_API)
  endif()

  if(ENABLE_MT)
    set(CompilerFlags
            CMAKE_CXX_FLAGS
            CMAKE_CXX_FLAGS_DEBUG
            CMAKE_CXX_FLAGS_RELEASE
            CMAKE_C_FLAGS
            CMAKE_C_FLAGS_DEBUG
            CMAKE_C_FLAGS_RELEASE
    )
    foreach(CompilerFlag ${CompilerFlags})
      string(REPLACE "/MD" "/MT" ${CompilerFlag} "${${CompilerFlag}}")
    endforeach()
  endif()
else()
  message("-- Unknown compiler, success is doubtful.")
endif()

# Reconfigure final output directory
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/Binaries)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/Binaries)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/Binaries)

set(AZURESTORAGE_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/includes)
set(AZURESTORAGE_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/includes ${CASABLANCA_INCLUDE_DIR} ${Boost_INCLUDE_DIRS} ${OPENSSL_INCLUDE_DIRS} ${UUID_INCLUDE_DIRS} ${LibXML2_INCLUDE_DIR})


set(AZURESTORAGE_LIBRARY azurestorage)
set(AZURESTORAGE_LIBRARIES ${AZURESTORAGE_LIBRARY} ${CASABLANCA_LIBRARY} ${Boost_LIBRARIES} ${Boost_FRAMEWORK} ${OPENSSL_LIBRARIES} ${UUID_LIBRARIES} ${LibXML2_LIBRARY} ${CMAKE_THREAD_LIBS_INIT})

# Set version numbers centralized
set (AZURESTORAGE_VERSION_MAJOR 7)
set (AZURESTORAGE_VERSION_MINOR 5)
set (AZURESTORAGE_VERSION_REVISION 0)

# Set output directories.
if(NOT DEFINED CMAKE_INSTALL_BINDIR)
  set(CMAKE_INSTALL_BINDIR bin)
endif()

if(NOT DEFINED CMAKE_INSTALL_LIBDIR)
  set(CMAKE_INSTALL_LIBDIR lib)
endif()

# Add sources per configuration
add_subdirectory(src)

if(BUILD_TESTS)
  set(AZURESTORAGE_LIBRARY_TEST azurestoragetest)
  add_subdirectory(tests)
endif()

if(BUILD_SAMPLES)
  set(AZUREAZURAGE_LIBRARY_SAMPLE azurestoragesample)
  add_subdirectory(samples)
endif()

