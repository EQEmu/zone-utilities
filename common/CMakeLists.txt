CMAKE_MINIMUM_REQUIRED(VERSION 3.10.2)

SET(common_sources
	compression.cpp
	config.cpp
	eq_physics.cpp
	eqg_loader.cpp
	eqg_model_loader.cpp
	eqg_v4_loader.cpp
	pfs.cpp
	pfs_crc.cpp
	s3d_loader.cpp
	string_util.cpp
	water_map.cpp
	water_map_v1.cpp
	water_map_v2.cpp
	wld_fragment.cpp
	zone_map.cpp
	math/eq_math.cpp
	math/oriented_bounding_box.cpp
	event/event_loop.cpp
)

SET(common_headers
	any.h
	compression.h
	config.h
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
	math/aligned_bounding_box.h
	math/eq_math.h
	math/oriented_bounding_box.h
	event/background_task.h
	event/event_loop.h
	event/timer.h
)

SOURCE_GROUP(core
	FILES
)

SOURCE_GROUP(eq
	FILES
)

SOURCE_GROUP(graphics
	FILES
)

SOURCE_GROUP(math
	FILES
	math/aligned_bounding_box.h
	math/eq_math.cpp
	math/eq_math.h
	math/oriented_bounding_box.cpp
	math/oriented_bounding_box.h
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
		FILES
		graphics/imgui_glfw.cpp
		graphics/imgui_glfw.h
	)
ENDIF()

ADD_LIBRARY(common ${common_sources} ${common_headers})


SET(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)
