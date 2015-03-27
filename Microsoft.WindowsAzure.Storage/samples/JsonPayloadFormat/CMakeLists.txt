include_directories(. ${AZURESTORAGESAMPLES_INCLUDE_DIRS})

# THE ORDER OF FILES IS VERY /VERY/ IMPORTANT
if(UNIX)
  set(SOURCES
     Application.cpp
     stdafx.cpp
    )
endif()

buildsample(${AZURESTORAGESAMPLES_JSON} ${SOURCES})