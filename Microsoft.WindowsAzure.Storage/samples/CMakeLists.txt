if(UNIX)
  set(SOURCES
    BlobsGettingStarted.cpp
    FilesGettingStarted.cpp
    JsonPayloadFormat.cpp
    main.cpp
    NativeClientLibraryDemo1.cpp
    NativeClientLibraryDemo2.cpp
    OAuthGettingStarted.cpp
    QueuesGettingStarted.cpp
    samples_common.h
    TablesGettingStarted.cpp
    FilesProperties.cpp
  )
endif()

add_executable(${AZUREAZURAGE_LIBRARY_SAMPLE} ${SOURCES})

target_include_directories(${AZUREAZURAGE_LIBRARY_SAMPLE} PRIVATE ${AZURESTORAGE_INCLUDE_DIRS})
target_link_libraries(${AZUREAZURAGE_LIBRARY_SAMPLE} ${AZURESTORAGE_LIBRARIES})
