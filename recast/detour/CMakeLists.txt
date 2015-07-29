CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

SET(detour_sources
	src/DetourAlloc.cpp
	src/DetourCommon.cpp
	src/DetourNavMesh.cpp
	src/DetourNavMeshBuilder.cpp
	src/DetourNavMeshQuery.cpp
	src/DetourNode.cpp
)

SET(detour_headers
	include/DetourAlloc.h
	include/DetourAssert.h
	include/DetourCommon.h
	include/DetourMath.h
	include/DetourNavMesh.h
	include/DetourNavMeshBuilder.h
	include/DetourNavMeshQuery.h
	include/DetourNode.h
	include/DetourStatus.h
)

ADD_LIBRARY(detour ${detour_sources} ${detour_headers})

SET(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)
