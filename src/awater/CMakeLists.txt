CMAKE_MINIMUM_REQUIRED(VERSION 3.10.2)

SET(awater_sources
	awater.cpp
	water_map.cpp
)

SET(awater_headers
	water_map.h
)

ADD_EXECUTABLE(awater ${awater_sources} ${awater_headers})

INSTALL(TARGETS awater RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX})

TARGET_LINK_LIBRARIES(awater PRIVATE common)
TARGET_LINK_LIBRARIES(awater PRIVATE log)

SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)