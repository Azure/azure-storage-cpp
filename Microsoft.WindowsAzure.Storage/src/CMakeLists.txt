include_directories(../includes ${CASABLANCA_INCLUDE_DIR} ${Boost_INCLUDE_DIR} ${LibXML++_INCLUDE_DIRS})

# THE ORDER OF FILES IS VERY /VERY/ IMPORTANT
if(UNIX)
  set(SOURCES
     xmlhelpers.cpp
     response_parsers.cpp
     request_result.cpp
     request_factory.cpp
     protocol_xml.cpp
     navigation.cpp
     async_semaphore.cpp
     util.cpp
     table_request_factory.cpp
     table_response_parsers.cpp
     table_query.cpp
     entity_property.cpp
     shared_access_signature.cpp
     retry_policies.cpp
     queue_request_factory.cpp
     protocol_json.cpp
     operation_context.cpp
     mime_multipart_helper.cpp
     logging.cpp
     hashing.cpp
     cloud_table_client.cpp
     cloud_table.cpp
     cloud_storage_account.cpp
     cloud_queue_message.cpp
     cloud_queue_client.cpp
     cloud_queue.cpp
     cloud_page_blob.cpp
     cloud_core.cpp
     cloud_client.cpp
     cloud_block_blob.cpp
     cloud_blob_directory.cpp
     cloud_blob_container.cpp
     cloud_blob_shared.cpp
     cloud_blob_ostreambuf.cpp
     cloud_blob_istreambuf.cpp
     cloud_blob_client.cpp
     cloud_blob.cpp
     blob_response_parsers.cpp
     blob_request_factory.cpp
     basic_types.cpp
     authentication.cpp
     cloud_common.cpp
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
     xmlhelpers.cpp
     response_parsers.cpp
     request_result.cpp
     request_factory.cpp
     protocol_xml.cpp
     navigation.cpp
     async_semaphore.cpp
     util.cpp
     table_request_factory.cpp
     table_response_parsers.cpp
     table_query.cpp
     entity_property.cpp
     shared_access_signature.cpp
     retry_policies.cpp
     queue_request_factory.cpp
     protocol_json.cpp
     operation_context.cpp
     mime_multipart_helper.cpp
     cloud_table_client.cpp
     cloud_table.cpp
     cloud_storage_account.cpp
     cloud_queue_message.cpp
     cloud_queue_client.cpp
     cloud_queue.cpp
     cloud_page_blob.cpp
     cloud_core.cpp
     cloud_client.cpp
     cloud_block_blob.cpp
     cloud_blob_directory.cpp
     cloud_blob_container.cpp
     cloud_blob_shared.cpp
     cloud_blob_ostreambuf.cpp
     cloud_blob_istreambuf.cpp
     cloud_blob_client.cpp
     cloud_blob.cpp
     blob_response_parsers.cpp
     blob_request_factory.cpp
     basic_types.cpp
     authentication.cpp
     cloud_common.cpp
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

add_library(${AZURESTORAGE_LIBRARY} ${SOURCES})

target_link_libraries(${AZURESTORAGE_LIBRARY}
  ${CMAKE_THREAD_LIBS_INIT}
  ${CASABLANCA_LIBRARY}
  ${Boost_LIBRARIES}
  ${Boost_FRAMEWORK}
  ${LibXML++_LIBRARIES}
  ${COREFOUNDATION}
  ${EXTRALINKS}
  )

# Portions specific to cpprest binary versioning.
set (AZURESTORAGE_VERSION_MAJOR 0)
set (AZURESTORAGE_VERSION_MINOR 5)
set (AZURESTORAGE_VERSION_REVISION 0)
if(WIN32)
  set_target_properties(${AZURESTORAGE_LIBRARY} PROPERTIES
    OUTPUT_NAME "${AZURESTORAGE_LIBRARY}_${AZURESTORAGE_VERSION_MAJOR}_${AZURESTORAGE_VERSION_MINOR}")
else()
  set_target_properties(${AZURESTORAGE_LIBRARY} PROPERTIES
    SOVERSION ${AZURESTORAGE_VERSION_MAJOR}.${AZURESTORAGE_VERSION_MINOR})
endif()

