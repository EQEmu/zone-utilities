CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

SET(map_view_sources
	camera.cpp
	imgui_gflw.cpp
	main.cpp
	map.cpp
	model.cpp
	shader.cpp
)

SET(map_view_headers
	camera.h
	imgui_gflw.h
	map.h
	model.h
	shader.h
)

ADD_EXECUTABLE(map_view ${map_view_sources} ${map_view_headers})

INSTALL(TARGETS map_view RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX})

TARGET_LINK_LIBRARIES(map_view common log glfw glew imgui ${GLFW_LIBRARIES} ${ZLIB_LIBRARY})

SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)