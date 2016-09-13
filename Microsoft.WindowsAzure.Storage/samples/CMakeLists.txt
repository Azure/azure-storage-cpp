# Reconfigure final output directory
set(AZURESTORAGESAMPLES_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/SamplesCommon)
set(AZURESTORAGESAMPLES_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/SamplesCommon ${AZURESTORAGE_INCLUDE_DIRS})

set(AZURESTORAGESAMPLES_COMMON samplescommon)
set(AZURESTORAGESAMPLES_BLOBS samplesblobs)
set(AZURESTORAGESAMPLES_JSON samplesjson)
set(AZURESTORAGESAMPLES_TABLES samplestables)
set(AZURESTORAGESAMPLES_QUEUES samplesqueues)
set(AZURESTORAGESAMPLES_FILES samplesfiles)
set(AZURESTORAGESAMPLES_LIBRARIES ${AZURESTORAGESAMPLES_COMMON} ${AZURESTORAGE_LIBRARIES})

macro (buildsample SAMPLE_NAME SAMPLE_SOURCES)

  if("${SAMPLE_NAME}" MATCHES "${AZURESTORAGESAMPLES_COMMON}")
    add_library(${SAMPLE_NAME} ${SAMPLE_SOURCES})
    target_link_libraries(${AZURESTORAGESAMPLES_LIBRARIES})
  else()
    add_executable(${SAMPLE_NAME} ${SAMPLE_SOURCES})
    target_link_libraries(${SAMPLE_NAME} ${AZURESTORAGESAMPLES_LIBRARIES})
  endif()

  # Portions specific to cpprest binary versioning.
  if(UNIX)
    set_target_properties(${SAMPLE_NAME} PROPERTIES
      SOVERSION ${AZURESTORAGE_VERSION_MAJOR}.${AZURESTORAGE_VERSION_MINOR})
  endif()
endmacro()

add_subdirectory(SamplesCommon)
add_subdirectory(BlobsGettingStarted)
add_subdirectory(JsonPayloadFormat)
add_subdirectory(QueuesGettingStarted)
add_subdirectory(TablesGettingStarted)
add_subdirectory(FilesGettingStarted)
