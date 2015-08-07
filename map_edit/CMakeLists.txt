CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

SET(map_edit_sources
	imgui_glfw.cpp
	main.cpp
	scene.cpp
	shader.cpp
	static_geometry.cpp
)

SET(map_edit_headers
	entity.h
	imgui_glfw.h
	scene.h
	scene_module.h
	shader.h
	static_geometry.h
)

ADD_EXECUTABLE(map_edit ${map_edit_sources} ${map_edit_headers})

INSTALL(TARGETS map_edit RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX})

TARGET_LINK_LIBRARIES(map_edit common log glew imgui glfw ${GLFW_LIBRARIES} ${ZLIB_LIBRARY} ${EQEMU_BULLET})

SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
