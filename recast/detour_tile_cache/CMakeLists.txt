CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

SET(detour_tile_cache_sources
	src/DetourTileCache.cpp
	src/DetourTileCacheBuilder.cpp
)

SET(detour_tile_cache_headers
	include/DetourTileCache.h
	include/DetourTileCacheBuilder.h
)

ADD_LIBRARY(detour_tile_cache ${detour_tile_cache_sources} ${detour_tile_cache_headers})

SET(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)
