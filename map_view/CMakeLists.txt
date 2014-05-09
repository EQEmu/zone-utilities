CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

SET(map_view_sources
	camera.cpp
	main.cpp
	map.cpp
	model.cpp
	overlay.cpp
	shader.cpp
)

SET(map_view_headers
	camera.h
	map.h
	model.h
	overlay.h
	shader.h
)

ADD_EXECUTABLE(map_view ${map_view_sources} ${map_view_headers})

INSTALL(TARGETS map_view RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX})

TARGET_LINK_LIBRARIES(map_view common glfw glew ${GLFW_LIBRARIES} ${ZLIB_LIBRARY} ${FREEIMAGE_LIBRARIES})

SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)