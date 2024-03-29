CMAKE_MINIMUM_REQUIRED(VERSION 3.10.2)

SET(map_edit_sources
	debug_draw.cpp
	dynamic_geometry.cpp
	main.cpp
	module_navigation.cpp
	module_navigation_build_tile.cpp
	module_volume.cpp
	module_wp.cpp
	rc_chunky_tri_mesh.cpp
	scene.cpp
	shader.cpp
	static_geometry.cpp
)

SET(map_edit_headers
	debug_draw.h
	dynamic_geometry.h
	entity.h
	module.h
	module_navigation.h
	module_navigation_build_tile.h
	module_volume.h
	module_wp.h
	rc_chunky_tri_mesh.h
	scene.h
	scene_module.h
	shader.h
	static_geometry.h
)

ADD_EXECUTABLE(map_edit ${map_edit_sources} ${map_edit_headers})

INSTALL(TARGETS map_edit RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX})

TARGET_LINK_LIBRARIES(map_edit PRIVATE common)
TARGET_LINK_LIBRARIES(map_edit PRIVATE log)
TARGET_LINK_LIBRARIES(map_edit PRIVATE recastnavigation)

SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
