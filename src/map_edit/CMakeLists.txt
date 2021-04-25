CMAKE_MINIMUM_REQUIRED(VERSION 3.10.2)

SET(map_edit_sources
	debug_draw.cpp
	dynamic_geometry.cpp
	imgui_glfw.cpp
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
	imgui_glfw.h
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

TARGET_LINK_LIBRARIES(map_edit PRIVATE ${BULLET_LIBRARIES})
TARGET_LINK_LIBRARIES(map_edit PRIVATE fmt::fmt)
TARGET_LINK_LIBRARIES(map_edit PRIVATE GLEW::GLEW)
TARGET_LINK_LIBRARIES(map_edit PRIVATE glfw)
TARGET_LINK_LIBRARIES(map_edit PRIVATE glm::glm)
TARGET_LINK_LIBRARIES(map_edit PRIVATE imgui::imgui)
TARGET_LINK_LIBRARIES(map_edit PRIVATE unofficial::libuv::libuv)
TARGET_LINK_LIBRARIES(map_edit PRIVATE ${OPENGL_gl_LIBRARY})
TARGET_LINK_LIBRARIES(map_edit PRIVATE recastnavigation)
TARGET_LINK_LIBRARIES(map_edit PRIVATE ZLIB::ZLIB)

IF(WIN32)
	TARGET_LINK_LIBRARIES(map_edit PRIVATE ws2_32)
	TARGET_LINK_LIBRARIES(map_edit PRIVATE psapi)
	TARGET_LINK_LIBRARIES(map_edit PRIVATE iphlpapi)
	TARGET_LINK_LIBRARIES(map_edit PRIVATE userenv)
ENDIF(WIN32)

SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
