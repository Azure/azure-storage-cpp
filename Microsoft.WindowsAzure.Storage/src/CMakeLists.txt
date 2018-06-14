include_directories(${Boost_INCLUDE_DIR} ${OPENSSL_INCLUDE_DIR})
include_directories(${AZURESTORAGE_INCLUDE_DIRS})

# THE ORDER OF FILES IS VERY /VERY/ IMPORTANT
if(UNIX OR WIN32)
  set(SOURCES
     xml_wrapper.cpp
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
     resources.cpp
     queue_request_factory.cpp
     file_request_factory.cpp
     file_response_parsers.cpp
     protocol_json.cpp
     operation_context.cpp
     mime_multipart_helper.cpp
     logging.cpp
     hashing.cpp
     constants.cpp
     streams.cpp
     cloud_file_ostreambuf.cpp
     cloud_file.cpp
     cloud_file_directory.cpp
     cloud_file_share.cpp
     cloud_file_client.cpp
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
     cloud_append_blob.cpp
     blob_response_parsers.cpp
     blob_request_factory.cpp
     basic_types.cpp
     authentication.cpp
     cloud_common.cpp
    )
endif()

if ("${CMAKE_BUILD_TYPE}" MATCHES "Debug")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fprofile-arcs -ftest-coverage")
endif()
if (APPLE)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${WARNINGS}")
else()
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
endif()

if(MSVC)
  add_compile_options(/Yustdafx.h)
  set_source_files_properties(stdafx.cpp PROPERTIES COMPILE_FLAGS "/Ycstdafx.h")

  if (NOT CMAKE_GENERATOR MATCHES "Visual Studio .*")
    set_property(SOURCE stdafx.cpp APPEND PROPERTY OBJECT_OUTPUTS "${CMAKE_CURRENT_BINARY_DIR}/stdafx.pch")
    set_property(SOURCE ${SOURCES} APPEND PROPERTY OBJECT_DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/stdafx.pch")
  endif()

  list(APPEND SOURCES stdafx.cpp)
endif()

add_library(${AZURESTORAGE_LIBRARY} ${SOURCES})

target_link_libraries(${AZURESTORAGE_LIBRARIES})

if(WIN32)
  target_link_libraries(${AZURESTORAGE_LIBRARY} Ws2_32.lib rpcrt4.lib xmllite.lib bcrypt.lib)
endif()

# Portions specific to azure storage binary versioning and installation.
if(UNIX)
  set_target_properties(${AZURESTORAGE_LIBRARY} PROPERTIES
    SOVERSION ${AZURESTORAGE_VERSION_MAJOR}
    VERSION ${AZURESTORAGE_VERSION_MAJOR}.${AZURESTORAGE_VERSION_MINOR})

elseif(WIN32)
  set_target_properties(${AZURESTORAGE_LIBRARY} PROPERTIES OUTPUT_NAME "wastorage")
endif()

install(FILES ${WAS_HEADERS} DESTINATION include/was)
install(FILES ${WASCORE_HEADERS} DESTINATION include/wascore)
install(FILES ${WASCORE_DATA} DESTINATION include/wascore)

install(
  TARGETS ${AZURESTORAGE_LIBRARY}
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
