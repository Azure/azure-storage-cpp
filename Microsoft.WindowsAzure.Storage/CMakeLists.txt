set(CMAKE_LEGACY_CYGWIN_WIN32 0)
cmake_minimum_required(VERSION 2.6)
project(azurestorage)

enable_testing()

set(WARNINGS)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")

# Platform (not compiler) specific settings
if(IOS)
  set(IOS_SOURCE_DIR "${CMAKE_SOURCE_DIR}/../Build_iOS")
  set(Boost_FRAMEWORK "-F ${IOS_SOURCE_DIR} -framework boost")
  set(Boost_INCLUDE_DIR "${IOS_SOURCE_DIR}/boost.framework/Headers")

  set(OPENSSL_FOUND 1)
  set(OPENSSL_INCLUDE_DIR "${IOS_SOURCE_DIR}/openssl/include")
  set(OPENSSL_LIBRARIES
    "${IOS_SOURCE_DIR}/openssl/lib/libcrypto.a"
    "${IOS_SOURCE_DIR}/openssl/lib/libssl.a"
    )

  set(CASABLANCA_INCLUDE_DIRS "${IOS_SOURCE_DIR}/casablanca/include")
  set(CASABLANCA_LIBRARIES "${IOS_SOURCE_DIR}/casablanca/lib/libcasablanca.a")

  # The cxx_flags must be set here, because the ios-cmake toolchain file unfortunately sets "-headerpad_max_install_names" which is not a valid clang flag.
  set(CMAKE_CXX_FLAGS "-fvisibility=hidden -fvisibility-inlines-hidden")

  set(BUILD_SHARED_LIBS OFF)
elseif(UNIX) # This includes OSX
  find_package(Boost REQUIRED COMPONENTS log log_setup random system thread locale regex filesystem)
  find_package(Threads REQUIRED)
  find_package(OpenSSL REQUIRED)
  find_package(LibXML++ REQUIRED)
  find_package(UUID REQUIRED)
  find_package(Casablanca REQUIRED)

  set(UNITTEST_LIBRARY "${CMAKE_SOURCE_DIR}/tests/UnitTest++/libUnitTest++.a")

  option(BUILD_SHARED_LIBS "Build shared Libraries." ON)
elseif(WIN32)
  option(BUILD_SHARED_LIBS "Build shared Libraries." ON)

  add_definitions(-DUNICODE)

  if(NOT BUILD_SHARED_LIBS)
    # This causes cmake to not link the test libraries separately, but instead hold onto their object files.
    set(TEST_LIBRARY_TARGET_TYPE OBJECT)
  endif()

  set(LIB lib)
else()
  message("-- Unsupported Build Platform.")
endif()

# Compiler (not platform) specific settings
if(("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang") OR IOS)
  message("-- Setting clang options")

  set(WARNINGS "-Wall -Wextra -Wcast-qual -Wconversion -Wformat=2 -Winit-self -Winvalid-pch -Wmissing-format-attribute -Wmissing-include-dirs -Wpacked -Wredundant-decls")
  set(OSX_SUPPRESSIONS "-Wno-overloaded-virtual -Wno-sign-conversion -Wno-deprecated -Wno-unknown-pragmas -Wno-reorder -Wno-char-subscripts -Wno-switch -Wno-unused-parameter -Wno-unused-variable -Wno-deprecated -Wno-unused-value -Wno-unknown-warning-option -Wno-return-type-c-linkage -Wno-unused-function -Wno-sign-compare -Wno-shorten-64-to-32 -Wno-reorder")
  set(WARNINGS "${WARNINGS} ${OSX_SUPPRESSIONS}")

  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++ -Wno-return-type-c-linkage -Wno-unneeded-internal-declaration")
  set(CMAKE_XCODE_ATTRIBUTE_CLANG_CXX_LIBRARY "libc++")
  set(CMAKE_XCODE_ATTRIBUTE_CLANG_CXX_LANGUAGE_STANDARD "c++11")

  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -fno-strict-aliasing")
  set(STRICT_CXX_FLAGS ${WARNINGS} "-Werror -pedantic")
elseif("${CMAKE_CXX_COMPILER_ID}" MATCHES "GNU")
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
set(AZURESTORAGE_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/includes ${CASABLANCA_INCLUDE_DIRS} ${Boost_INCLUDE_DIR} ${OPENSSL_INCLUDE_DIR} ${LibXML++_INCLUDE_DIRS} ${UUID_INCLUDE_DIRS})

set(AZURESTORAGE_LIBRARY azurestorage)
set(AZURESTORAGE_LIBRARY_TEST azurestoragetest)
set(AZURESTORAGE_LIBRARIES ${AZURESTORAGE_LIBRARY} ${CASABLANCA_LIBRARIES} ${Boost_LIBRARIES} ${Boost_FRAMEWORK} ${OPENSSL_LIBRARIES} ${LibXML++_LIBRARIES} ${UUID_LIBRARIES})

include_directories(${AZURESTORAGE_INCLUDE_DIRS})

add_subdirectory(src)
add_subdirectory(tests) 
