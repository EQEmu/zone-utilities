#include <DetourNavMeshBuilder.h>
#include "module_navigation_build_tile.h"
#include <RecastAssert.h>

void rcMarkOrientedBoundingBoxArea(rcContext* ctx, OrientedBoundingBox &obb, 
	unsigned char areaId, rcCompactHeightfield& chf)
{
	rcAssert(ctx);

	float min_x = obb.GetMinX();
	float min_y = obb.GetMinY();
	float min_z = obb.GetMinZ();
	float max_x = obb.GetMaxX();
	float max_y = obb.GetMaxY();
	float max_z = obb.GetMaxZ();

	glm::vec4 v1(min_x, max_y, min_z, 1.0f);
	glm::vec4 v2(min_x, max_y, max_z, 1.0f);
	glm::vec4 v3(max_x, max_y, max_z, 1.0f);
	glm::vec4 v4(max_x, max_y, min_z, 1.0f);
	glm::vec4 v5(min_x, min_y, min_z, 1.0f);
	glm::vec4 v6(min_x, min_y, max_z, 1.0f);
	glm::vec4 v7(max_x, min_y, max_z, 1.0f);
	glm::vec4 v8(max_x, min_y, min_z, 1.0f);

	v1 = obb.GetTransformation() * v1;
	v2 = obb.GetTransformation() * v2;
	v3 = obb.GetTransformation() * v3;
	v4 = obb.GetTransformation() * v4;
	v5 = obb.GetTransformation() * v5;
	v6 = obb.GetTransformation() * v6;
	v7 = obb.GetTransformation() * v7;
	v8 = obb.GetTransformation() * v8;

	float bmin[3], bmax[3];
	rcVcopy(bmin, &v1[0]);
	rcVcopy(bmax, &v1[0]);

	rcVmin(bmin, &v1[0]);
	rcVmax(bmax, &v1[0]);
	rcVmin(bmin, &v2[0]);
	rcVmax(bmax, &v2[0]);
	rcVmin(bmin, &v3[0]);
	rcVmax(bmax, &v3[0]);
	rcVmin(bmin, &v4[0]);
	rcVmax(bmax, &v4[0]);
	rcVmin(bmin, &v5[0]);
	rcVmax(bmax, &v5[0]);
	rcVmin(bmin, &v6[0]);
	rcVmax(bmax, &v6[0]);
	rcVmin(bmin, &v7[0]);
	rcVmax(bmax, &v7[0]);

	int minx = (int)((bmin[0] - chf.bmin[0]) / chf.cs);
	int miny = (int)((bmin[1] - chf.bmin[1]) / chf.ch);
	int minz = (int)((bmin[2] - chf.bmin[2]) / chf.cs);
	int maxx = (int)((bmax[0] - chf.bmin[0]) / chf.cs);
	int maxy = (int)((bmax[1] - chf.bmin[1]) / chf.ch);
	int maxz = (int)((bmax[2] - chf.bmin[2]) / chf.cs);

	if (maxx < 0) return;
	if (minx >= chf.width) return;
	if (maxz < 0) return;
	if (minz >= chf.height) return;

	if (minx < 0) minx = 0;
	if (maxx >= chf.width) maxx = chf.width - 1;
	if (minz < 0) minz = 0;
	if (maxz >= chf.height) maxz = chf.height - 1;

	for (int z = minz; z <= maxz; ++z)
	{
		for (int x = minx; x <= maxx; ++x)
		{
			const rcCompactCell& c = chf.cells[x + z*chf.width];
			for (int i = (int)c.index, ni = (int)(c.index + c.count); i < ni; ++i)
			{
				rcCompactSpan& s = chf.spans[i];
				if (chf.areas[i] == RC_NULL_AREA)
					continue;
				if ((int)s.y >= miny && (int)s.y <= maxy)
				{
					float p[3];
					p[0] = chf.bmin[0] + (x + 0.5f)*chf.cs;
					p[1] = 0;
					p[2] = chf.bmin[2] + (z + 0.5f)*chf.cs;

					if (obb.ContainsPoint(glm::vec3(p[1], p[0], p[2]))) {
						chf.areas[i] = areaId;
					}
				}
			}
		}
	}
}

void ModuleNavigationBuildTile::Run() {
	if (!m_nav_module->m_scene->GetZoneGeometry() || !m_nav_module->m_chunky_mesh) {
		return;
	}
	
	const float* verts = (float*)m_nav_module->m_scene->GetZoneGeometry()->GetCollidableVerts().data();
	const int nverts = (int)m_nav_module->m_scene->GetZoneGeometry()->GetCollidableVerts().size();
	const int ntris = (int)m_nav_module->m_scene->GetZoneGeometry()->GetCollidableInds().size() / 3;
	const rcChunkyTriMesh* chunky_mesh = m_nav_module->m_chunky_mesh.get();
	
	rcConfig cfg;
	memset(&cfg, 0, sizeof(cfg));
	cfg.cs = m_nav_module->m_cell_size;
	cfg.ch = m_nav_module->m_cell_height;
	cfg.walkableSlopeAngle = m_nav_module->m_agent_max_slope;
	cfg.walkableHeight = (int)ceilf(m_nav_module->m_agent_height / cfg.ch);
	cfg.walkableClimb = (int)floorf(m_nav_module->m_agent_max_climb / cfg.ch);
	cfg.walkableRadius = (int)ceilf(m_nav_module->m_agent_radius / cfg.cs);
	cfg.maxEdgeLen = (int)(m_nav_module->m_edge_max_len / m_nav_module->m_cell_size);
	cfg.maxSimplificationError = m_nav_module->m_edge_max_error;
	cfg.minRegionArea = (int)rcSqr(m_nav_module->m_region_min_size);
	cfg.mergeRegionArea = (int)rcSqr(m_nav_module->m_region_merge_size);
	cfg.maxVertsPerPoly = (int)m_nav_module->m_verts_per_poly;
	cfg.tileSize = (int)m_nav_module->m_tile_size;
	cfg.borderSize = cfg.walkableRadius + 3;
	cfg.width = cfg.tileSize + cfg.borderSize * 2;
	cfg.height = cfg.tileSize + cfg.borderSize * 2;
	cfg.detailSampleDist = m_nav_module->m_detail_sample_dist < 0.9f ? 0 : m_nav_module->m_cell_size * m_nav_module->m_detail_sample_dist;
	cfg.detailSampleMaxError = m_nav_module->m_cell_height * m_nav_module->m_detail_sample_max_error;
	
	rcVcopy(cfg.bmin, (float*)&m_tile_min);
	rcVcopy(cfg.bmax, (float*)&m_tile_max);
	cfg.bmin[0] -= cfg.borderSize * cfg.cs;
	cfg.bmin[2] -= cfg.borderSize * cfg.cs;
	cfg.bmax[0] += cfg.borderSize * cfg.cs;
	cfg.bmax[2] += cfg.borderSize * cfg.cs;
	
	rcHeightfield *solid = rcAllocHeightfield();
	if (!rcCreateHeightfield(m_ctx.get(), *solid, cfg.width, cfg.height, cfg.bmin, cfg.bmax, cfg.cs, cfg.ch)) {
		rcFreeHeightField(solid);
		return;
	}
	
	unsigned char *triareas = new unsigned char[chunky_mesh->maxTrisPerChunk];
	
	float tbmin[2], tbmax[2];
	tbmin[0] = cfg.bmin[0];
	tbmin[1] = cfg.bmin[2];
	tbmax[0] = cfg.bmax[0];
	tbmax[1] = cfg.bmax[2];
	int cid[512];
	const int ncid = rcGetChunksOverlappingRect(chunky_mesh, tbmin, tbmax, cid, 512);
	if (!ncid) {
		rcFreeHeightField(solid);
		delete[] triareas;
		return;
	}
	
	for (int i = 0; i < ncid; ++i)
	{
		const rcChunkyTriMeshNode& node = chunky_mesh->nodes[cid[i]];
		const int* ctris = &chunky_mesh->tris[node.i * 3];
		const int nctris = node.n;
	
		memset(triareas, 0, nctris * sizeof(unsigned char));
		rcMarkWalkableTriangles(m_ctx.get(), cfg.walkableSlopeAngle,
			verts, nverts, ctris, nctris, triareas);
	
		rcRasterizeTriangles(m_ctx.get(), verts, nverts, ctris, triareas, nctris, *solid, cfg.walkableClimb);
	}
	
	delete[] triareas;
	triareas = nullptr;
	
	rcFilterLowHangingWalkableObstacles(m_ctx.get(), cfg.walkableClimb, *solid);
	rcFilterLedgeSpans(m_ctx.get(), cfg.walkableHeight, cfg.walkableClimb, *solid);
	rcFilterWalkableLowHeightSpans(m_ctx.get(), cfg.walkableHeight, *solid);
	
	rcCompactHeightfield *chf = rcAllocCompactHeightfield();
	
	if (!rcBuildCompactHeightfield(m_ctx.get(), cfg.walkableHeight, cfg.walkableClimb, *solid, *chf)) {
		rcFreeHeightField(solid);
		rcFreeCompactHeightfield(chf);
		return;
	}
	
	rcFreeHeightField(solid);
	solid = nullptr;
	
	if (!rcErodeWalkableArea(m_ctx.get(), cfg.walkableRadius, *chf))
	{
		rcFreeCompactHeightfield(chf);
		return;
	}

	//go through and mark our convex areas...
	//for (auto &volume : m_nav_module->m_volumes) {
	//	rcMarkConvexPolyArea(m_ctx.get(), volume.verts, 4, volume.min, volume.max, (unsigned char)volume.area_type, *chf);
	//}
	
	if (m_nav_module->m_partition_type == NAVIGATION_PARTITION_WATERSHED)
	{
		// Prepare for region partitioning, by calculating distance field along the walkable surface.
		if (!rcBuildDistanceField(m_ctx.get(), *chf))
		{
			rcFreeCompactHeightfield(chf);
			return;
		}
	
		// Partition the walkable surface into simple regions without holes.
		if (!rcBuildRegions(m_ctx.get(), *chf, cfg.borderSize, cfg.minRegionArea, cfg.mergeRegionArea))
		{
			rcFreeCompactHeightfield(chf);
			return;
		}
	}
	else if (m_nav_module->m_partition_type == NAVIGATION_PARTITION_MONOTONE) {
		if (!rcBuildRegionsMonotone(m_ctx.get(), *chf, cfg.borderSize, cfg.minRegionArea, cfg.mergeRegionArea))
		{
			rcFreeCompactHeightfield(chf);
			return;
		}
	}
	else {
		if (!rcBuildLayerRegions(m_ctx.get(), *chf, cfg.borderSize, cfg.minRegionArea))
		{
			rcFreeCompactHeightfield(chf);
			return;
		}
	}
	
	rcContourSet *cset = rcAllocContourSet();
	if (!rcBuildContours(m_ctx.get(), *chf, cfg.maxSimplificationError, cfg.maxEdgeLen, *cset)) {
		rcFreeCompactHeightfield(chf);
		rcFreeContourSet(cset);
		return;
	}
	
	if (cset->nconts == 0) {
		rcFreeCompactHeightfield(chf);
		rcFreeContourSet(cset);
		return;
	}
	
	rcPolyMesh *pmesh = rcAllocPolyMesh();
	if (!rcBuildPolyMesh(m_ctx.get(), *cset, cfg.maxVertsPerPoly, *pmesh)) {
		rcFreeCompactHeightfield(chf);
		rcFreeContourSet(cset);
		rcFreePolyMesh(pmesh);
		return;
	}
	
	rcPolyMeshDetail *dmesh = rcAllocPolyMeshDetail();
	if (!rcBuildPolyMeshDetail(m_ctx.get(), *pmesh, *chf,
		cfg.detailSampleDist, cfg.detailSampleMaxError,
		*dmesh))
	{
		rcFreeCompactHeightfield(chf);
		rcFreeContourSet(cset);
		rcFreePolyMesh(pmesh);
		rcFreePolyMeshDetail(dmesh);
		return;
	}
	
	rcFreeCompactHeightfield(chf);
	chf = nullptr;
	rcFreeContourSet(cset);
	cset = nullptr;
	
	if (cfg.maxVertsPerPoly <= DT_VERTS_PER_POLYGON)
	{
		if (pmesh->nverts >= 0xffff)
		{
			rcFreePolyMesh(pmesh);
			rcFreePolyMeshDetail(dmesh);
			return;
		}
	
		const int nvp = pmesh->nvp;
		const float cs = pmesh->cs;
		const float ch = pmesh->ch;
		const float* orig = pmesh->bmin;
	
		for (int i = 0; i < pmesh->npolys; ++i)
		{
			if (pmesh->areas[i] == RC_WALKABLE_AREA)
				pmesh->areas[i] = NavigationPolyFlagNormal;

			switch (pmesh->areas[i])
			{
			case NavigationAreaFlagNormal:
				pmesh->flags[i] = NavigationPolyFlagNormal;
				break;
			case NavigationAreaFlagWater:
				pmesh->flags[i] = NavigationPolyFlagWater;
				break;
			case NavigationAreaFlagLava:
				pmesh->flags[i] = NavigationPolyFlagLava;
				break;
			case NavigationAreaFlagZoneLine:
				pmesh->flags[i] = NavigationPolyFlagZoneLine;
				break;
			case NavigationAreaFlagPvP:
				pmesh->flags[i] = NavigationPolyFlagPvP;
				break;
			case NavigationAreaFlagSlime:
				pmesh->flags[i] = NavigationPolyFlagSlime;
				break;
			case NavigationAreaFlagIce:
				pmesh->flags[i] = NavigationPolyFlagIce;
				break;
			case NavigationAreaFlagVWater:
				pmesh->flags[i] = NavigationPolyFlagVWater;
				break;
			case NavigationAreaFlagGeneralArea:
				pmesh->flags[i] = NavigationPolyFlagGeneralArea;
				break;
			default:
				pmesh->flags[i] = NavigationPolyFlagDisabled;
			}
		}
	
		dtNavMeshCreateParams params;
		memset(&params, 0, sizeof(params));
		params.verts = pmesh->verts;
		params.vertCount = pmesh->nverts;
		params.polys = pmesh->polys;
		params.polyAreas = pmesh->areas;
		params.polyFlags = pmesh->flags;
		params.polyCount = pmesh->npolys;
		params.nvp = pmesh->nvp;
		params.detailMeshes = dmesh->meshes;
		params.detailVerts = dmesh->verts;
		params.detailVertsCount = dmesh->nverts;
		params.detailTris = dmesh->tris;
		params.detailTriCount = dmesh->ntris;
		params.offMeshConVerts = nullptr;
		params.offMeshConRad = nullptr;
		params.offMeshConDir = nullptr;
		params.offMeshConAreas = nullptr;
		params.offMeshConFlags = nullptr;
		params.offMeshConUserID = nullptr;
		params.offMeshConCount = 0;
		params.walkableHeight = m_nav_module->m_agent_height;
		params.walkableRadius = m_nav_module->m_agent_radius;
		params.walkableClimb = m_nav_module->m_agent_max_climb;
		params.tileX = m_x;
		params.tileY = m_y;
		params.tileLayer = 0;
		rcVcopy(params.bmin, pmesh->bmin);
		rcVcopy(params.bmax, pmesh->bmax);
		params.cs = cfg.cs;
		params.ch = cfg.ch;
		params.buildBvTree = false;
	
		m_nav_data = nullptr;
		m_nav_data_size = 0;
	
		if (!dtCreateNavMeshData(&params, &m_nav_data, &m_nav_data_size))
		{
			rcFreePolyMesh(pmesh);
			rcFreePolyMeshDetail(dmesh);
			return;
		}
	}
	
	rcFreePolyMesh(pmesh);
	rcFreePolyMeshDetail(dmesh);
}

void ModuleNavigationBuildTile::Finished() {
	if (m_nav_data) {
		m_nav_module->m_nav_mesh->removeTile(m_nav_module->m_nav_mesh->getTileRefAt(m_x, m_y, 0), 0, 0);
		dtStatus status = m_nav_module->m_nav_mesh->addTile(m_nav_data, m_nav_data_size, DT_TILE_FREE_DATA, 0, 0);
		if (dtStatusFailed(status)) {
			dtFree(m_nav_data);
			m_nav_data = nullptr;
		}
		else {
			m_nav_data = nullptr;
		}
	}
	m_nav_module->m_work_pending--;
	if(m_nav_module->m_work_pending == 0) {
		m_nav_module->CreateNavMeshModel();
	}
}
