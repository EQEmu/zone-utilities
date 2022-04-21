CMAKE_MINIMUM_REQUIRED(VERSION 3.10.2)

SET(common_sources
	compression.cpp
	config.cpp
	eq_math.cpp
	eq_physics.cpp
	eqg_loader.cpp
	eqg_model_loader.cpp
	eqg_v4_loader.cpp
	oriented_bounding_box.cpp
	pfs.cpp
	pfs_crc.cpp
	s3d_loader.cpp
	string_util.cpp
	water_map.cpp
	water_map_v1.cpp
	water_map_v2.cpp
	wld_fragment.cpp
	zone_map.cpp
	event/event_loop.cpp
)

SET(common_headers
	aligned_bounding_box.h
	any.h
	compression.h
	config.h
	eq_math.h
	eq_physics.h
	eqemu_endian.h
	eqg_geometry.h
	eqg_invis_wall.h
	eqg_loader.h
	eqg_material.h
	eqg_model_loader.h
	eqg_region.h
	eqg_structs.h
	eqg_terrain.h
	eqg_terrain_tile.h
	eqg_v4_loader.h
	eqg_water_sheet.h
	light.h
	octree.h
	oriented_bounding_box.h
	pfs.h
	pfs_crc.h
	placeable.h
	placeable_group.h
	safe_alloc.h
	s3d_bsp.h
	s3d_geometry.h
	s3d_loader.h
	s3d_skeleton_track.h
	s3d_texture.h
	s3d_texture_brush.h
	s3d_texture_brush_set.h
	string_util.h
	water_map.h
	water_map_v1.h
	water_map_v2.h
	wld_fragment_reference.h
	wld_fragment.h
	wld_structs.h
	zone_map.h
	event/background_task.h
	event/event_loop.h
	event/timer.h
)

IF(EQEMU_ENABLE_GL)
	SET(common_sources
		${common_sources}
		graphics/imgui_glfw.cpp
	)
	
	SET(common_headers
		${common_headers}
		graphics/imgui_glfw.h
	)
	
	SOURCE_GROUP(graphics
		graphics/imgui_glfw.cpp
		graphics/imgui_glfw.h
	)
ENDIF()

ADD_LIBRARY(common ${common_sources} ${common_headers})

TARGET_LINK_LIBRARIES(common PUBLIC glm::glm)
TARGET_LINK_LIBRARIES(common PUBLIC unofficial::libuv::libuv)
TARGET_LINK_LIBRARIES(common PUBLIC ZLIB::ZLIB)
TARGET_LINK_LIBRARIES(common PUBLIC ${OPENGL_gl_LIBRARY})
target_link_libraries(common PUBLIC LinearMath Bullet3Common BulletDynamics BulletCollision)

IF(EQEMU_ENABLE_GL)
	TARGET_LINK_LIBRARIES(common PUBLIC GLEW::GLEW)
ENDIF()

SET(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)
