CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

SET(map_info_sources
	main.cpp
	map.cpp
	RaycastMesh.cpp
)

SET(map_info_headers
	map.h
	RaycastMesh.h
)

ADD_EXECUTABLE(map_info ${map_info_sources} ${map_info_headers})

INSTALL(TARGETS map_info RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX})

TARGET_LINK_LIBRARIES(map_info common log ${ZLIB_LIBRARY} ${LIBWEBSOCKETS_LIBRARIES})

SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)