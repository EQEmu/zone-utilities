CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

SET(recast_sources
	src/Recast.cpp
	src/RecastAlloc.cpp
	src/RecastArea.cpp
	src/RecastContour.cpp
	src/RecastFilter.cpp
	src/RecastLayers.cpp
	src/RecastMesh.cpp
	src/RecastMeshDetail.cpp
	src/RecastRasterization.cpp
	src/RecastRegion.cpp
)

SET(recast_headers
	include/Recast.h
	include/RecastAlloc.h
	include/RecastAssert.h
)

ADD_LIBRARY(recast ${recast_sources} ${recast_headers})

SET(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)
