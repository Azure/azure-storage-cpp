include_directories(${AZURESTORAGE_INCLUDE_DIRS})

# THE ORDER OF FILES IS VERY /VERY/ IMPORTANT
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

if ("${CMAKE_BUILD_TYPE}" MATCHES "Debug")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fprofile-arcs -ftest-coverage")
endif()

add_library(${AZURESTORAGE_LIBRARY} ${SOURCES})

if(WIN32)
  if (NOT BUILD_SHARED_LIBS)
    add_definitions(-D_NO_WASTORAGE_API)
  endif()
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /bigobj")
  set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /bigobj")
endif()

target_link_libraries(${AZURESTORAGE_LIBRARIES})

# Portions specific to cpprest binary versioning.
if(UNIX)
  set_target_properties(${AZURESTORAGE_LIBRARY} PROPERTIES
    SOVERSION ${AZURESTORAGE_VERSION_MAJOR}.${AZURESTORAGE_VERSION_MINOR})
endif()

