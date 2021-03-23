CMAKE_MINIMUM_REQUIRED(VERSION 3.2)

SET(recast_navigation_sources
	DebugUtils/Source/DebugDraw.cpp
	DebugUtils/Source/DetourDebugDraw.cpp
	DebugUtils/Source/RecastDebugDraw.cpp
	DebugUtils/Source/RecastDump.cpp
	Detour/Source/DetourAlloc.cpp
	Detour/Source/DetourAssert.cpp
	Detour/Source/DetourCommon.cpp
	Detour/Source/DetourNavMesh.cpp
	Detour/Source/DetourNavMeshBuilder.cpp
	Detour/Source/DetourNavMeshQuery.cpp
	Detour/Source/DetourNode.cpp
	DetourCrowd/Source/DetourCrowd.cpp
	DetourCrowd/Source/DetourLocalBoundary.cpp
	DetourCrowd/Source/DetourObstacleAvoidance.cpp
	DetourCrowd/Source/DetourPathCorridor.cpp
	DetourCrowd/Source/DetourPathQueue.cpp
	DetourCrowd/Source/DetourProximityGrid.cpp
	DetourTileCache/Source/DetourTileCache.cpp
	DetourTileCache/Source/DetourTileCacheBuilder.cpp
	Recast/Source/Recast.cpp
	Recast/Source/RecastAlloc.cpp
	Recast/Source/RecastArea.cpp
	Recast/Source/RecastAssert.cpp
	Recast/Source/RecastContour.cpp
	Recast/Source/RecastFilter.cpp
	Recast/Source/RecastLayers.cpp
	Recast/Source/RecastMesh.cpp
	Recast/Source/RecastMeshDetail.cpp
	Recast/Source/RecastRasterization.cpp
	Recast/Source/RecastRegion.cpp
)

SET(recast_navigation_headers
	DebugUtils/Include/DebugDraw.h
	DebugUtils/Include/DetourDebugDraw.h
	DebugUtils/Include/RecastDebugDraw.h
	DebugUtils/Include/RecastDump.h
	Detour/Include/DetourAlloc.h
	Detour/Include/DetourAssert.h
	Detour/Include/DetourCommon.h
	Detour/Include/DetourMath.h
	Detour/Include/DetourNavMesh.h
	Detour/Include/DetourNavMeshBuilder.h
	Detour/Include/DetourNavMeshQuery.h
	Detour/Include/DetourNode.h
	Detour/Include/DetourStatus.h
	DetourCrowd/Include/DetourLocalBoundary.h
	DetourCrowd/Include/DetourObstacleAvoidance.h
	DetourCrowd/Include/DetourPathCorridor.h
	DetourCrowd/Include/DetourPathQueue.h
	DetourCrowd/Include/DetourProximityGrid.h
	DetourTileCache/Include/DetourTileCache.h
	DetourTileCache/Include/DetourTileCacheBuilder.h
	Recast/Include/Recast.h
	Recast/Include/RecastAlloc.h
	Recast/Include/RecastAssert.h
)

SOURCE_GROUP(DebugUtils FILES
	DebugUtils/Source/DebugDraw.cpp
	DebugUtils/Source/DetourDebugDraw.cpp
	DebugUtils/Source/RecastDebugDraw.cpp
	DebugUtils/Source/RecastDump.cpp
	DebugUtils/Include/DebugDraw.h
	DebugUtils/Include/DetourDebugDraw.h
	DebugUtils/Include/RecastDebugDraw.h
	DebugUtils/Include/RecastDump.h
)

SOURCE_GROUP(Detour FILES
	Detour/Source/DetourAlloc.cpp
	Detour/Source/DetourAssert.cpp
	Detour/Source/DetourCommon.cpp
	Detour/Source/DetourNavMesh.cpp
	Detour/Source/DetourNavMeshBuilder.cpp
	Detour/Source/DetourNavMeshQuery.cpp
	Detour/Source/DetourNode.cpp
	Detour/Include/DetourAlloc.h
	Detour/Include/DetourAssert.h
	Detour/Include/DetourCommon.h
	Detour/Include/DetourMath.h
	Detour/Include/DetourNavMesh.h
	Detour/Include/DetourNavMeshBuilder.h
	Detour/Include/DetourNavMeshQuery.h
	Detour/Include/DetourNode.h
	Detour/Include/DetourStatus.h
)

SOURCE_GROUP(DetourTileCache FILES
	DetourTileCache/Source/DetourTileCache.cpp
	DetourTileCache/Source/DetourTileCacheBuilder.cpp
	DetourTileCache/Include/DetourTileCache.h
	DetourTileCache/Include/DetourTileCacheBuilder.h
)

SOURCE_GROUP(Recast FILES
	Recast/Source/Recast.cpp
	Recast/Source/RecastAlloc.cpp
	Recast/Source/RecastArea.cpp
	Recast/Source/RecastAssert.cpp
	Recast/Source/RecastContour.cpp
	Recast/Source/RecastFilter.cpp
	Recast/Source/RecastLayers.cpp
	Recast/Source/RecastMesh.cpp
	Recast/Source/RecastMeshDetail.cpp
	Recast/Source/RecastRasterization.cpp
	Recast/Source/RecastRegion.cpp
	Recast/Include/Recast.h
	Recast/Include/RecastAlloc.h
	Recast/Include/RecastAssert.h
)

SOURCE_GROUP(DetourCrowd FILES
	DetourCrowd/Source/DetourCrowd.cpp
	DetourCrowd/Source/DetourLocalBoundary.cpp
	DetourCrowd/Source/DetourObstacleAvoidance.cpp
	DetourCrowd/Source/DetourPathCorridor.cpp
	DetourCrowd/Source/DetourPathQueue.cpp
	DetourCrowd/Source/DetourProximityGrid.cpp
	DetourCrowd/Include/DetourCrowd.h
	DetourCrowd/Include/DetourLocalBoundary.h
	DetourCrowd/Include/DetourObstacleAvoidance.h
	DetourCrowd/Include/DetourPathCorridor.h
	DetourCrowd/Include/DetourPathQueue.h
	DetourCrowd/Include/DetourProximityGrid.h
)

ADD_LIBRARY(recastnavigation ${recast_navigation_sources} ${recast_navigation_headers})

SET(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)
