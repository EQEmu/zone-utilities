#include "string_util.h"

#include <DetourNavMeshBuilder.h>

#include "imgui.h"
#include "module_navigation.h"
#include "thread_pool.h"
#include "log_macros.h"

inline unsigned int nextPow2(unsigned int v)
{
	v--;
	v |= v >> 1;
	v |= v >> 2;
	v |= v >> 4;
	v |= v >> 8;
	v |= v >> 16;
	v++;
	return v;
}

inline unsigned int ilog2(unsigned int v)
{
	unsigned int r;
	unsigned int shift;
	r = (v > 0xffff) << 4; v >>= r;
	shift = (v > 0xff) << 3; v >>= shift; r |= shift;
	shift = (v > 0xf) << 2; v >>= shift; r |= shift;
	shift = (v > 0x3) << 1; v >>= shift; r |= shift;
	r |= (v >> 1);
	return r;
}

ModuleNavigation::ModuleNavigation() : m_thread_pool(4) {
	m_cell_size = 0.7f;
	m_cell_height = 0.2f;
	m_agent_height = 6.0f;
	m_agent_radius = 0.9f;
	m_agent_max_climb = 6.0f;
	m_agent_max_slope = 60.0f;
	m_region_min_size = 8;
	m_region_merge_size = 20;
	m_edge_max_len = 12.0f;
	m_edge_max_error = 1.3f;
	m_verts_per_poly = 6.0f;
	m_detail_sample_dist = 6.0f;
	m_detail_sample_max_error = 1.0f;
	m_partition_type = NAVIGATION_PARTITION_WATERSHED;
	m_max_tiles = 0;
	m_max_polys_per_tile = 0;
	m_tile_size = 256;
	m_nav_mesh = nullptr;
	m_tiles_building = 0;
	m_render_nav_mesh = true;
}

ModuleNavigation::~ModuleNavigation() {
	if(m_nav_mesh) {
		dtFreeNavMesh(m_nav_mesh);
	}
}

void ModuleNavigation::OnLoad(Scene *s) {
	m_scene = s;
}

void ModuleNavigation::OnShutdown() {
	if(m_nav_mesh_renderable) {
		m_scene->UnregisterEntity(m_nav_mesh_renderable.get());
		m_nav_mesh_renderable.release();
	}
}

void ModuleNavigation::OnDrawMenu() {
	if(ImGui::BeginMenu("Navigation"))
	{
		if(ImGui::MenuItem("Clear")) {

		}
		ImGui::EndMenu();
	}
}

void ModuleNavigation::OnDrawUI() {
	m_thread_pool.Process();

	ImGui::Begin("Navigation");

	if(m_tiles_building > 0) {
		ImGui::Text(EQEmu::StringFormat("Building NavMesh: %u tiles remaining", m_tiles_building).c_str());
		ImGui::End();
		return;
	}

	ImGui::Text("Rasterization");
	ImGui::SliderFloat("Cell Size", &m_cell_size, 0.1f, 1.0f, "%.1f");
	ImGui::SliderFloat("Cell Height", &m_cell_height, 0.1f, 1.0f, "%.1f");
	
	auto zone_geo = m_scene->GetZoneGeometry();

	if(zone_geo) {
		const float* bmin = (float*)&zone_geo->GetCollidableMin();
		const float* bmax = (float*)&zone_geo->GetCollidableMax();
		int gw = 0, gh = 0;
		rcCalcGridSize(bmin, bmax, m_cell_size, &gw, &gh);
		ImGui::Text(EQEmu::StringFormat("Voxels  %d x %d", gw, gh).c_str());
	}

	ImGui::Separator();
	ImGui::Text("Agent");
	ImGui::SliderFloat("Height", &m_agent_height, 0.1f, 15.0f, "%.1f");
	ImGui::SliderFloat("Radius", &m_agent_radius, 0.1f, 15.0f, "%.1f");
	ImGui::SliderFloat("Max Climb", &m_agent_max_climb, 0.1f, 15.0f, "%.1f");
	ImGui::SliderFloat("Max Slope", &m_agent_max_slope, 0.0f, 90.0f, "%.0f");

	ImGui::Separator();
	ImGui::Text("Region");
	ImGui::SliderFloat("Min Region Size", &m_region_min_size, 0.0f, 150.0f, "%.0f");
	ImGui::SliderFloat("Merged Region Size", &m_region_merge_size, 0.0f, 150.0f, "%.0f");
	
	ImGui::Separator();
	ImGui::Text("Partitioning");
	const char *partition_types[] = { "Watershed", "Monotone", "Layers" };

	ImGui::Combo("Type", &m_partition_type, partition_types, 3);
	
	ImGui::Separator();
	ImGui::Text("Polygonization");
	ImGui::SliderFloat("Max Edge Length", &m_edge_max_len, 0.0f, 50.0f, "%.0f");
	ImGui::SliderFloat("Max Edge Error", &m_edge_max_error, 0.1f, 3.0f, "%.1f");
	ImGui::SliderFloat("Verts Per Poly", &m_verts_per_poly, 3.0f, 12.0f, "%.0f");
	
	ImGui::Separator();
	ImGui::Text("Detail Mesh");
	ImGui::SliderFloat("Sample Distance", &m_detail_sample_dist, 0.0f, 32.0f, "%.0f");
	ImGui::SliderFloat("Max Sample Error", &m_detail_sample_max_error, 0.0f, 16.0f, "%.0f");
	ImGui::Separator();

	ImGui::Text("Tiling");
	ImGui::SliderFloat("TileSize", &m_tile_size, 16.0f, 1024.0f, "%.0f");

	if(zone_geo) {
		const float* bmin = (float*)&zone_geo->GetCollidableMin();
		const float* bmax = (float*)&zone_geo->GetCollidableMax();

		int gw = 0, gh = 0;
		rcCalcGridSize(bmin, bmax, m_cell_size, &gw, &gh);

		const int ts = (int)m_tile_size;
		const int tw = (gw + ts - 1) / ts;
		const int th = (gh + ts - 1) / ts;

		ImGui::Text(EQEmu::StringFormat("Tiles  %d x %d", tw, th).c_str());
		int tile_bits = rcMin((int)ilog2(nextPow2(tw*th)), 14);

		if(tile_bits > 14)
			tile_bits = 14;
		int poly_bits = 22 - tile_bits;
		m_max_tiles = 1 << tile_bits;
		m_max_polys_per_tile = 1 << poly_bits;
		ImGui::Text(EQEmu::StringFormat("Max Tiles  %d", m_max_tiles).c_str());
		ImGui::Text(EQEmu::StringFormat("Max Polys  %d", m_max_polys_per_tile).c_str());

		if(m_nav_mesh_renderable) {
			ImGui::Text(EQEmu::StringFormat("Current Nodes: %u", (int)m_nav_mesh_renderable->GetTrianglesInds().size() / 3).c_str());
		}
		else {
			ImGui::Text(EQEmu::StringFormat("Current Nodes: %u", 0).c_str());
		}
	} else {
		m_max_tiles = 0;
		m_max_polys_per_tile = 0;
	}

	ImGui::Separator();

	ImGui::Text("Rendering");
	if(ImGui::Checkbox("Render NavMesh", &m_render_nav_mesh)) {
		if(!m_render_nav_mesh) {
			if(m_nav_mesh_renderable) {
				m_scene->UnregisterEntity(m_nav_mesh_renderable.get());
			}
		} else {
			if(m_nav_mesh_renderable) {
				m_scene->RegisterEntity(m_nav_mesh_renderable.get());
			}
		}
	}
	ImGui::Separator();

	if(ImGui::Button("Build NavMesh")) {
		BuildNavigationMesh();
	}

	ImGui::End();
}

void ModuleNavigation::OnSceneLoad(const char *zone_name) {
	auto zone_geo = m_scene->GetZoneGeometry();
	m_chunky_mesh.reset(new rcChunkyTriMesh());

	if(zone_geo) {
		if(!rcCreateChunkyTriMesh((float*)zone_geo->GetCollidableVerts().data(), (int*)zone_geo->GetCollidableInds().data(), (int)zone_geo->GetCollidableInds().size() / 3, 256, m_chunky_mesh.get())) {
			m_chunky_mesh.reset();
		}
	}

	if(m_nav_mesh) {
		dtFreeNavMesh(m_nav_mesh);
		m_nav_mesh = nullptr;

		if(m_nav_mesh_renderable) {
			m_scene->UnregisterEntity(m_nav_mesh_renderable.get());
			m_nav_mesh_renderable.release();
		}
	}
}

void ModuleNavigation::OnSuspend() {
	if(m_nav_mesh_renderable) {
		m_scene->UnregisterEntity(m_nav_mesh_renderable.get());
	}
}

void ModuleNavigation::OnResume() {
	if(m_render_nav_mesh && m_nav_mesh_renderable) {
		m_scene->RegisterEntity(m_nav_mesh_renderable.get());
	}
}

bool ModuleNavigation::HasWork() {
	return m_tiles_building > 0;
}

bool ModuleNavigation::CanSave() {
	return false;
}

void ModuleNavigation::Save() {
}

void ModuleNavigation::OnHotkey(int ident) {
}

void ModuleNavigation::BuildNavigationMesh() {
	if(m_tiles_building > 0) {
		return;
	}

	auto zone_geo = m_scene->GetZoneGeometry();
	if(!zone_geo) {
		return;
	}

	if(m_nav_mesh) {
		dtFreeNavMesh(m_nav_mesh);
		m_nav_mesh = nullptr;
	}

	m_nav_mesh = dtAllocNavMesh();

	dtNavMeshParams params;
	rcVcopy(params.orig, (float*)&zone_geo->GetCollidableMin());
	params.tileWidth = m_tile_size * m_cell_size;
	params.tileHeight = m_tile_size * m_cell_size;
	params.maxTiles = m_max_tiles;
	params.maxPolys = m_max_polys_per_tile;

	dtStatus status;
	status = m_nav_mesh->init(&params);
	if(dtStatusFailed(status)) {
		dtFreeNavMesh(m_nav_mesh);
		m_nav_mesh = nullptr;
		return;
	}


	const float* bmin = (float*)&zone_geo->GetCollidableMin();
	const float* bmax = (float*)&zone_geo->GetCollidableMax();
	int gw = 0, gh = 0;
	rcCalcGridSize(bmin, bmax, m_cell_size, &gw, &gh);
	const int ts = (int)m_tile_size;
	const int tw = (gw + ts - 1) / ts;
	const int th = (gh + ts - 1) / ts;
	const float tcs = m_tile_size * m_cell_size;

	for(int y = 0; y < th; ++y)
	{
		for(int x = 0; x < tw; ++x)
		{
			glm::vec3 tile_min(bmin[0] + x * tcs, bmin[1], bmin[2] + y * tcs);
			glm::vec3 tile_max(bmin[0] + (x + 1) * tcs, bmax[1], bmin[2] + (y + 1) * tcs);

			m_tiles_building++;
			m_thread_pool.AddWork(new ModuleNavigationBuildTile(this, x, y, tile_min, tile_max));
		}
	}
}

void ModuleNavigationBuildTile::Run() {
	if(!m_nav_module->m_scene->GetZoneGeometry() || !m_nav_module->m_chunky_mesh) {
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
	if(!rcCreateHeightfield(m_ctx.get(), *solid, cfg.width, cfg.height, cfg.bmin, cfg.bmax, cfg.cs, cfg.ch)) {
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
	if(!ncid) {
		rcFreeHeightField(solid);
		delete[] triareas;
		return;
	}

	for(int i = 0; i < ncid; ++i)
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

	if(!rcBuildCompactHeightfield(m_ctx.get(), cfg.walkableHeight, cfg.walkableClimb, *solid, *chf)) {
		rcFreeHeightField(solid);
		rcFreeCompactHeightfield(chf);
		return;
	}

	rcFreeHeightField(solid);
	solid = nullptr;

	if(!rcErodeWalkableArea(m_ctx.get(), cfg.walkableRadius, *chf))
	{
		rcFreeCompactHeightfield(chf);
		return;
	}

	if(m_nav_module->m_partition_type == NAVIGATION_PARTITION_WATERSHED)
	{
		// Prepare for region partitioning, by calculating distance field along the walkable surface.
		if(!rcBuildDistanceField(m_ctx.get(), *chf))
		{
			rcFreeCompactHeightfield(chf);
			return;
		}

		// Partition the walkable surface into simple regions without holes.
		if(!rcBuildRegions(m_ctx.get(), *chf, cfg.borderSize, cfg.minRegionArea, cfg.mergeRegionArea))
		{
			rcFreeCompactHeightfield(chf);
			return;
		}
	}
	else if(m_nav_module->m_partition_type == NAVIGATION_PARTITION_MONOTONE) {
		if(!rcBuildRegionsMonotone(m_ctx.get(), *chf, cfg.borderSize, cfg.minRegionArea, cfg.mergeRegionArea))
		{
			rcFreeCompactHeightfield(chf);
			return;
		}
	} else {
		if(!rcBuildLayerRegions(m_ctx.get(), *chf, cfg.borderSize, cfg.minRegionArea))
		{
			rcFreeCompactHeightfield(chf);
			return;
		}
	}

	rcContourSet *cset = rcAllocContourSet();
	if(!rcBuildContours(m_ctx.get(), *chf, cfg.maxSimplificationError, cfg.maxEdgeLen, *cset)) {
		rcFreeCompactHeightfield(chf);
		rcFreeContourSet(cset);
		return;
	}

	if(cset->nconts == 0) {
		rcFreeCompactHeightfield(chf);
		rcFreeContourSet(cset);
		return;
	}

	rcPolyMesh *pmesh = rcAllocPolyMesh();
	if(!rcBuildPolyMesh(m_ctx.get(), *cset, cfg.maxVertsPerPoly, *pmesh)) {
		rcFreeCompactHeightfield(chf);
		rcFreeContourSet(cset);
		rcFreePolyMesh(pmesh);
		return;
	}

	rcPolyMeshDetail *dmesh = rcAllocPolyMeshDetail();
	if(!rcBuildPolyMeshDetail(m_ctx.get(), *pmesh, *chf,
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

	if(cfg.maxVertsPerPoly <= DT_VERTS_PER_POLYGON)
	{
		if(pmesh->nverts >= 0xffff)
		{
			rcFreePolyMesh(pmesh);
			rcFreePolyMeshDetail(dmesh);
			return;
		}

		// Update poly flags from areas.
		for(int i = 0; i < pmesh->npolys; ++i)
		{
			//if(pmesh->areas[i] == RC_WALKABLE_AREA)
			//	m_pmesh->areas[i] = SAMPLE_POLYAREA_GROUND;
			//
			//if(m_pmesh->areas[i] == SAMPLE_POLYAREA_GROUND ||
			//   m_pmesh->areas[i] == SAMPLE_POLYAREA_GRASS ||
			//   m_pmesh->areas[i] == SAMPLE_POLYAREA_ROAD)
			//{
			//	m_pmesh->flags[i] = SAMPLE_POLYFLAGS_WALK;
			//}
			//else if(m_pmesh->areas[i] == SAMPLE_POLYAREA_WATER)
			//{
			//	m_pmesh->flags[i] = SAMPLE_POLYFLAGS_SWIM;
			//}
			//else if(m_pmesh->areas[i] == SAMPLE_POLYAREA_DOOR)
			//{
			//	m_pmesh->flags[i] = SAMPLE_POLYFLAGS_WALK | SAMPLE_POLYFLAGS_DOOR;
			//}
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
		params.buildBvTree = true;

		m_nav_data = nullptr;
		m_nav_data_size = 0;

		if(!dtCreateNavMeshData(&params, &m_nav_data, &m_nav_data_size))
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
	if(m_nav_data) {
		m_nav_module->m_nav_mesh->removeTile(m_nav_module->m_nav_mesh->getTileRefAt(m_x, m_y, 0), 0, 0);
		dtStatus status = m_nav_module->m_nav_mesh->addTile(m_nav_data, m_nav_data_size, DT_TILE_FREE_DATA, 0, 0);
		if(dtStatusFailed(status)) {
			dtFree(m_nav_data);
			m_nav_data = nullptr;
		} else {
			m_nav_data = nullptr;
		}
	}
	m_nav_module->m_tiles_building--;

	if(m_nav_module->m_tiles_building == 0) {
		m_nav_module->CreateNavMeshModel();
	}
}

void ModuleNavigation::CreateNavMeshModel() {
	if(m_nav_mesh_renderable) {
		m_scene->UnregisterEntity(m_nav_mesh_renderable.get());
	}

	m_nav_mesh_renderable.reset(new NavMeshModel());
	NavigationDebugDraw dd;
	dd.nav_module = this;

	duDebugDrawNavMesh(&dd, *m_nav_mesh, 0xffu);
	m_nav_mesh_renderable->SetTrianglesTint(glm::vec4(0.75f, 1.0f, 0.25f, 1.0f));
	m_nav_mesh_renderable->SetLinesTint(glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
	m_nav_mesh_renderable->SetPointsTint(glm::vec4(1.0f, 1.0f, 0.0f, 0.5f));
	m_nav_mesh_renderable->Compile();

	if(m_render_nav_mesh) {
		m_scene->RegisterEntity(m_nav_mesh_renderable.get());
	}
}

void NavigationDebugDraw::begin(duDebugDrawPrimitives prim, float size) {
	verts_in_use = 0;
	switch(prim)
	{
	case DU_DRAW_POINTS:
		mode = 1;
		break;
	case DU_DRAW_LINES:
		mode = 2;
		break;
	case DU_DRAW_TRIS:
		mode = 3;
		break;
	case DU_DRAW_QUADS:
		mode = 4;
		break;
	};
}

void NavigationDebugDraw::vertex(const float* pos, unsigned int color) {
	verts[verts_in_use++] = glm::vec3(pos[0], pos[1], pos[2]);
	CreatePrimitive();
}

void NavigationDebugDraw::vertex(const float x, const float y, const float z, unsigned int color) {
	verts[verts_in_use++] = glm::vec3(x, y, z);
	CreatePrimitive();
}

void NavigationDebugDraw::vertex(const float* pos, unsigned int color, const float* uv) {
	verts[verts_in_use++] = glm::vec3(pos[0], pos[1], pos[2]);
	CreatePrimitive();
}

void NavigationDebugDraw::vertex(const float x, const float y, const float z, unsigned int color, const float u, const float v) {
	verts[verts_in_use++] = glm::vec3(x, y, z);
	CreatePrimitive();
}

void NavigationDebugDraw::CreatePrimitive() {
	if(mode == 0 || verts_in_use != mode) {
		return;
	}

	switch(mode) {
	case 1:
	{
		unsigned int index = (unsigned int)nav_module->m_nav_mesh_renderable->GetPointsVerts().size();
		nav_module->m_nav_mesh_renderable->GetPointsVerts().push_back(verts[0]);
		nav_module->m_nav_mesh_renderable->GetPointsInds().push_back(index);
	}
		break;
	case 2:
	{
		unsigned int index = (unsigned int)nav_module->m_nav_mesh_renderable->GetLinesVerts().size();
		nav_module->m_nav_mesh_renderable->GetLinesVerts().push_back(verts[0]);
		nav_module->m_nav_mesh_renderable->GetLinesVerts().push_back(verts[1]);
		nav_module->m_nav_mesh_renderable->GetLinesInds().push_back(index);
		nav_module->m_nav_mesh_renderable->GetLinesInds().push_back(index + 1);
	}
		break;
	case 3:
	{
		unsigned int index = (unsigned int)nav_module->m_nav_mesh_renderable->GetTrianglesVerts().size();
		nav_module->m_nav_mesh_renderable->GetTrianglesVerts().push_back(verts[0]);
		nav_module->m_nav_mesh_renderable->GetTrianglesVerts().push_back(verts[1]);
		nav_module->m_nav_mesh_renderable->GetTrianglesVerts().push_back(verts[2]);
		nav_module->m_nav_mesh_renderable->GetTrianglesInds().push_back(index);
		nav_module->m_nav_mesh_renderable->GetTrianglesInds().push_back(index + 1);
		nav_module->m_nav_mesh_renderable->GetTrianglesInds().push_back(index + 2);
	}
		break; //2 3 0
	case 4:
	{
		unsigned int index = (unsigned int)nav_module->m_nav_mesh_renderable->GetTrianglesInds().size();
		nav_module->m_nav_mesh_renderable->GetTrianglesVerts().push_back(verts[0]);
		nav_module->m_nav_mesh_renderable->GetTrianglesVerts().push_back(verts[1]);
		nav_module->m_nav_mesh_renderable->GetTrianglesVerts().push_back(verts[2]);
		nav_module->m_nav_mesh_renderable->GetTrianglesVerts().push_back(verts[3]);
		nav_module->m_nav_mesh_renderable->GetTrianglesInds().push_back(index);
		nav_module->m_nav_mesh_renderable->GetTrianglesInds().push_back(index + 1);
		nav_module->m_nav_mesh_renderable->GetTrianglesInds().push_back(index + 2);
		nav_module->m_nav_mesh_renderable->GetTrianglesInds().push_back(index + 2);
		nav_module->m_nav_mesh_renderable->GetTrianglesInds().push_back(index + 3);
		nav_module->m_nav_mesh_renderable->GetTrianglesInds().push_back(index);
	}
		break;
	}

	verts_in_use = 0;
}
