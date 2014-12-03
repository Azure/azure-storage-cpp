include_directories(../includes ${CASABLANCA_INCLUDE_DIR} ${Boost_INCLUDE_DIR} ${LibXML++_INCLUDE_DIRS} UnitTest++/src)

# THE ORDER OF FILES IS VERY /VERY/ IMPORTANT
if(UNIX)
  set(SOURCES
     blob_lease_test.cpp
     blob_streams_test.cpp
     blob_test_base.cpp
     cloud_blob_client_test.cpp
     cloud_blob_container_test.cpp
     cloud_blob_directory_test.cpp
     cloud_blob_test.cpp
     cloud_block_blob_test.cpp
     cloud_page_blob_test.cpp
     cloud_queue_client_test.cpp
     cloud_queue_test.cpp
     cloud_storage_account_test.cpp
     cloud_table_client_test.cpp
     cloud_table_test.cpp
     executor_test.cpp
     main.cpp
     read_from_secondary_test.cpp
     retry_policy_test.cpp
     service_properties_test.cpp
     stdafx.cpp
     test_base.cpp
     test_helper.cpp
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
     blob_lease_test.cpp
     blob_streams_test.cpp
     blob_test_base.cpp
     cloud_blob_client_test.cpp
     cloud_blob_container_test.cpp
     cloud_blob_directory_test.cpp
     cloud_blob_test.cpp
     cloud_block_blob_test.cpp
     cloud_page_blob_test.cpp
     cloud_queue_client_test.cpp
     cloud_queue_test.cpp
     cloud_storage_account_test.cpp
     cloud_table_client_test.cpp
     cloud_table_test.cpp
     executor_test.cpp
     main.cpp
     read_from_secondary_test.cpp
     retry_policy_test.cpp
     service_properties_test.cpp
     stdafx.cpp
     test_base.cpp
     test_helper.cpp
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

add_executable(${AZURESTORAGE_LIBRARY_TEST} ${SOURCES})

target_link_libraries(${AZURESTORAGE_LIBRARY_TEST}
  ${AZURESTORAGE_LIBRARY}
  ${UNITTEST_LIBRARY}
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
  set_target_properties(${AZURESTORAGE_LIBRARY_TEST} PROPERTIES
    OUTPUT_NAME "${AZURESTORAGE_LIBRARY_TEST}")
else()
  set_target_properties(${AZURESTORAGE_LIBRARY_TEST} PROPERTIES
    SOVERSION ${AZURESTORAGE_VERSION_MAJOR}.${AZURESTORAGE_VERSION_MINOR})
endif()

