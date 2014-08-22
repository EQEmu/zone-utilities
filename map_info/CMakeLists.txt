CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

SET(map_info_sources
	main.cpp
	map.cpp
	oriented_bounding_box.cpp
	RaycastMesh.cpp
	water_map.cpp
	water_map_v1.cpp
	water_map_v2.cpp
)

SET(map_info_headers
	map.h
	oriented_bounding_box.h
	RaycastMesh.h
	water_map.h
	water_map_v1.h
	water_map_v2.h
)

ADD_EXECUTABLE(map_info ${map_info_sources} ${map_info_headers})

INSTALL(TARGETS map_info RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX})

TARGET_LINK_LIBRARIES(map_info common log ${ZLIB_LIBRARY} ${LIBWEBSOCKETS_LIBRARIES})

SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)