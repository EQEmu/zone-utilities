CMAKE_MINIMUM_REQUIRED(VERSION 3.10.2)

SET(common_sources
	core/compression.cpp
	core/string_util.cpp
	core/config.cpp
	eq/eq_physics.cpp
	eq/pfs.cpp
	eq/pfs_crc.cpp
	eq/water_map.cpp
	eq/water_map_v1.cpp
	eq/water_map_v2.cpp
	eq/zone_map.cpp
	event/event_loop.cpp
	graphics/eq_math.cpp
	graphics/eqg_loader.cpp
	graphics/eqg_model_loader.cpp
	graphics/eqg_v4_loader.cpp
	graphics/oriented_bounding_box.cpp
	graphics/s3d_loader.cpp
	graphics/wld_fragment.cpp
	log/composite_logger.cpp
	log/console_logger.cpp
	log/file_logger.cpp
)

SET(common_headers
	core/any.h
	core/config.h
	core/config_interface.h
	core/compression.h
	core/safe_alloc.h
	core/string_util.h
	dependency/container.h
	eq/eq_physics.h
	eq/eqemu_endian.h
	eq/pfs.h
	eq/pfs_crc.h
	eq/water_map.h
	eq/water_map_v1.h
	eq/water_map_v2.h
	eq/zone_map.h
	event/background_task.h
	event/event_loop.h
	event/timer.h
	graphics/aligned_bounding_box.h
	graphics/eq_math.h
	graphics/eqg_geometry.h
	graphics/eqg_invis_wall.h
	graphics/eqg_loader.h
	graphics/eqg_material.h
	graphics/eqg_model_loader.h
	graphics/eqg_region.h
	graphics/eqg_structs.h
	graphics/eqg_terrain.h
	graphics/eqg_terrain_tile.h
	graphics/eqg_v4_loader.h
	graphics/eqg_water_sheet.h
	graphics/light.h
	graphics/octree.h
	graphics/oriented_bounding_box.h
	graphics/placeable.h
	graphics/placeable_group.h
	graphics/s3d_bsp.h
	graphics/s3d_geometry.h
	graphics/s3d_loader.h
	graphics/s3d_skeleton_track.h
	graphics/s3d_texture.h
	graphics/s3d_texture_brush.h
	graphics/s3d_texture_brush_set.h
	graphics/wld_fragment_reference.h
	graphics/wld_fragment.h
	graphics/wld_structs.h
	log/composite_logger.h
	log/console_logger.h
	log/file_logger.h
	log/logger_interface.h
)

SOURCE_GROUP(core FILES 
	core/any.h
	core/config.h
	core/config_interface.h
	core/compression.h
	core/safe_alloc.h
	core/string_util.h
)

SOURCE_GROUP(dependency FILES 
	dependency/container.h
)

SOURCE_GROUP(eq FILES
	eq/eq_physics.cpp
	eq/pfs.cpp
	eq/pfs_crc.cpp
	eq/water_map.cpp
	eq/water_map_v1.cpp
	eq/water_map_v2.cpp
	eq/zone_map.cpp
	eq/eq_physics.h
	eq/eqemu_endian.h
	eq/pfs.h
	eq/pfs_crc.h
	eq/water_map.h
	eq/water_map_v1.h
	eq/water_map_v2.h
	eq/zone_map.h
)

SOURCE_GROUP(event FILES 
	event/background_task.h
	event/event_loop.cpp
	event/event_loop.h
	event/timer.h
)

SOURCE_GROUP(graphics FILES
	graphics/eq_math.cpp
	graphics/eqg_loader.cpp
	graphics/eqg_model_loader.cpp
	graphics/eqg_v4_loader.cpp
	graphics/oriented_bounding_box.cpp
	graphics/s3d_loader.cpp
	graphics/aligned_bounding_box.h
	graphics/eq_math.h
	graphics/eqg_geometry.h
	graphics/eqg_invis_wall.h
	graphics/eqg_loader.h
	graphics/eqg_material.h
	graphics/eqg_model_loader.h
	graphics/eqg_region.h
	graphics/eqg_structs.h
	graphics/eqg_terrain.h
	graphics/eqg_terrain_tile.h
	graphics/eqg_v4_loader.h
	graphics/eqg_water_sheet.h
	graphics/light.h
	graphics/octree.h
	graphics/oriented_bounding_box.h
	graphics/placeable.h
	graphics/placeable_group.h
	graphics/s3d_bsp.h
	graphics/s3d_geometry.h
	graphics/s3d_loader.h
	graphics/s3d_skeleton_track.h
	graphics/s3d_texture.h
	graphics/s3d_texture_brush.h
	graphics/s3d_texture_brush_set.h
	graphics/wld_fragment_reference.h
	graphics/wld_fragment.h
	graphics/wld_structs.h
)

SOURCE_GROUP(log FILES 
	log/composite_logger.cpp
	log/composite_logger.h
	log/console_logger.cpp
	log/console_logger.h
	log/file_logger.cpp
	log/file_logger.h
	log/logger_interface.h
)

ADD_LIBRARY(common ${common_sources} ${common_headers})


SET(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)
