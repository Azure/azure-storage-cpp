include_directories(../../includes ${CASABLANCA_INCLUDE_DIR} ${Boost_INCLUDE_DIR} ${LibXML++_INCLUDE_DIRS})

# THE ORDER OF FILES IS VERY /VERY/ IMPORTANT
if(UNIX)
  set(SOURCES
     Application.cpp
     stdafx.cpp
    )
  if(APPLE)
    # set(SOURCES ${SOURCES} apple.cpp)
    find_library(COREFOUNDATION CoreFoundation "/")
    set(EXTRALINKS ${COREFOUNDATION})
  else()
    # set(SOURCES ${SOURCES} linux.cpp)
  endif()
elseif(WIN32)
  set(SOURCES
     Application.cpp
     stdafx.cpp
    )
  add_definitions(
    -D_ASYNCRT_EXPORT
    -DAZURESTORAGESERVICES_EXPORTS
    -D_PPLX_EXPORT
    -DWIN32
    -D_MBCS
    -D_USRDLL
    )
  set(EXTRALINKS Httpapi.lib Winhttp.dll)
endif()

set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${WARNINGS} -Werror -pedantic")

add_executable(${AZURESTORAGESAMPLES_BLOBS} ${SOURCES})

target_link_libraries(${AZURESTORAGESAMPLES_BLOBS}
  ${AZURESTORAGESAMPLES_COMMON}
  ${AZURE_STORAGE_LIBRARY}
  ${UUID_LIBRARY}
  ${CMAKE_THREAD_LIBS_INIT}
  ${CASABLANCA_LIBRARY}
  ${Boost_LIBRARIES}
  ${Boost_FRAMEWORK}
  ${LibXML++_LIBRARIES}
  ${COREFOUNDATION}
  ${EXTRALINKS}
  )

# Portions specific to cpprest binary versioning.
if(WIN32)
  set_target_properties(${AZURESTORAGESAMPLES_BLOBS} PROPERTIES
    OUTPUT_NAME "${AZURESTORAGESAMPLES_BLOBS}")
else()
  set_target_properties(${AZURESTORAGESAMPLES_BLOBS} PROPERTIES
    SOVERSION ${AZURESTORAGE_VERSION_MAJOR}.${AZURESTORAGE_VERSION_MINOR})
endif()

