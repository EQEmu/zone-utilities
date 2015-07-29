CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

SET(recast_debug_sources
	src/DebugDraw.cpp
	src/DetourDebugDraw.cpp
	src/RecastDebugDraw.cpp
	src/RecastDump.cpp
)

SET(recast_debug_headers
	include/DebugDraw.h
	include/DetourDebugDraw.h
	include/RecastDebugDraw.h
	include/RecastDump.h
)

ADD_LIBRARY(recast_debug ${recast_debug_sources} ${recast_debug_headers})

SET(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)
