CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

SET(common_sources
	compression.cpp
	eqg_loader.cpp
	eqg_v4_loader.cpp
	pfs.cpp
	s3d_loader.cpp
	wld_fragment.cpp
)

SET(common_headers
	any.h
	archive.h
	bsp.h
	compression.h
	geometry.h
	endian.h
	eqg_geometry.h
	eqg_loader.h
	eqg_region.h
	eqg_structs.h
	eqg_v4_loader.h
	light.h
	material.h
	pfs.h
	placeable.h
	s3d_loader.h
	texture.h
	texture_brush.h
	texture_brush_set.h
	wld_fragment.h
	wld_structs.h
)

ADD_LIBRARY(common ${common_sources} ${common_headers})


SET(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)
