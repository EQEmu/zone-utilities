#include <algorithm>
#include "string_util.h"
#include "imgui.h"
#include "module_navigation.h"
#include "thread_pool.h"
#include "log_macros.h"

#include "module_navigation_build_tile.h"

#include <DetourNavMeshQuery.h>

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

void calcChunkSize(const float* bmin, const float* bmax, float cs, int* x, int* y, int *z)
{
	*x = (int)((bmax[0] - bmin[0]) / cs + 0.5f);
	*y = (int)((bmax[1] - bmin[1]) / cs + 0.5f);
	*z = (int)((bmax[2] - bmin[2]) / cs + 0.5f);
}

ModuleNavigation::ModuleNavigation() : m_thread_pool(4)
{
	m_mode = 0;

	m_cell_size = 0.2f;
	m_cell_height = 0.1f;
	m_agent_height = 3.5f;
	m_agent_radius = 0.7f;
	m_agent_max_climb = 3.5f;
	m_agent_max_slope = 55.0f;
	m_region_min_size = 8;
	m_region_merge_size = 20;
	m_edge_max_len = 12.0f;
	m_edge_max_error = 1.3f;
	m_verts_per_poly = 6.0f;
	m_detail_sample_dist = 18.0f;
	m_detail_sample_max_error = 1.0f;
	m_partition_type = NAVIGATION_PARTITION_WATERSHED;
	m_max_tiles = 0;
	m_max_polys_per_tile = 0;
	m_tile_size = 256;

	m_tiles_building = 0;
	m_nav_mesh = nullptr;

	m_nav_mesh_renderable.reset(new DebugDraw(false));

	m_debug_renderable.reset(new DebugDraw());
	m_debug_renderable->SetLinesTint(glm::vec4(1.0f, 1.0f, 0.0f, 1.0f));
	m_debug_renderable->SetPointsTint(glm::vec4(0.0f, 1.0f, 1.0f, 1.0f));
	m_debug_renderable->SetTrianglesTint(glm::vec4(1.0f, 0.0f, 1.0f, 1.0f));

	m_start_path_renderable.reset(new LineModel());
	m_start_path_renderable->SetTint(glm::vec4(1.0f, 0.0f, 1.0f, 1.0f));
	m_start_path_renderable->SetDepthTestEnabled(false);
	m_start_path_renderable->SetWidth(3.0f);

	m_end_path_renderable.reset(new LineModel());
	m_end_path_renderable->SetTint(glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));
	m_end_path_renderable->SetDepthTestEnabled(false);
	m_end_path_renderable->SetWidth(3.0f);

	m_path_renderable.reset(new LineModel());
	m_path_renderable->SetTint(glm::vec4(1.0f, 1.0f, 0.0f, 0.859375f));
	m_path_renderable->SetDepthTestEnabled(false);
	m_path_renderable->SetWidth(4.0f);

	m_path_start_set = false;
	m_path_end_set = false;

	for (int i = 0; i < NavigationAreaFlagDisabled; ++i) {
		m_path_costs[i] = 1.0f;
	}
}

ModuleNavigation::~ModuleNavigation()
{
	if (m_nav_mesh) {
		dtFreeNavMesh(m_nav_mesh);
	}

	m_scene->UnregisterEntitiesByModule(this);
}

void ModuleNavigation::OnLoad(Scene *s)
{
	m_scene = s;
}

void ModuleNavigation::OnShutdown()
{
	m_scene->UnregisterEntitiesByModule(this);
}

void ModuleNavigation::OnDrawMenu()
{
	if (ImGui::BeginMenu("Navigation"))
	{
		if (ImGui::MenuItem("Clear")) {

		}
		ImGui::EndMenu();
	}
}

void ModuleNavigation::OnDrawUI()
{
	auto zone_geo = m_scene->GetZoneGeometry();
	if (!zone_geo) {
		return;
	}

	const float* bmin = (float*)&zone_geo->GetCollidableMin();
	const float* bmax = (float*)&zone_geo->GetCollidableMax();
	
	ImGui::Begin("Navigation");	

	ImGui::Text("Tools");
	ImGui::RadioButton("NavMesh Generation", &m_mode, (int)ModeNavMeshGen);
	if (m_nav_mesh && m_tiles_building == 0) {
		ImGui::RadioButton("Test Mesh", &m_mode, (int)ModeTestNavigation);
	}
	else {
		ImGui::RadioButton("Test Mesh", false);
	}
	ImGui::End();

	if (m_mode == ModeNavMeshGen) {
		DrawNavMeshGenerationUI();
	}

	if (m_mode == ModeTestNavigation) {
		DrawTestUI();
	}
}

void ModuleNavigation::OnSceneLoad(const char *zone_name)
{
	auto zone_geo = m_scene->GetZoneGeometry();
	if (zone_geo) {
		auto &bmin = zone_geo->GetCollidableMin();
		auto &bmax = zone_geo->GetCollidableMax();

		m_bounding_box_min.x = bmin.x;
		m_bounding_box_min.y = bmin.y;
		m_bounding_box_min.z = bmin.z;
		m_bounding_box_max.x = bmax.x;
		m_bounding_box_max.y = bmax.y;
		m_bounding_box_max.z = bmax.z;
		UpdateBoundingBox();
	}

	m_start_path_renderable->Clear();
	m_end_path_renderable->Clear();
	m_path_renderable->Clear();
	m_start_path_renderable->Update();
	m_end_path_renderable->Update();
	m_path_renderable->Update();
	m_path_start_set = false;
	m_path_end_set = false;
	m_nav_mesh_renderable->Clear();
	m_nav_mesh_renderable->Update();
	InitVolumes();
}

void ModuleNavigation::OnSuspend()
{
	m_scene->UnregisterEntitiesByModule(this);
}

void ModuleNavigation::OnResume()
{
	if (m_bounding_box_renderable) {
		m_scene->RegisterEntity(this, m_bounding_box_renderable.get());
	}

	if (m_nav_mesh_renderable) {
		m_scene->RegisterEntity(this, m_nav_mesh_renderable.get());
	}

	if (m_debug_renderable) {
		m_scene->RegisterEntity(this, m_debug_renderable.get());
	}

	if (m_start_path_renderable) {
		m_scene->RegisterEntity(this, m_start_path_renderable.get());
	}

	if (m_end_path_renderable) {
		m_scene->RegisterEntity(this, m_end_path_renderable.get());
	}

	if (m_path_renderable) {
		m_scene->RegisterEntity(this, m_path_renderable.get());
	}
}

bool ModuleNavigation::HasWork()
{
	if (m_tiles_building > 0) {
		return true;
	}

	return false;
}

bool ModuleNavigation::CanSave()
{
	return true;
}

void ModuleNavigation::Save()
{
}

void ModuleNavigation::OnHotkey(int ident)
{
}

void ModuleNavigation::OnClick(int mouse_button, const glm::vec3 *collide_hit, const glm::vec3 *non_collide_hit)
{
	auto &io = ImGui::GetIO();
	if (m_mode == ModeTestNavigation && mouse_button == GLFW_MOUSE_BUTTON_LEFT && !io.KeyShift && collide_hit) {
		SetNavigationTestNodeStart(*collide_hit);
		CalcPath();
	}
	else if (m_mode == ModeTestNavigation && mouse_button == GLFW_MOUSE_BUTTON_LEFT && io.KeyShift && collide_hit) {
		SetNavigationTestNodeEnd(*collide_hit);
		CalcPath();
	}
}

void ModuleNavigation::UpdateBoundingBox()
{
	if (!m_bounding_box_renderable)
		m_bounding_box_renderable.reset(new LineModel());

	m_bounding_box_renderable->Clear();
	m_bounding_box_renderable->AddBox(m_bounding_box_min, m_bounding_box_max);
	m_bounding_box_renderable->Update();
}

void ModuleNavigation::DrawNavMeshGenerationUI()
{
	m_thread_pool.Process();
	
	auto zone_geo = m_scene->GetZoneGeometry();
	const float* bmin = (float*)&m_bounding_box_min;
	const float* bmax = (float*)&m_bounding_box_max;
	int gw = 0, gh = 0;
	if (!zone_geo) {
		return;
	}

	ImGui::Begin("NavMesh Properties");

	ImGui::Text("Bounding Box");
	bool update_bb = false;
	if (ImGui::SliderFloat("Min x", &m_bounding_box_min.x, bmin[0], bmax[0], "%.1f")) {
		update_bb = true;
	}

	if (ImGui::SliderFloat("Min y", &m_bounding_box_min.y, bmin[1], bmax[1], "%.1f")) {
		update_bb = true;
	}

	if (ImGui::SliderFloat("Min z", &m_bounding_box_min.z, bmin[2], bmax[2], "%.1f")) {
		update_bb = true;
	}

	if (ImGui::SliderFloat("Max x", &m_bounding_box_max.x, bmin[0], bmax[0], "%.1f")) {
		update_bb = true;
	}

	if (ImGui::SliderFloat("Max y", &m_bounding_box_max.y, bmin[1], bmax[1], "%.1f")) {
		update_bb = true;
	}

	if (ImGui::SliderFloat("Max z", &m_bounding_box_max.z, bmin[2], bmax[2], "%.1f")) {
		update_bb = true;
	}

	if (update_bb) {
		UpdateBoundingBox();
	}

	ImGui::Separator();

	ImGui::Text("Rasterization");
	ImGui::SliderFloat("Cell Size", &m_cell_size, 0.1f, 1.0f, "%.1f");
	ImGui::SliderFloat("Cell Height", &m_cell_height, 0.1f, 1.0f, "%.1f");

	rcCalcGridSize((float*)&m_bounding_box_min, (float*)&m_bounding_box_max, m_cell_size, &gw, &gh);
	ImGui::Text(EQEmu::StringFormat("Voxels  %d x %d", gw, gh).c_str());

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

	const int ts = (int)m_tile_size;
	const int tw = (gw + ts - 1) / ts;
	const int th = (gh + ts - 1) / ts;
	
	ImGui::Text(EQEmu::StringFormat("Tiles  %d x %d", tw, th).c_str());
	int tile_bits = rcMin((int)ilog2(nextPow2(tw*th)), 14);
	
	if (tile_bits > 14)
		tile_bits = 14;
	int poly_bits = 22 - tile_bits;
	m_max_tiles = 1 << tile_bits;
	m_max_polys_per_tile = 1 << poly_bits;
	ImGui::Text(EQEmu::StringFormat("Max Tiles  %d", m_max_tiles).c_str());
	ImGui::Text(EQEmu::StringFormat("Max Polys  %d", m_max_polys_per_tile).c_str());

	if (m_nav_mesh_renderable) {
		ImGui::Text(EQEmu::StringFormat("Current Nodes: %u", (int)m_nav_mesh_renderable->GetTrianglesInds().size() / 3).c_str());
	}
	else {
		ImGui::Text(EQEmu::StringFormat("Current Nodes: %u", 0).c_str());
	}

	if (m_tiles_building > 0) {
		ImGui::Text(EQEmu::StringFormat("Building NavMesh... %u tiles remaining.", m_tiles_building).c_str());
	}
	else {
		if (ImGui::Button("Build NavMesh")) {
			BuildNavigationMesh();
		}
	}

	ImGui::End();
}

void ModuleNavigation::DrawTestUI()
{
	ImGui::Begin("NavMesh Properties");
	ImGui::Text("LMB to place start. Shift + LMB to place end.");
	ImGui::Separator();

	ImGui::Text("Area Costs");
	ImGui::SliderFloat("Normal", &m_path_costs[NavigationAreaFlagNormal], 1.0f, 100.0f, "%.1f");
	ImGui::SliderFloat("Water", &m_path_costs[NavigationAreaFlagWater], 1.0f, 100.0f, "%.1f");
	ImGui::SliderFloat("Lava", &m_path_costs[NavigationAreaFlagLava], 1.0f, 100.0f, "%.1f");
	ImGui::SliderFloat("PvP", &m_path_costs[NavigationAreaFlagPvP], 1.0f, 100.0f, "%.1f");
	ImGui::SliderFloat("Slime", &m_path_costs[NavigationAreaFlagSlime], 1.0f, 100.0f, "%.1f");
	ImGui::SliderFloat("Ice", &m_path_costs[NavigationAreaFlagIce], 1.0f, 100.0f, "%.1f");
	ImGui::SliderFloat("V Water", &m_path_costs[NavigationAreaFlagVWater], 1.0f, 100.0f, "%.1f");
	ImGui::SliderFloat("General Area", &m_path_costs[NavigationAreaFlagGeneralArea], 0.1f, 100.0f, "%.1f");
	ImGui::End();
}

void ModuleNavigation::BuildNavigationMesh()
{
	if (HasWork()) {
		return;
	}

	auto zone_geo = m_scene->GetZoneGeometry();
	if (!zone_geo) {
		return;
	}

	std::shared_ptr<EQPhysics> phys = m_scene->GetZonePhysics();
	if (!phys) {
		return;
	}

	m_nav_mesh_renderable->Clear();
	m_nav_mesh_renderable->Update();

	CreateChunkyTriMesh(zone_geo);

	if (m_nav_mesh) {
		dtFreeNavMesh(m_nav_mesh);
	}

	m_nav_mesh = dtAllocNavMesh();

	dtNavMeshParams params;
	rcVcopy(params.orig, (float*)&m_bounding_box_min);
	params.tileWidth = m_tile_size * m_cell_size;
	params.tileHeight = m_tile_size * m_cell_size;
	params.maxTiles = m_max_tiles;
	params.maxPolys = m_max_polys_per_tile;

	dtStatus status;
	status = m_nav_mesh->init(&params);
	if (dtStatusFailed(status)) {
		dtFreeNavMesh(m_nav_mesh);
		m_nav_mesh = nullptr;
		return;
	}

	const float* bmin = (float*)&m_bounding_box_min;
	const float* bmax = (float*)&m_bounding_box_max;
	int gw = 0, gh = 0;
	rcCalcGridSize(bmin, bmax, m_cell_size, &gw, &gh);
	const int ts = (int)m_tile_size;
	const int tw = (gw + ts - 1) / ts;
	const int th = (gh + ts - 1) / ts;
	const float tcs = m_tile_size * m_cell_size;

	for (int y = 0; y < th; ++y)
	{
		for (int x = 0; x < tw; ++x)
		{
			glm::vec3 tile_min(bmin[0] + x * tcs, bmin[1], bmin[2] + y * tcs);
			glm::vec3 tile_max(bmin[0] + (x + 1) * tcs, bmax[1], bmin[2] + (y + 1) * tcs);

			m_tiles_building++;
			m_thread_pool.AddWork(new ModuleNavigationBuildTile(this, x, y, tile_min, tile_max, phys));
		}
	}
}

bool VertexWithinBounds(const glm::vec3 &v, glm::vec3 &min, glm::vec3 &max) {
	if (v.x < min.x || v.x > max.x)
		return false;

	if (v.y < min.y || v.y > max.y)
		return false;

	if (v.z < min.z || v.z > max.z)
		return false;

	return true;
}

void ModuleNavigation::CreateChunkyTriMesh(std::shared_ptr<ZoneMap> zone_geo)
{

	//only include tris within bb of zone
	std::vector<int> inds;
	inds.reserve(zone_geo->GetCollidableInds().size());

	size_t sz = zone_geo->GetCollidableInds().size();
	size_t tris_out = 0;

	for (size_t i = 0; i < sz; i += 3) {
		auto i1 = zone_geo->GetCollidableInds()[i];
		auto i2 = zone_geo->GetCollidableInds()[i + 1];
		auto i3 = zone_geo->GetCollidableInds()[i + 2];
		auto &v1 = zone_geo->GetCollidableVerts()[i1];
		auto &v2 = zone_geo->GetCollidableVerts()[i2];
		auto &v3 = zone_geo->GetCollidableVerts()[i3];

		if (!VertexWithinBounds(v1, m_bounding_box_min, m_bounding_box_max)) {
			continue;
		}

		if (!VertexWithinBounds(v2, m_bounding_box_min, m_bounding_box_max)) {
			continue;
		}

		if (!VertexWithinBounds(v3, m_bounding_box_min, m_bounding_box_max)) {
			continue;
		}

		inds.push_back(i1);
		inds.push_back(i2);
		inds.push_back(i3);
	}

	m_chunky_mesh.reset(new rcChunkyTriMesh());
	if (!rcCreateChunkyTriMesh(
		(float*)zone_geo->GetCollidableVerts().data(),
		(int*)inds.data(),
		(int)inds.size() / 3,
		512,
		m_chunky_mesh.get()))
	{
		m_chunky_mesh.reset();
	}
}

void ModuleNavigation::CreateNavMeshModel()
{
	NavigationDebugDraw dd;
	dd.model = m_nav_mesh_renderable.get();
	duDebugDrawNavMesh(&dd, *m_nav_mesh, 0xffu);
	m_nav_mesh_renderable->Update();
	m_nav_mesh_renderable->SetTrianglesTint(glm::vec4(0.0f, 0.75f, 1.0f, 0.25f));
	m_nav_mesh_renderable->SetLinesTint(glm::vec4(0.0f, 0.2f, 0.25f, 0.8f));
	m_nav_mesh_renderable->SetPointsTint(glm::vec4(0.0f, 0.0f, 0.75f, 0.5f));
}

void ModuleNavigation::SetNavigationTestNodeStart(const glm::vec3 &p)
{
	m_path_start = p;
	m_path_start_set = true;
	m_start_path_renderable->Clear();
	float box_size = 2.0f;
	m_start_path_renderable->AddBox(glm::vec3(m_path_start.x - box_size, m_path_start.y - box_size, m_path_start.z - box_size), 
		glm::vec3(m_path_start.x + box_size, m_path_start.y + box_size, m_path_start.z + box_size));
	m_start_path_renderable->Update();
}

void ModuleNavigation::SetNavigationTestNodeEnd(const glm::vec3 &p)
{
	m_path_end = p;
	m_path_end_set = true;
	m_end_path_renderable->Clear();
	float box_size = 2.0f;
	m_end_path_renderable->AddBox(glm::vec3(m_path_end.x - box_size, m_path_end.y - box_size, m_path_end.z - box_size),
		glm::vec3(m_path_end.x + box_size, m_path_end.y + box_size, m_path_end.z + box_size));
	m_end_path_renderable->Update();
}

void ModuleNavigation::CalcPath()
{
	if (!m_path_start_set || !m_path_end_set || !m_nav_mesh) {
		return;
	}

	glm::vec3 ext(2.0f, 3.0f, 2.0f);
	dtQueryFilter filter;
	filter.setIncludeFlags(NavigationPolyFlagAll ^ NavigationPolyFlagDisabled);
	filter.setAreaCost(NavigationAreaFlagNormal, m_path_costs[NavigationAreaFlagNormal]);
	filter.setAreaCost(NavigationAreaFlagWater, m_path_costs[NavigationAreaFlagWater]);
	filter.setAreaCost(NavigationAreaFlagLava, m_path_costs[NavigationAreaFlagLava]);
	filter.setAreaCost(NavigationAreaFlagPvP, m_path_costs[NavigationAreaFlagPvP]);
	filter.setAreaCost(NavigationAreaFlagSlime, m_path_costs[NavigationAreaFlagSlime]);
	filter.setAreaCost(NavigationAreaFlagIce, m_path_costs[NavigationAreaFlagIce]);
	filter.setAreaCost(NavigationAreaFlagVWater, m_path_costs[NavigationAreaFlagVWater]);
	filter.setAreaCost(NavigationAreaFlagGeneralArea, m_path_costs[NavigationAreaFlagGeneralArea]);

	dtNavMeshQuery *query = dtAllocNavMeshQuery();
	query->init(m_nav_mesh, 4092);
	dtPolyRef start_ref;
	dtPolyRef end_ref;

	query->findNearestPoly(&m_path_start[0], &ext[0], &filter, &start_ref, 0);
	query->findNearestPoly(&m_path_end[0], &ext[0], &filter, &end_ref, 0);

	if (!start_ref || !end_ref) {
		m_path_renderable->Clear();
		m_path_renderable->Update();
		dtFreeNavMeshQuery(query);
		return;
	}

	int npoly = 0;
	dtPolyRef path[256] = { 0 };
	query->findPath(start_ref, end_ref, &m_path_start[0], &m_path_end[0], &filter, path, &npoly, 256);

	if (npoly) {
		glm::vec3 epos = m_path_end;
		if (path[npoly - 1] != end_ref)
			query->closestPointOnPoly(path[npoly - 1], &m_path_end[0], &epos[0], 0);

		float straight_path[256 * 3];
		unsigned char straight_path_flags[256];
		int n_straight_polys;
		dtPolyRef straight_path_polys[256];
		query->findStraightPath(&m_path_start[0], &epos[0], path, npoly,
			straight_path, straight_path_flags,
			straight_path_polys, &n_straight_polys, 256, DT_STRAIGHTPATH_ALL_CROSSINGS);

		dtFreeNavMeshQuery(query);

		if (n_straight_polys) {
			m_path_renderable->Clear();

			for (int i = 0; i < n_straight_polys - 1; ++i) {
				glm::vec3 s(straight_path[i * 3], straight_path[i * 3 + 1] + 0.4f, straight_path[i * 3 + 2]);
				glm::vec3 e(straight_path[(i + 1) * 3], straight_path[(i + 1) * 3 + 1] + 0.4f, straight_path[(i + 1) * 3 + 2]);
				m_path_renderable->AddLine(s, e);
			}

			m_path_renderable->Update();
		}
	}
	else {
		dtFreeNavMeshQuery(query);
		return;
	}
}

void ModuleNavigation::InitVolumes()
{
	m_volumes.clear();

	auto physics = m_scene->GetZonePhysics();
	if (!physics)
		return;

	WaterMap *w = physics->GetWaterMap();
	if (!w)
		return;

	std::vector<RegionDetails> regions;
	w->GetRegionDetails(regions);

	for (auto &region : regions) {
		RegionVolume v;

		v.min = FLT_MAX;
		v.max = -FLT_MAX;

		for (int i = 0; i < 4; ++i) {
			if (region.verts[i].y < v.min) {
				v.min = region.verts[i].y;
			} else if (region.verts[i].y > v.max) {
				v.max = region.verts[i].y;
			}
		}

		switch (region.type) {
		case RegionTypeNormal:
			v.area_type = NavigationAreaFlagNormal;
			break;
		case RegionTypeWater:
			v.area_type = NavigationAreaFlagWater;
			break;
		case RegionTypeLava:
			v.area_type = NavigationAreaFlagLava;
			break;
		case RegionTypePVP:
			v.area_type = NavigationAreaFlagPvP;
			break;
		case RegionTypeSlime:
			v.area_type = NavigationAreaFlagSlime;
			break;
		case RegionTypeIce:
			v.area_type = NavigationAreaFlagIce;
			break;
		case RegionTypeVWater:
			v.area_type = NavigationAreaFlagVWater;
			break;
		default:
			v.area_type = NavigationAreaFlagNormal;
		}

		for (int i = 0; i < 4; ++i) {
			v.verts[(i * 3)] = region.verts[i].x;
			v.verts[(i * 3) + 1] = region.verts[i].y - v.min;
			v.verts[(i * 3) + 2] = region.verts[i].z;
		}

		m_volumes.push_back(v);
	}

	//todo: move this to "render volume"
	//NavigationDebugDraw dd;
	//dd.model = m_debug_renderable.get();
	//m_debug_renderable->Clear();
	//for (auto &volume : m_volumes) {
	//	dd.begin(DU_DRAW_LINES, 2.0f);
	//	//min
	//	//1 -> 2
	//	dd.vertex(volume.verts[(0 * 3) + 0], volume.min, volume.verts[(0 * 3) + 2], duRGBA(255, 255, 255, 64));
	//	dd.vertex(volume.verts[(1 * 3) + 0], volume.min, volume.verts[(1 * 3) + 2], duRGBA(255, 255, 255, 64));
	//
	//	//2 -> 3
	//	dd.vertex(volume.verts[(1 * 3) + 0], volume.min, volume.verts[(1 * 3) + 2], duRGBA(255, 255, 255, 64));
	//	dd.vertex(volume.verts[(2 * 3) + 0], volume.min, volume.verts[(2 * 3) + 2], duRGBA(255, 255, 255, 64));
	//
	//	//3 -> 4
	//	dd.vertex(volume.verts[(2 * 3) + 0], volume.min, volume.verts[(2 * 3) + 2], duRGBA(255, 255, 255, 64));
	//	dd.vertex(volume.verts[(3 * 3) + 0], volume.min, volume.verts[(3 * 3) + 2], duRGBA(255, 255, 255, 64));
	//
	//	//4 -> 1
	//	dd.vertex(volume.verts[(3 * 3) + 0], volume.min, volume.verts[(3 * 3) + 2], duRGBA(255, 255, 255, 64));
	//	dd.vertex(volume.verts[(0 * 3) + 0], volume.min, volume.verts[(0 * 3) + 2], duRGBA(255, 255, 255, 64));
	//
	//	//max
	//	//1 -> 2
	//	dd.vertex(volume.verts[(0 * 3) + 0], volume.max, volume.verts[(0 * 3) + 2], duRGBA(255, 255, 255, 64));
	//	dd.vertex(volume.verts[(1 * 3) + 0], volume.max, volume.verts[(1 * 3) + 2], duRGBA(255, 255, 255, 64));
	//
	//	//2 -> 3
	//	dd.vertex(volume.verts[(1 * 3) + 0], volume.max, volume.verts[(1 * 3) + 2], duRGBA(255, 255, 255, 64));
	//	dd.vertex(volume.verts[(2 * 3) + 0], volume.max, volume.verts[(2 * 3) + 2], duRGBA(255, 255, 255, 64));
	//
	//	//3 -> 4
	//	dd.vertex(volume.verts[(2 * 3) + 0], volume.max, volume.verts[(2 * 3) + 2], duRGBA(255, 255, 255, 64));
	//	dd.vertex(volume.verts[(3 * 3) + 0], volume.max, volume.verts[(3 * 3) + 2], duRGBA(255, 255, 255, 64));
	//
	//	//4 -> 1
	//	dd.vertex(volume.verts[(3 * 3) + 0], volume.max, volume.verts[(3 * 3) + 2], duRGBA(255, 255, 255, 64));
	//	dd.vertex(volume.verts[(0 * 3) + 0], volume.max, volume.verts[(0 * 3) + 2], duRGBA(255, 255, 255, 64));
	//
	//	//connect each to themselves...
	//	dd.vertex(volume.verts[(0 * 3) + 0], volume.min, volume.verts[(0 * 3) + 2], duRGBA(255, 255, 255, 64));
	//	dd.vertex(volume.verts[(0 * 3) + 0], volume.max, volume.verts[(0 * 3) + 2], duRGBA(255, 255, 255, 64));
	//
	//	dd.vertex(volume.verts[(1 * 3) + 0], volume.min, volume.verts[(1 * 3) + 2], duRGBA(255, 255, 255, 64));
	//	dd.vertex(volume.verts[(1 * 3) + 0], volume.max, volume.verts[(1 * 3) + 2], duRGBA(255, 255, 255, 64));
	//
	//	dd.vertex(volume.verts[(2 * 3) + 0], volume.min, volume.verts[(2 * 3) + 2], duRGBA(255, 255, 255, 64));
	//	dd.vertex(volume.verts[(2 * 3) + 0], volume.max, volume.verts[(2 * 3) + 2], duRGBA(255, 255, 255, 64));
	//
	//	dd.vertex(volume.verts[(3 * 3) + 0], volume.min, volume.verts[(3 * 3) + 2], duRGBA(255, 255, 255, 64));
	//	dd.vertex(volume.verts[(3 * 3) + 0], volume.max, volume.verts[(3 * 3) + 2], duRGBA(255, 255, 255, 64));
	//
	//	dd.end();
	//}
	//m_debug_renderable->Update();
}

void NavigationDebugDraw::begin(duDebugDrawPrimitives prim, float size) {
	verts_in_use = 0;
	switch (prim)
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
	if (mode == 0 || verts_in_use != mode) {
		return;
	}

	switch (mode) {
	case 1:
	{
		unsigned int index = (unsigned int)model->GetPointsVerts().size();
		model->GetPointsVerts().push_back(verts[0]);
		model->GetPointsInds().push_back(index);
	}
	break;
	case 2:
	{
		unsigned int index = (unsigned int)model->GetLinesVerts().size();
		model->GetLinesVerts().push_back(verts[0]);
		model->GetLinesVerts().push_back(verts[1]);
		model->GetLinesInds().push_back(index);
		model->GetLinesInds().push_back(index + 1);
	}
	break;
	case 3:
	{
		unsigned int index = (unsigned int)model->GetTrianglesVerts().size();
		model->GetTrianglesVerts().push_back(verts[0]);
		model->GetTrianglesVerts().push_back(verts[1]);
		model->GetTrianglesVerts().push_back(verts[2]);
		model->GetTrianglesInds().push_back(index);
		model->GetTrianglesInds().push_back(index + 1);
		model->GetTrianglesInds().push_back(index + 2);
	}
	break; //2 3 0
	case 4:
	{
		unsigned int index = (unsigned int)model->GetTrianglesInds().size();
		model->GetTrianglesVerts().push_back(verts[0]);
		model->GetTrianglesVerts().push_back(verts[1]);
		model->GetTrianglesVerts().push_back(verts[2]);
		model->GetTrianglesVerts().push_back(verts[3]);
		model->GetTrianglesInds().push_back(index);
		model->GetTrianglesInds().push_back(index + 1);
		model->GetTrianglesInds().push_back(index + 2);
		model->GetTrianglesInds().push_back(index + 2);
		model->GetTrianglesInds().push_back(index + 3);
		model->GetTrianglesInds().push_back(index);
	}
	break;
	}

	verts_in_use = 0;
}

