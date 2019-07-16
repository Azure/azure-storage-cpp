include_directories(. ${AZURESTORAGESAMPLES_INCLUDE_DIRS})

if(UNIX)
  set(SOURCES Application.cpp)
endif()

buildsample(${AZURESTORAGESAMPLES_OAUTH} ${SOURCES})
