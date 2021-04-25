CMAKE_MINIMUM_REQUIRED(VERSION 3.10.2)

SET(common_sources
	eq_physics.cpp
	eqg_loader.cpp
	eqg_model_loader.cpp
	eqg_v4_loader.cpp
	s3d_loader.cpp
	water_map.cpp
	water_map_v1.cpp
	water_map_v2.cpp
	wld_fragment.cpp
	zone_map.cpp
	core/compression.cpp
	core/config.cpp
	core/crc.cpp
	format/pfs.cpp
	math/distance.cpp
	math/matrix_op.cpp
	string/transform.cpp
)

SET(common_headers
	eq_physics.h
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
	water_map.h
	water_map_v1.h
	water_map_v2.h
	wld_fragment_reference.h
	wld_fragment.h
	wld_structs.h
	zone_map.h
	core/compression.h
	core/config.h
	core/crc.h
	event/background_task.h
	event/event_loop.h
	event/task.h
	event/timer.h
	format/pfs.h
	math/aligned_bounding_box.h
	math/constant.h
	math/distance.h
	math/matrix_op.h
	math/octree.h
	math/oriented_bounding_box.h
	string/transform.h
)

SOURCE_GROUP(core FILES
	core/compression.cpp
	core/compression.h
	core/config.cpp
	core/config.h
	core/crc.cpp
	core/crc.h
)

SOURCE_GROUP(event FILES
	event/background_task.h
	event/event_loop.h
	event/task.h
	event/timer.h
)

SOURCE_GROUP(format FILES 
	format/pfs.cpp
	format/pfs.h
)

SOURCE_GROUP(math FILES
	math/aligned_bounding_box.h
	math/constant.h
	math/distance.cpp
	math/distance.h
	math/matrix_op.cpp
	math/matrix_op.h
	math/octree.h
	math/oriented_bounding_box.h
)

SOURCE_GROUP(srtring FILES
	string/transform.cpp
	string/transform.h
)

ADD_LIBRARY(common ${common_sources} ${common_headers})


SET(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)
