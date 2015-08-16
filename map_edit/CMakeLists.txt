CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

SET(map_edit_sources
	imgui_glfw.cpp
	main.cpp
	module_navigation.cpp
	nav_mesh_model.cpp
	rc_chunky_tri_mesh.cpp
	scene.cpp
	shader.cpp
	static_geometry.cpp
	volume_box.cpp
)

SET(map_edit_headers
	entity.h
	imgui_glfw.h
	module.h
	module_navigation.h
	nav_mesh_model.h
	rc_chunky_tri_mesh.h
	scene.h
	scene_module.h
	shader.h
	static_geometry.h
	volume_box.h
	volume_navigation.h
)

ADD_EXECUTABLE(map_edit ${map_edit_sources} ${map_edit_headers})

INSTALL(TARGETS map_edit RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX})

TARGET_LINK_LIBRARIES(map_edit common log glew imgui glfw ${GLFW_LIBRARIES} ${ZLIB_LIBRARY} ${EQEMU_BULLET} recast_navigation)

SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
