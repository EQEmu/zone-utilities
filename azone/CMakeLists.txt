CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

SET(azone_sources
	azone.cpp
	map.cpp
)

SET(azone_headers
	map.h
)

ADD_EXECUTABLE(azone ${azone_sources} ${azone_headers})

INSTALL(TARGETS azone RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX})

TARGET_LINK_LIBRARIES(azone common log ${ZLIB_LIBRARY})

SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)