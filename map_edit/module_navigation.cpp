#include <algorithm>
#include <sstream>
#include "string_util.h"
#include "imgui.h"
#include "module_navigation.h"
#include "thread_pool.h"
#include "log_macros.h"
#include "module_navigation_build_tile.h"
#include "compression.h"
#include "event/background_task.h"
#include "event/task.h"

#include "config.h"
#include <DetourNavMeshQuery.h>
#include <DetourCommon.h>

const uint32_t nav_file_version = 3;
const uint32_t nav_mesh_file_version = 2;

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

ModuleNavigation::ModuleNavigation()
{
	m_mode = 1;
	Clear();
	ClearConnections();

	m_work_pending = 0;
	m_nav_mesh = nullptr;

	m_nav_mesh_renderable.reset(new DebugDraw(false));
	m_connections_renderable.reset(new DebugDraw(false));
	m_connections_renderable->GetPointsHandle().SetLineWidth(3.0f);
	m_connections_renderable->GetLinesHandle().SetLineWidth(3.0f);
	m_connections_renderable->GetPointsHandle().SetDepthWriteEnabled(false);
	m_connections_renderable->GetLinesHandle().SetDepthWriteEnabled(false);

	m_start_path_renderable.reset(new DynamicGeometry());
	m_start_path_renderable->SetDrawType(GL_LINES);
	m_start_path_renderable->SetLineWidth(1.0f);

	m_end_path_renderable.reset(new DynamicGeometry());
	m_end_path_renderable->SetDrawType(GL_LINES);
	m_end_path_renderable->SetLineWidth(1.0f);

	m_path_renderable.reset(new DynamicGeometry());
	m_path_renderable->SetDrawType(GL_LINES);
	m_path_renderable->SetLineWidth(2.0f);

	m_path_debug_renderable.reset(new DynamicGeometry());
	m_path_debug_renderable->SetDrawType(GL_LINES);
	m_path_debug_renderable->SetLineWidth(2.0f);

	m_path_start_set = false;
	m_path_end_set = false;
	m_conn_start_set = false;

	m_path_costs[NavigationAreaFlagNormal] = 1.0;
	m_path_costs[NavigationAreaFlagWater] = 3.0;
	m_path_costs[NavigationAreaFlagLava] = 5.0;
	m_path_costs[NavigationAreaFlagZoneLine] = 1.0;
	m_path_costs[NavigationAreaFlagPvP] = 1.0;
	m_path_costs[NavigationAreaFlagSlime] = 2.0;
	m_path_costs[NavigationAreaFlagIce] = 2.0;
	m_path_costs[NavigationAreaFlagVWater] = 3.0;
	m_path_costs[NavigationAreaFlagGeneralArea] = 1.0;
	m_path_costs[NavigationAreaFlagPortal] = 0.1;
	m_path_costs[NavigationAreaFlagPrefer] = 0.1;

	m_flag_enabled[NavigationAreaFlagNormal] = true;
	m_flag_enabled[NavigationAreaFlagWater] = true;
	m_flag_enabled[NavigationAreaFlagLava] = true;
	m_flag_enabled[NavigationAreaFlagZoneLine] = true;
	m_flag_enabled[NavigationAreaFlagPvP] = true;
	m_flag_enabled[NavigationAreaFlagSlime] = true;
	m_flag_enabled[NavigationAreaFlagIce] = true;
	m_flag_enabled[NavigationAreaFlagVWater] = true;
	m_flag_enabled[NavigationAreaFlagGeneralArea] = true;
	m_flag_enabled[NavigationAreaFlagPortal] = true;
	m_flag_enabled[NavigationAreaFlagPrefer] = true;
	m_flag_enabled[NavigationAreaFlagDisabled] = false;

	m_step_size = 10.0f;
	m_complex_path = false;
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
			m_start_path_renderable->Clear();
			m_end_path_renderable->Clear();
			m_path_renderable->Clear();
			m_path_debug_renderable->Clear();
			m_start_path_renderable->Update();
			m_end_path_renderable->Update();
			m_path_renderable->Update();
			m_path_debug_renderable->Update();
			m_path_start_set = false;
			m_path_end_set = false;
			m_conn_start_set = false;
			m_nav_mesh_renderable->Clear();
			m_nav_mesh_renderable->Update();
			m_connections_renderable->Clear();
			m_connections_renderable->Update();
			m_conn_start_set = false;
			m_chunky_mesh.reset();
			if (m_nav_mesh) {
				dtFreeNavMesh(m_nav_mesh);
				m_nav_mesh = nullptr;
			}
			m_volumes.clear();
			Clear();
			ClearConnections();
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
	if (!HasWork()) {
		if (ImGui::RadioButton("NavMesh Generation", &m_mode, (int)ModeNavMeshGen)) {
			m_conn_start_set = false;
			UpdateConnectionsModel();
		}
	}
	else {
		ImGui::RadioButton("NavMesh Generation", m_mode == ModeNavMeshGen);
	}

	if (!HasWork()) {
		if (ImGui::RadioButton("Connections", &m_mode, (int)ModeNavMeshConnections)) {
			m_conn_start_set = false;
			UpdateConnectionsModel();
		}
	}
	else {
		ImGui::RadioButton("Connections", m_mode == ModeNavMeshConnections);
	}

	if (!HasWork()) {
		if (ImGui::RadioButton("Test Mesh", &m_mode, (int)ModeTestNavigation)) {
			m_conn_start_set = false;
			UpdateConnectionsModel();
		}
	}
	else {
		ImGui::RadioButton("Test Mesh", m_mode == ModeTestNavigation);
	}

	ImGui::End();

	if (m_mode == ModeNavMeshGen) {
		DrawNavMeshGenerationUI();
	}

	if (m_mode == ModeTestNavigation) {
		DrawTestUI();
	}

	if (m_mode == ModeNavMeshConnections) {
		DrawMeshConnectionUI();
	}
}

void ModuleNavigation::OnDrawOptions()
{
}

void ModuleNavigation::OnSceneLoad(const char *zone_name)
{
	m_start_path_renderable->Clear();
	m_end_path_renderable->Clear();
	m_path_renderable->Clear();
	m_path_debug_renderable->Clear();
	m_start_path_renderable->Update();
	m_end_path_renderable->Update();
	m_path_renderable->Update();
	m_path_debug_renderable->Update();
	m_path_start_set = false;
	m_path_end_set = false;
	m_conn_start_set = false;
	m_nav_mesh_renderable->Clear();
	m_nav_mesh_renderable->Update();
	m_connections_renderable->Clear();
	m_connections_renderable->Update();
	m_conn_start_set = false;
	m_chunky_mesh.reset();
	ClearConnections();

	if (m_nav_mesh) {
		dtFreeNavMesh(m_nav_mesh);
		m_nav_mesh = nullptr;
	}

	if (!LoadNavSettings()) {
		Clear();
		ClearConnections();
	}

	LoadNavMesh();

	UpdateConnectionsModel();
}

void ModuleNavigation::OnSuspend()
{
	m_scene->UnregisterEntitiesByModule(this);
}

void ModuleNavigation::OnResume()
{
	if (m_nav_mesh_renderable) {
		m_scene->RegisterEntity(this, m_nav_mesh_renderable.get());
	}

	if (m_connections_renderable) {
		m_scene->RegisterEntity(this, m_connections_renderable.get());
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

	if (m_path_debug_renderable) {
		m_scene->RegisterEntity(this, m_path_debug_renderable.get());
	}
}

bool ModuleNavigation::HasWork()
{
	if (m_work_pending > 0) {
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
	SaveNavSettings();
	SaveNavMesh();
}

void ModuleNavigation::OnHotkey(int ident)
{
}

void ModuleNavigation::OnClick(int mouse_button, const glm::vec3 *collide_hit, const glm::vec3 *select_hit, Entity *selected)
{
	auto &io = ImGui::GetIO();
	if (m_mode == ModeNavMeshGen && mouse_button == GLFW_MOUSE_BUTTON_LEFT && !io.KeyShift && collide_hit) {
		BuildTile(*collide_hit);
	}
	else if (m_mode == ModeNavMeshGen && mouse_button == GLFW_MOUSE_BUTTON_LEFT && io.KeyShift && collide_hit) {
		RemoveTile(*collide_hit);
	} else if (m_mode == ModeTestNavigation && mouse_button == GLFW_MOUSE_BUTTON_LEFT && !io.KeyShift && collide_hit) {
		SetNavigationTestNodeStart(*collide_hit);
		CalcPath();
	}
	else if (m_mode == ModeTestNavigation && mouse_button == GLFW_MOUSE_BUTTON_LEFT && io.KeyShift && collide_hit) {
		SetNavigationTestNodeEnd(*collide_hit);
		CalcPath();
	}
	else if (m_mode == ModeNavMeshConnections && mouse_button == GLFW_MOUSE_BUTTON_LEFT && !io.KeyShift && collide_hit) {
		if (!m_conn_start_set) {
			m_conn_start_set = true;
			m_conn_start = *collide_hit;
		}
		else {
			m_conn_start_set = false;
			unsigned short flag = 0;

			switch (m_connection_area)
			{
			case NavigationAreaFlagNormal:
				flag = NavigationPolyFlagNormal;
				break;
			case NavigationAreaFlagWater:
				flag = NavigationPolyFlagWater;
				break;
			case NavigationAreaFlagLava:
				flag = NavigationPolyFlagLava;
				break;
			case NavigationAreaFlagZoneLine:
				flag = NavigationPolyFlagZoneLine;
				break;
			case NavigationAreaFlagPvP:
				flag = NavigationPolyFlagPvP;
				break;
			case NavigationAreaFlagSlime:
				flag = NavigationPolyFlagSlime;
				break;
			case NavigationAreaFlagIce:
				flag = NavigationPolyFlagIce;
				break;
			case NavigationAreaFlagVWater:
				flag = NavigationPolyFlagVWater;
				break;
			case NavigationAreaFlagGeneralArea:
				flag = NavigationPolyFlagGeneralArea;
				break;
			case NavigationAreaFlagPortal:
				flag = NavigationPolyFlagPortal;
				break;
			case NavigationAreaFlagPrefer:
				flag = NavigationAreaFlagPrefer;
				break;
			case NavigationAreaFlagDisabled:
			default:
				flag = NavigationPolyFlagDisabled;
			}

			AddMeshConnection(m_conn_start, *collide_hit, m_connection_radius, m_connection_dir, (unsigned char)m_connection_area, flag);
		}

		UpdateConnectionsModel();
	}
	else if (m_mode == ModeNavMeshConnections && mouse_button == GLFW_MOUSE_BUTTON_LEFT && io.KeyShift && collide_hit) {
		const float *p = (float*)collide_hit;
		float nearest_dist = FLT_MAX;
		int nearest_index = -1;
		unsigned int sz = m_connection_count * 2;
		for (unsigned int i = 0; i < sz; ++i) {
			const float *v = (float*)&m_connection_verts[i];
			float d = rcVdistSqr(p, v);
			if (d < nearest_dist)
			{
				nearest_dist = d;
				nearest_index = i / 2;
			}
		}

		if (nearest_index != -1 &&
			sqrtf(nearest_dist) <= m_connection_rads[nearest_index])
		{
			DeleteMeshConnection(nearest_index);
			UpdateConnectionsModel();
		}
	}
}

void ModuleNavigation::Clear()
{
	m_cell_size = 0.8f;
	m_cell_height = 0.4f;
	m_agent_height = 6.55f;
	m_agent_radius = 1.31f;
	m_agent_max_climb = 6.55f;
	m_agent_max_slope = 60.0f;
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
	m_tile_size = 512.0f;
}

void ModuleNavigation::DrawNavMeshGenerationUI()
{
	auto zone_geo = m_scene->GetZoneGeometry();
	const float* bmin = (float*)&m_scene->GetBoundingBoxMin();
	const float* bmax = (float*)&m_scene->GetBoundingBoxMax();
	int gw = 0, gh = 0;
	if (!zone_geo) {
		return;
	}

	ImGui::Begin("NavMesh Properties");
	ImGui::Text("LMB to place a navigation mesh tile. Shift + LMB to delete a navigation mesh tile.");
	ImGui::Separator();

	ImGui::Text("Rasterization");
	ImGui::SliderFloat("Cell Size", &m_cell_size, 0.1f, 2.0f, "%.1f");
	ImGui::SliderFloat("Cell Height", &m_cell_height, 0.1f, 2.0f, "%.1f");

	rcCalcGridSize(bmin, bmax, m_cell_size, &gw, &gh);
	ImGui::Text(EQEmu::StringFormat("Voxels  %d x %d", gw, gh).c_str());

	ImGui::Separator();
	ImGui::Text("Agent");
	ImGui::SliderFloat("Height", &m_agent_height, 0.1f, 20.0f, "%.1f");
	ImGui::SliderFloat("Radius", &m_agent_radius, 0.0f, 15.0f, "%.1f");
	ImGui::SliderFloat("Max Climb", &m_agent_max_climb, 0.1f, 100.0f, "%.1f");
	ImGui::SliderFloat("Max Slope", &m_agent_max_slope, 0.0f, 90.0f, "%.0f");

	ImGui::Separator();
	ImGui::Text("Region");
	ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.5f);
	ImGui::SliderFloat("Min Region Size", &m_region_min_size, 0.0f, 60.0f, "%.0f");
	ImGui::SliderFloat("Merged Region Size", &m_region_merge_size, 0.0f, 60.0f, "%.0f");
	ImGui::PopItemWidth();

	ImGui::Separator();
	ImGui::Text("Partitioning");
	const char *partition_types[] = { "Watershed", "Monotone", "Layers" };

	ImGui::Combo("Type", &m_partition_type, partition_types, 3);

	ImGui::Separator();
	ImGui::Text("Polygonization");
	ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.5f);
	ImGui::SliderFloat("Max Edge Length", &m_edge_max_len, 0.0f, 128.0f, "%.0f");
	ImGui::SliderFloat("Max Edge Error", &m_edge_max_error, 0.1f, 3.0f, "%.1f");
	ImGui::SliderFloat("Verts Per Poly", &m_verts_per_poly, 3.0f, 6.0f, "%.0f");
	ImGui::PopItemWidth();

	ImGui::Separator();
	ImGui::Text("Detail Mesh");
	ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.5f);
	ImGui::SliderFloat("Sample Distance", &m_detail_sample_dist, 1.0f, 32.0f, "%.0f");
	ImGui::SliderFloat("Max Sample Error", &m_detail_sample_max_error, 1.0f, 16.0f, "%.0f");
	ImGui::PopItemWidth();
	ImGui::Separator();

	ImGui::Text("Tiling");
	ImGui::SliderFloat("TileSize", &m_tile_size, 16.0f, 4096.0f, "%.0f");

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

	if (HasWork()) {
		ImGui::Text(EQEmu::StringFormat("%u tasks remaining...", m_work_pending).c_str());
	}
	else {
		if (ImGui::Button("Build All NavMesh Tiles")) {
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
	bool status = ImGui::SliderFloat("Cost: Normal", &m_path_costs[NavigationAreaFlagNormal], 0.1f, 10.0f, "%.1f");
	status = status || ImGui::SliderFloat("Cost: Water", &m_path_costs[NavigationAreaFlagWater], 0.1f, 10.0f, "%.1f");
	status = status || ImGui::SliderFloat("Cost: Lava", &m_path_costs[NavigationAreaFlagLava], 0.1f, 10.0f, "%.1f");
	status = status || ImGui::SliderFloat("Cost: PvP", &m_path_costs[NavigationAreaFlagPvP], 0.1f, 10.0f, "%.1f");
	status = status || ImGui::SliderFloat("Cost: Slime", &m_path_costs[NavigationAreaFlagSlime], 0.1f, 10.0f, "%.1f");
	status = status || ImGui::SliderFloat("Cost: Ice", &m_path_costs[NavigationAreaFlagIce], 0.1f, 10.0f, "%.1f");
	status = status || ImGui::SliderFloat("Cost: V Water", &m_path_costs[NavigationAreaFlagVWater], 0.1f, 10.0f, "%.1f");
	status = status || ImGui::SliderFloat("Cost: Teleport", &m_path_costs[NavigationAreaFlagPortal], 0.1f, 10.0f, "%.1f");
	status = status || ImGui::SliderFloat("Cost: General Area", &m_path_costs[NavigationAreaFlagGeneralArea], 0.1f, 10.0f, "%.1f");
	status = status || ImGui::SliderFloat("Cost: Prefer", &m_path_costs[NavigationAreaFlagPrefer], 0.1f, 10.0f, "%.1f");

	ImGui::Separator();
	ImGui::Text("Area Flags");
	status = status || ImGui::Checkbox("Flag: Normal", &m_flag_enabled[NavigationAreaFlagNormal]);
	status = status || ImGui::Checkbox("Flag: Water", &m_flag_enabled[NavigationAreaFlagWater]);
	status = status || ImGui::Checkbox("Flag: Lava", &m_flag_enabled[NavigationAreaFlagLava]);
	status = status || ImGui::Checkbox("Flag: Zone Line", &m_flag_enabled[NavigationAreaFlagZoneLine]);
	status = status || ImGui::Checkbox("Flag: PvP", &m_flag_enabled[NavigationAreaFlagPvP]);
	status = status || ImGui::Checkbox("Flag: Slime", &m_flag_enabled[NavigationAreaFlagSlime]);
	status = status || ImGui::Checkbox("Flag: Ice", &m_flag_enabled[NavigationAreaFlagIce]);
	status = status || ImGui::Checkbox("Flag: V Water", &m_flag_enabled[NavigationAreaFlagVWater]);
	status = status || ImGui::Checkbox("Flag: General Area", &m_flag_enabled[NavigationAreaFlagGeneralArea]);
	status = status || ImGui::Checkbox("Flag: Portal", &m_flag_enabled[NavigationAreaFlagPortal]);
	status = status || ImGui::Checkbox("Flag: Prefer", &m_flag_enabled[NavigationAreaFlagPrefer]);
	status = status || ImGui::Checkbox("Flag: Disabled", &m_flag_enabled[NavigationAreaFlagDisabled]);

	ImGui::Separator();
	status = status || ImGui::Checkbox("Smooth Path", &m_complex_path);
	if (m_complex_path) {
		status = status || ImGui::SliderFloat("Step Size", &m_step_size, 0.5f, 256.0f);
	}

	if (status) {
		CalcPath();
	}

	ImGui::End();
}

void ModuleNavigation::DrawMeshConnectionUI()
{
	ImGui::Begin("NavMesh Properties");
	ImGui::Text("LMB to place points. Shift + LMB to delete a connection.");
	ImGui::Separator();
	ImGui::Text("Connection");
	ImGui::RadioButton("One-Way", &m_connection_dir, 0);
	ImGui::SameLine();
	ImGui::RadioButton("Bi-Directional", &m_connection_dir, 1);

	const char* area_types[] = { "Normal", "Water", "Lava", "ZoneLine", "PVP", "Slime", "Ice", "V Water", "Generic Area", "Portal", "Prefer" };

	ImGui::Combo("Area Type", &m_connection_area, area_types, 11);
	
	ImGui::Separator();
	if (ImGui::SliderFloat("Radius", &m_connection_radius, 0.3f, 30.0f, "%.1f")) {
		UpdateConnectionsModel();
	}

	ImGui::End();
}

void ModuleNavigation::BuildNavigationMesh()
{
	if (HasWork()) {
		return;
	}

	std::shared_ptr<EQPhysics> phys = m_scene->GetZonePhysics();
	if (!phys) {
		return;
	}

	InitNavigationMesh();

	const float* bmin = (float*)&m_scene->GetBoundingBoxMin();
	const float* bmax = (float*)&m_scene->GetBoundingBoxMax();
	int gw = (int)((bmax[0] - bmin[0]) / m_cell_size + 0.5f);
	int gh = (int)((bmax[2] - bmin[2]) / m_cell_size + 0.5f);
	const int ts = (int)m_tile_size;
	const int tw = (gw + ts - 1) / ts;
	const int th = (gh + ts - 1) / ts;
	const float tcs = m_tile_size * m_cell_size;

	m_work_pending += (th * tw);

	for (int y = 0; y < th; ++y)
	{
		for (int x = 0; x < tw; ++x)
		{
			glm::vec3 tile_min(bmin[0] + x * tcs, bmin[1], bmin[2] + y * tcs);
			glm::vec3 tile_max(bmin[0] + (x + 1) * tcs, bmax[1], bmin[2] + (y + 1) * tcs);
			
			auto work = new ModuleNavigationBuildTile(this, x, y, tile_min, tile_max, phys);
			EQ::Task([work](EQ::Task::ResolveFn resolve, EQ::Task::RejectFn reject) {
				work->Run();
			}).Finally([work]() {
				work->Finished();
				delete work;
			}).Run();
		}
	}
}

void ModuleNavigation::InitNavigationMesh()
{
	auto zone_geo = m_scene->GetZoneGeometry();
	if (!zone_geo) {
		return;
	}

	CreateChunkyTriMesh(zone_geo);

	LoadVolumes();

	//clear previous data
	if (m_nav_mesh) {
		dtFreeNavMesh(m_nav_mesh);
	}

	m_nav_mesh = dtAllocNavMesh();

	dtNavMeshParams params;
	rcVcopy(params.orig, (float*)&m_scene->GetBoundingBoxMin());
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
}

void ModuleNavigation::BuildTile(const glm::vec3 &pos)
{
	if (HasWork()) {
		return;
	}

	std::shared_ptr<EQPhysics> phys = m_scene->GetZonePhysics();
	if (!phys) {
		return;
	}

	if (!m_nav_mesh) {
		InitNavigationMesh();
	}

	auto bmin = m_scene->GetBoundingBoxMin();
	auto bmax = m_scene->GetBoundingBoxMax();

	const float ts = m_tile_size * m_cell_size;
	const int tx = (int)((pos[0] - bmin[0]) / ts);
	const int ty = (int)((pos[2] - bmin[2]) / ts);

	glm::vec3 tile_min(bmin[0] + tx * ts, bmin[1], bmin[2] + ty * ts);
	glm::vec3 tile_max(bmin[0] + (tx + 1) * ts, bmax[1], bmin[2] + (ty + 1) * ts);
	m_work_pending++;

	auto work = new ModuleNavigationBuildTile(this, tx, ty, tile_min, tile_max, phys);
	EQ::Task([work](EQ::Task::ResolveFn resolve, EQ::Task::RejectFn reject) {
		work->Run();
	}).Finally([work]() {
		work->Finished();
		delete work;
	}).Run();
}

void ModuleNavigation::RemoveTile(const glm::vec3 &pos)
{
	if (HasWork()) {
		return;
	}

	if (m_nav_mesh) {
		auto bmin = m_scene->GetBoundingBoxMin();
		auto bmax = m_scene->GetBoundingBoxMax();

		const float ts = m_tile_size * m_cell_size;
		const int tx = (int)((pos[0] - bmin[0]) / ts);
		const int ty = (int)((pos[2] - bmin[2]) / ts);

		m_nav_mesh->removeTile(m_nav_mesh->getTileRefAt(tx, ty, 0), 0, 0);
		CreateNavMeshModel();
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

		if (!VertexWithinBounds(v1, m_scene->GetBoundingBoxMin(), m_scene->GetBoundingBoxMax())) {
			continue;
		}

		if (!VertexWithinBounds(v2, m_scene->GetBoundingBoxMin(), m_scene->GetBoundingBoxMax())) {
			continue;
		}

		if (!VertexWithinBounds(v3, m_scene->GetBoundingBoxMin(), m_scene->GetBoundingBoxMax())) {
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
	m_nav_mesh_renderable->Clear();
	NavigationDebugDraw dd;
	dd.model = m_nav_mesh_renderable.get();
	duDebugDrawNavMesh(&dd, *m_nav_mesh, 0xffu ^ DU_DRAWNAVMESH_OFFMESHCONS);
	m_nav_mesh_renderable->SetTrianglesTint(glm::vec4(0.0f, 0.75f, 1.0f, 0.25f));
	m_nav_mesh_renderable->SetLinesTint(glm::vec4(0.0f, 0.2f, 0.25f, 0.8f));
	m_nav_mesh_renderable->SetPointsTint(glm::vec4(0.0f, 0.0f, 0.75f, 0.5f));
	m_nav_mesh_renderable->Update();
}

void ModuleNavigation::SetNavigationTestNodeStart(const glm::vec3 &p)
{
	m_path_start = p;
	m_path_start_set = true;
	m_start_path_renderable->Clear();
	float sz = 2.0f;
	m_start_path_renderable->AddLineCylinder(glm::vec3(m_path_start.x - sz, m_path_start.y, m_path_start.z - sz),
		glm::vec3(m_path_start.x + sz, m_path_start.y + (sz * 2), m_path_start.z + sz),
		glm::vec3(0.0, 1.0, 0.0));

	m_start_path_renderable->Update();
}

void ModuleNavigation::SetNavigationTestNodeEnd(const glm::vec3 &p)
{
	m_path_end = p;
	m_path_end_set = true;
	m_end_path_renderable->Clear();
	float sz = 2.0f;
	m_end_path_renderable->AddLineCylinder(glm::vec3(m_path_end.x - sz, m_path_end.y, m_path_end.z - sz),
		glm::vec3(m_path_end.x + sz, m_path_end.y + (sz * 2), m_path_end.z + sz),
		glm::vec3(1.0, 0.0, 0.0));
	m_end_path_renderable->Update();
}

dtStatus ModuleNavigation::GetPolyHeightNoConnections(const dtNavMeshQuery *query, dtPolyRef ref, const float *pos, float *height) const
{
	auto *m_nav = query->getAttachedNavMesh();

	if (!m_nav) {
		return DT_FAILURE;
	}

	const dtMeshTile* tile = 0;
	const dtPoly* poly = 0;
	if (dtStatusFailed(m_nav->getTileAndPolyByRef(ref, &tile, &poly))) {
		return DT_FAILURE | DT_INVALID_PARAM;
	}

	if (poly->getType() != DT_POLYTYPE_OFFMESH_CONNECTION) {
		const unsigned int ip = (unsigned int)(poly - tile->polys);
		const dtPolyDetail* pd = &tile->detailMeshes[ip];
		for (int j = 0; j < pd->triCount; ++j)
		{
			const unsigned char* t = &tile->detailTris[(pd->triBase + j) * 4];
			const float* v[3];
			for (int k = 0; k < 3; ++k)
			{
				if (t[k] < poly->vertCount)
					v[k] = &tile->verts[poly->verts[t[k]] * 3];
				else
					v[k] = &tile->detailVerts[(pd->vertBase + (t[k] - poly->vertCount)) * 3];
			}
			float h;
			if (dtClosestHeightPointTriangle(pos, v[0], v[1], v[2], h))
			{
				if (height)
					*height = h;
				return DT_SUCCESS;
			}
		}
	}

	return DT_FAILURE | DT_INVALID_PARAM;
}

dtStatus ModuleNavigation::GetPolyHeightOnPath(const dtPolyRef *path, const int path_len, const glm::vec3 &pos, const dtNavMeshQuery *query, float *h) const
{
	if (!path || !path_len) {
		return DT_FAILURE;
	}

	for (int i = 0; i < path_len; ++i) {
		dtPolyRef ref = path[i];

		if (dtStatusSucceed(GetPolyHeightNoConnections(query, ref, &pos[0], h))) {
			return DT_SUCCESS;
		}
	}

	return DT_FAILURE;
}

void ModuleNavigation::CalcPath()
{
	if (!m_path_start_set || !m_path_end_set || !m_nav_mesh) {
		return;
	}

	glm::vec3 ext(15.0f, 100.0f, 15.0f);
	dtQueryFilter filter;
	
	unsigned short flags = 0;
	if (m_flag_enabled[NavigationAreaFlagNormal]) {
		flags |= NavigationPolyFlagNormal;
	}
	
	if (m_flag_enabled[NavigationAreaFlagWater]) {
		flags |= NavigationPolyFlagWater;
	}
	
	if (m_flag_enabled[NavigationAreaFlagLava]) {
		flags |= NavigationPolyFlagLava;
	}
	
	if (m_flag_enabled[NavigationAreaFlagZoneLine]) {
		flags |= NavigationPolyFlagZoneLine;
	}
	
	if (m_flag_enabled[NavigationAreaFlagPvP]) {
		flags |= NavigationPolyFlagPvP;
	}
	
	if (m_flag_enabled[NavigationAreaFlagSlime]) {
		flags |= NavigationPolyFlagSlime;
	}
	
	if (m_flag_enabled[NavigationAreaFlagIce]) {
		flags |= NavigationPolyFlagIce;
	}
	
	if (m_flag_enabled[NavigationAreaFlagVWater]) {
		flags |= NavigationPolyFlagVWater;
	}
	
	if (m_flag_enabled[NavigationAreaFlagGeneralArea]) {
		flags |= NavigationPolyFlagGeneralArea;
	}
	
	if (m_flag_enabled[NavigationAreaFlagPortal]) {
		flags |= NavigationPolyFlagPortal;
	}
	
	if (m_flag_enabled[NavigationAreaFlagPrefer]) {
		flags |= NavigationPolyFlagPrefer;
	}
	
	if (m_flag_enabled[NavigationAreaFlagDisabled]) {
		flags |= NavigationPolyFlagDisabled;
	}
	
	filter.setIncludeFlags(flags);
	filter.setAreaCost(NavigationAreaFlagNormal, m_path_costs[NavigationAreaFlagNormal]);
	filter.setAreaCost(NavigationAreaFlagWater, m_path_costs[NavigationAreaFlagWater]);
	filter.setAreaCost(NavigationAreaFlagLava, m_path_costs[NavigationAreaFlagLava]);
	filter.setAreaCost(NavigationAreaFlagPvP, m_path_costs[NavigationAreaFlagPvP]);
	filter.setAreaCost(NavigationAreaFlagSlime, m_path_costs[NavigationAreaFlagSlime]);
	filter.setAreaCost(NavigationAreaFlagIce, m_path_costs[NavigationAreaFlagIce]);
	filter.setAreaCost(NavigationAreaFlagVWater, m_path_costs[NavigationAreaFlagVWater]);
	filter.setAreaCost(NavigationAreaFlagGeneralArea, m_path_costs[NavigationAreaFlagGeneralArea]);
	filter.setAreaCost(NavigationAreaFlagPortal, m_path_costs[NavigationAreaFlagPortal]);
	filter.setAreaCost(NavigationAreaFlagPrefer, m_path_costs[NavigationAreaFlagPrefer]);
	
	dtNavMeshQuery *query = dtAllocNavMeshQuery();
	query->init(m_nav_mesh, 32768);
	dtPolyRef start_ref;
	dtPolyRef end_ref;
	
	eqLogMessage(LogInfo, "Calculating path from (%.2f, %.2f, %.2f) -> (%.2f, %.2f, %.2f)",
		m_path_start[0], m_path_start[1], m_path_start[2],
		m_path_end[0], m_path_end[1], m_path_end[2]);
	
	query->findNearestPoly(&m_path_start[0], &ext[0], &filter, &start_ref, 0);
	query->findNearestPoly(&m_path_end[0], &ext[0], &filter, &end_ref, 0);
	
	if (!start_ref || !end_ref) {
		m_path_renderable->Clear();
		m_path_renderable->Update();
		m_path_debug_renderable->Clear();
		m_path_debug_renderable->Update();
		dtFreeNavMeshQuery(query);
		return;
	}

	static const int MAX_POLYS = 256;
	static const int MAX_SMOOTH = 2048;
	
	dtPolyRef polys[MAX_POLYS];
	int npolys = 0;
	
	query->findPath(start_ref, end_ref, &m_path_start[0], &m_path_end[0], &filter, &polys[0], &npolys, MAX_POLYS);
	
	if (m_complex_path) {
		if (npolys) {
			int spath_n = 0;
			glm::vec3 spath[MAX_POLYS];
			unsigned char spath_flags[MAX_POLYS];
			dtPolyRef spath_refs[MAX_POLYS];
			query->findStraightPath(&m_path_start[0], &m_path_end[0], polys, npolys, (float*)&spath[0], spath_flags, spath_refs, &spath_n, MAX_POLYS, 
				DT_STRAIGHTPATH_AREA_CROSSINGS | DT_STRAIGHTPATH_ALL_CROSSINGS);

			m_path_renderable->Clear();

			auto &initial = spath[0];
			m_path_renderable->AddLineCylinder(glm::vec3(initial.x - 0.5f, initial.y - 0.5f, initial.z - 0.5f), glm::vec3(initial.x + 0.5f, initial.y + 0.5f, initial.z + 0.5f), glm::vec3(1.0f, 1.0f, 0.0f));
			eqLogMessage(LogInfo, "Adding initial point (%.2f, %.2f, %.2f)", initial.x, initial.y, initial.z);

			for (auto i = 0; i < spath_n - 1; ++i) {
				auto &p1 = spath[i];
				auto &p2 = spath[i + 1];
				auto &flag = spath_flags[i];

				if (flag & DT_STRAIGHTPATH_OFFMESH_CONNECTION) {
					m_path_renderable->AddLine(p1, p2, glm::vec3(0.0f, 0.0f, 1.0f));
				}
				else {
					auto dist = glm::distance(p1, p2);
					auto dir = glm::normalize(p2 - p1);
					float total = 0.0f;
					glm::vec3 previous_pt = p1;

					while (total < dist) {
						glm::vec3 current_pt;
						float dist_to_move = m_step_size;
						float ff = m_step_size / 2.0f;

						if (total + dist_to_move + ff >= dist) {
							current_pt = p2;
							total = dist;
						}
						else {
							total += dist_to_move;
							current_pt = p1 + dir * total;
						}

						float h = 0.0f;
						if (dtStatusSucceed(GetPolyHeightOnPath(polys, npolys, current_pt, query, &h))) {
							current_pt.y = h;
						}

						eqLogMessage(LogInfo, "Adding current point (%.2f, %.2f, %.2f) %.2f/%.2f", current_pt.x, current_pt.y, current_pt.z, total, dist);
						m_path_renderable->AddLineCylinder(glm::vec3(current_pt.x - 0.5f, current_pt.y - 0.5f, current_pt.z - 0.5f), glm::vec3(current_pt.x + 0.5f, current_pt.y + 0.5f, current_pt.z + 0.5f), glm::vec3(1.0f, 1.0f, 0.0f));

						previous_pt = current_pt;
					}
				}
			}

			m_path_renderable->Update();
		}
		
		dtFreeNavMeshQuery(query);
	}
	else {
		if (npolys) {
			float spath[MAX_POLYS * 3];
			unsigned char spath_flags[MAX_POLYS];
			dtPolyRef spath_refs[MAX_POLYS];
			int spath_n = 0;
			query->findStraightPath(&m_path_start[0], &m_path_end[0], polys, npolys, spath, spath_flags, spath_refs, &spath_n, MAX_POLYS, DT_STRAIGHTPATH_AREA_CROSSINGS | DT_STRAIGHTPATH_ALL_CROSSINGS);

			if (spath_n) {
				m_path_renderable->Clear();
				
				for (auto i = 0; i < spath_n - 1; ++i) {
					auto &x1 = spath[i * 3];
					auto &y1 = spath[i * 3 + 1];
					auto &z1 = spath[i * 3 + 2];
					auto &x2 = spath[i * 3 + 3];
					auto &y2 = spath[i * 3 + 4];
					auto &z2 = spath[i * 3 + 5];
					m_path_renderable->AddLine(glm::vec3(x1, y1, z1), glm::vec3(x2, y2, z2), glm::vec3(1.0f, 1.0f, 0.0f));
				}

				m_path_renderable->Update();
			}
		}
		else {
			dtFreeNavMeshQuery(query);
		}
	}
}

void ModuleNavigation::SaveNavSettings()
{
	//write project setting files
	//zone_name.navprj
	std::string filename = Config::Instance().GetPath("project", "maps/project/") + "/" + m_scene->GetZoneName() + ".navprj";
	FILE *f = fopen(filename.c_str() , "wb");

	if (f) {
		char magic[6] = { 'N', 'A', 'V', 'P', 'R', 'J' };
		fwrite(magic, sizeof(magic), 1, f);
		fwrite(&nav_file_version, sizeof(uint32_t), 1, f);

		auto &bmin = m_scene->GetBoundingBoxMin();
		auto &bmax = m_scene->GetBoundingBoxMax();

		fwrite(&bmin.x, sizeof(float), 1, f);
		fwrite(&bmin.y, sizeof(float), 1, f);
		fwrite(&bmin.z, sizeof(float), 1, f);
		fwrite(&bmax.x, sizeof(float), 1, f);
		fwrite(&bmax.y, sizeof(float), 1, f);
		fwrite(&bmax.z, sizeof(float), 1, f);
		
		fwrite(&m_cell_size, sizeof(float), 1, f);
		fwrite(&m_cell_height, sizeof(float), 1, f);
		fwrite(&m_agent_height, sizeof(float), 1, f);
		fwrite(&m_agent_radius, sizeof(float), 1, f);
		fwrite(&m_agent_max_climb, sizeof(float), 1, f);
		fwrite(&m_agent_max_slope, sizeof(float), 1, f);
		fwrite(&m_region_min_size, sizeof(float), 1, f);
		fwrite(&m_region_merge_size, sizeof(float), 1, f);
		fwrite(&m_edge_max_len, sizeof(float), 1, f);
		fwrite(&m_edge_max_error, sizeof(float), 1, f);
		fwrite(&m_verts_per_poly, sizeof(float), 1, f);
		fwrite(&m_detail_sample_dist, sizeof(float), 1, f);
		fwrite(&m_detail_sample_max_error, sizeof(float), 1, f);
		fwrite(&m_tile_size, sizeof(float), 1, f);

		int32_t partition_type = m_partition_type;
		int32_t max_tiles = m_partition_type;
		int32_t max_polys_per_tile = m_partition_type;
		fwrite(&partition_type, sizeof(int32_t), 1, f);
		fwrite(&max_tiles, sizeof(int32_t), 1, f);
		fwrite(&max_polys_per_tile, sizeof(int32_t), 1, f);

		fwrite(&m_connection_dir, sizeof(uint8_t), 1, f);
		fwrite(&m_connection_area, sizeof(uint32_t), 1, f);
		fwrite(&m_connection_radius, sizeof(float), 1, f);
		fwrite(&m_connection_id_counter, sizeof(uint32_t), 1, f);
		fwrite(&m_connection_count, sizeof(uint32_t), 1, f);
		for (unsigned int i = 0; i < m_connection_count; ++i) {
			fwrite(&m_connection_verts[i * 2], sizeof(float) * 3, 1, f);
			fwrite(&m_connection_verts[(i * 2) + 1], sizeof(float) * 3, 1, f);
			fwrite(&m_connection_rads[i], sizeof(float), 1, f);
			fwrite(&m_connection_dirs[i], sizeof(uint8_t), 1, f);
			fwrite(&m_connection_areas[i], sizeof(uint8_t), 1, f);
			fwrite(&m_connection_flags[i], sizeof(uint16_t), 1, f);
			fwrite(&m_connection_ids[i], sizeof(uint32_t), 1, f);
		}

		fclose(f);
	}
}

bool ModuleNavigation::LoadNavSettings()
{
	std::string filename = Config::Instance().GetPath("project", "maps/project/") + "/" + m_scene->GetZoneName() + ".navprj";
	FILE *f = fopen(filename.c_str(), "rb");
	if (f) {
		char magic[6] = { 0 };
		if (fread(magic, 6, 1, f) != 1) {
			fclose(f);
			return false;
		}

		if(strncmp(magic, "NAVPRJ", 6) != 0)
		{
			fclose(f);
			return false;
		}

		uint32_t version = 0;
		if (fread(&version, sizeof(uint32_t), 1, f) != 1) {
			fclose(f);
			return false;
		}

		auto &bmin = m_scene->GetBoundingBoxMin();
		auto &bmax = m_scene->GetBoundingBoxMax();

		if (version == nav_file_version) {
			if (fread(&bmin.x, sizeof(float), 1, f) != 1) {
				fclose(f);
				return false;
			}

			if (fread(&bmin.y, sizeof(float), 1, f) != 1) {
				fclose(f);
				return false;
			}

			if (fread(&bmin.z, sizeof(float), 1, f) != 1) {
				fclose(f);
				return false;
			}

			if (fread(&bmax.x, sizeof(float), 1, f) != 1) {
				fclose(f);
				return false;
			}

			if (fread(&bmax.y, sizeof(float), 1, f) != 1) {
				fclose(f);
				return false;
			}

			if (fread(&bmax.z, sizeof(float), 1, f) != 1) {
				fclose(f);
				return false;
			}

			if (fread(&m_cell_size, sizeof(float), 1, f) != 1) {
				fclose(f);
				return false;
			}

			if (fread(&m_cell_height, sizeof(float), 1, f) != 1) {
				fclose(f);
				return false;
			}

			if (fread(&m_agent_height, sizeof(float), 1, f) != 1) {
				fclose(f);
				return false;
			}

			if (fread(&m_agent_radius, sizeof(float), 1, f) != 1) {
				fclose(f);
				return false;
			}

			if (fread(&m_agent_max_climb, sizeof(float), 1, f) != 1) {
				fclose(f);
				return false;
			}

			if (fread(&m_agent_max_slope, sizeof(float), 1, f) != 1) {
				fclose(f);
				return false;
			}

			if (fread(&m_region_min_size, sizeof(float), 1, f) != 1) {
				fclose(f);
				return false;
			}

			if (fread(&m_region_merge_size, sizeof(float), 1, f) != 1) {
				fclose(f);
				return false;
			}

			if (fread(&m_edge_max_len, sizeof(float), 1, f) != 1) {
				fclose(f);
				return false;
			}

			if (fread(&m_edge_max_error, sizeof(float), 1, f) != 1) {
				fclose(f);
				return false;
			}

			if (fread(&m_verts_per_poly, sizeof(float), 1, f) != 1) {
				fclose(f);
				return false;
			}

			if (fread(&m_detail_sample_dist, sizeof(float), 1, f) != 1) {
				fclose(f);
				return false;
			}

			if (fread(&m_detail_sample_max_error, sizeof(float), 1, f) != 1) {
				fclose(f);
				return false;
			}

			if (fread(&m_tile_size, sizeof(float), 1, f) != 1) {
				fclose(f);
				return false;
			}
			
			int32_t partition_type = 0;
			int32_t max_tiles = 0;
			int32_t max_polys_per_tile = 0;

			if (fread(&partition_type, sizeof(int32_t), 1, f) != 1) {
				fclose(f);
				return false;
			}

			if (fread(&max_tiles, sizeof(int32_t), 1, f) != 1) {
				fclose(f);
				return false;
			}

			if (fread(&max_polys_per_tile, sizeof(int32_t), 1, f) != 1) {
				fclose(f);
				return false;
			}

			m_partition_type = (int)partition_type;
			m_max_tiles = (int)max_tiles;
			m_max_polys_per_tile = (int)max_polys_per_tile;

			if (fread(&m_connection_dir, sizeof(uint8_t), 1, f) != 1) {
				fclose(f);
				return false;
			}

			if (fread(&m_connection_area, sizeof(uint32_t), 1, f) != 1) {
				fclose(f);
				return false;
			}

			if (fread(&m_connection_radius, sizeof(float), 1, f) != 1) {
				fclose(f);
				return false;
			}

			if (fread(&m_connection_id_counter, sizeof(uint32_t), 1, f) != 1) {
				fclose(f);
				return false;
			}

			if (fread(&m_connection_count, sizeof(uint32_t), 1, f) != 1) {
				fclose(f);
				return false;
			}

			for (unsigned int i = 0; i < m_connection_count; ++i) {
				for (int j = 0; j < 2; ++j) {
					float x, y, z;
					if (fread(&x, sizeof(float), 1, f) != 1) {
						fclose(f);
						return false;
					}

					if (fread(&y, sizeof(float), 1, f) != 1) {
						fclose(f);
						return false;
					}

					if (fread(&z, sizeof(float), 1, f) != 1) {
						fclose(f);
						return false;
					}

					m_connection_verts.push_back(glm::vec3(x, y, z));
				}

				float rads;
				uint8_t dirs;
				uint8_t areas;
				uint16_t flags;
				uint32_t ids;
				if (fread(&rads, sizeof(float), 1, f) != 1) {
					fclose(f);
					return false;
				}

				if (fread(&dirs, sizeof(uint8_t), 1, f) != 1) {
					fclose(f);
					return false;
				}

				if (fread(&areas, sizeof(uint8_t), 1, f) != 1) {
					fclose(f);
					return false;
				}

				if (fread(&flags, sizeof(uint16_t), 1, f) != 1) {
					fclose(f);
					return false;
				}

				if (fread(&ids, sizeof(uint32_t), 1, f) != 1) {
					fclose(f);
					return false;
				}

				m_connection_rads.push_back(rads);
				m_connection_dirs.push_back(dirs);
				m_connection_areas.push_back(areas);
				m_connection_flags.push_back(flags);
				m_connection_ids.push_back(ids);
			}
		}
		else {
			return false;
		}

		m_scene->UpdateBoundingBox();
		fclose(f);
		return true;
	}

	return false;
}

void ModuleNavigation::SaveNavMesh()
{
	//write navmesh out
	//zone_name.nav
	if (!m_nav_mesh)
		return;

	std::string filename = Config::Instance().GetPath("nav", "maps/nav/") + "/" + m_scene->GetZoneName() + ".nav";
	FILE *f = fopen(filename.c_str(), "wb");

	if (f) {
		const dtNavMesh *mesh = m_nav_mesh;
		char magic[9] = { 'E', 'Q', 'N', 'A', 'V', 'M', 'E', 'S', 'H' };
		fwrite(magic, sizeof(magic), 1, f);
		fwrite(&nav_mesh_file_version, sizeof(uint32_t), 1, f);

		std::stringstream ss(std::stringstream::in | std::stringstream::out | std::stringstream::binary);

		uint32_t number_of_tiles = 0;
		for (int i = 0; i < m_nav_mesh->getMaxTiles(); ++i)
		{
			const dtMeshTile* tile = mesh->getTile(i);
			if (!tile || !tile->header || !tile->dataSize) 
				continue;
			number_of_tiles++;
		}

		ss.write((const char*)&number_of_tiles, sizeof(uint32_t));

		dtNavMeshParams params;
		memcpy(&params, mesh->getParams(), sizeof(dtNavMeshParams));
		ss.write((const char*)&params, sizeof(dtNavMeshParams));

		for (int i = 0; i < mesh->getMaxTiles(); ++i)
		{
			const dtMeshTile* tile = mesh->getTile(i);
			if (!tile || !tile->header || !tile->dataSize) 
				continue;

			//write tileref uint32
			uint32_t tile_ref = mesh->getTileRef(tile);
			ss.write((const char*)&tile_ref, sizeof(uint32_t));

			//write datasize int32
			int32_t data_size = tile->dataSize;
			ss.write((const char*)&data_size, sizeof(int32_t));

			ss.write((const char*)tile->data, data_size);
		}

		std::vector<char> buffer;
		auto buffer_len = ss.str().length() + 128;
		buffer.resize(buffer_len);

		uint32_t out_size = (uint32_t)EQEmu::DeflateData(ss.str().c_str(), (uint32_t)ss.str().length(), &buffer[0], buffer_len);
		fwrite(&out_size, sizeof(uint32_t), 1, f);
		uint32_t uncompressed_size = (uint32_t)ss.str().length();
		fwrite(&uncompressed_size, sizeof(uint32_t), 1, f);
		fwrite(&buffer[0], out_size, 1, f);
		fclose(f);
	}
}

void ModuleNavigation::LoadNavMesh()
{
	std::string filename = Config::Instance().GetPath("nav", "maps/nav/") + "/" + m_scene->GetZoneName() + ".nav";
	FILE *f = fopen(filename.c_str(), "rb");
	if (f) {
		char magic[9] = { 0 };
		if (fread(magic, 9, 1, f) != 1) {
			fclose(f);
			return;
		}

		if (strncmp(magic, "EQNAVMESH", 9) != 0)
		{
			fclose(f);
			return;
		}

		uint32_t version = 0;
		if (fread(&version, sizeof(uint32_t), 1, f) != 1) {
			fclose(f);
			return;
		}

		if (version != nav_mesh_file_version) {
			fclose(f);
			return;
		}

		uint32_t data_size;
		if (fread(&data_size, sizeof(data_size), 1, f) != 1) {
			fclose(f);
			return;
		}

		uint32_t buffer_size;
		if (fread(&buffer_size, sizeof(buffer_size), 1, f) != 1) {
			fclose(f);
			return;
		}

		std::vector<char> data;
		data.resize(data_size);
		if (fread(&data[0], data_size, 1, f) != 1) {
			fclose(f);
			return;
		}

		std::vector<char> buffer;
		buffer.resize(buffer_size);
		uint32_t v = EQEmu::InflateData(&data[0], data_size, &buffer[0], buffer_size);
		fclose(f);

		char *buf = &buffer[0];
		m_nav_mesh = dtAllocNavMesh();
		
		uint32_t number_of_tiles = *(uint32_t*)buf;
		buf += sizeof(uint32_t);

		dtNavMeshParams params = *(dtNavMeshParams*)buf;
		buf += sizeof(dtNavMeshParams);

		dtStatus status = m_nav_mesh->init(&params);
		if (dtStatusFailed(status))
		{
			dtFreeNavMesh(m_nav_mesh);
			m_nav_mesh = nullptr;
			return;
		}

		for (unsigned int i = 0; i < number_of_tiles; ++i)
		{
			uint32_t tile_ref = *(uint32_t*)buf;
			buf += sizeof(uint32_t);
			
			int32_t data_size = *(uint32_t*)buf;
			buf += sizeof(uint32_t);

			if (!tile_ref || !data_size) {
				dtFreeNavMesh(m_nav_mesh);
				m_nav_mesh = nullptr;
				return;
			}

			unsigned char* data = (unsigned char*)dtAlloc(data_size, DT_ALLOC_PERM);
			memcpy(data, buf, data_size);
			buf += data_size;

			m_nav_mesh->addTile(data, data_size, DT_TILE_FREE_DATA, tile_ref, 0);
		}

		auto zone_geo = m_scene->GetZoneGeometry();
		if (zone_geo) {
			CreateChunkyTriMesh(zone_geo);
		}
		CreateNavMeshModel();
	}
}

void ModuleNavigation::LoadVolumes()
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
			}
			else if (region.verts[i].y > v.max) {
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
		case RegionTypeGeneralArea:
			v.area_type = NavigationAreaFlagGeneralArea;
			break;
		case RegionTypePreferPathing:
			v.area_type = NavigationAreaFlagPrefer;
			break;
		case RegionTypeDisableNavMesh:
		default:
			v.area_type = NavigationAreaFlagDisabled;
		}

		for (int i = 0; i < 4; ++i) {
			v.verts[(i * 3)] = region.verts[i].x;
			v.verts[(i * 3) + 1] = region.verts[i].y - v.min;
			v.verts[(i * 3) + 2] = region.verts[i].z;
		}

		m_volumes.push_back(v);
	}
}

void ModuleNavigation::AddMeshConnection(const glm::vec3 &start, const glm::vec3 &end, float radius, unsigned char dir, unsigned char area, unsigned short flags)
{
	m_connection_verts.push_back(start);
	m_connection_verts.push_back(end);
	m_connection_rads.push_back(radius);
	m_connection_dirs.push_back(dir);
	m_connection_areas.push_back(area);
	m_connection_flags.push_back(flags);
	m_connection_ids.push_back(m_connection_id_counter++);
	m_connection_count++;
}

void ModuleNavigation::DeleteMeshConnection(unsigned int i)
{
	int vert_start = i * 2;
	int vert_end = (i + 1) * 2;
	m_connection_verts.erase(m_connection_verts.begin() + vert_start, m_connection_verts.begin() + vert_end);
	m_connection_rads.erase(m_connection_rads.begin() + i);
	m_connection_dirs.erase(m_connection_dirs.begin() + i);
	m_connection_areas.erase(m_connection_areas.begin() + i);
	m_connection_flags.erase(m_connection_flags.begin() + i);
	m_connection_ids.erase(m_connection_ids.begin() + i);
	m_connection_count--;
}

void ModuleNavigation::ClearConnections()
{
	m_connection_verts.clear();
	m_connection_rads.clear();
	m_connection_dirs.clear();
	m_connection_areas.clear();
	m_connection_flags.clear();
	m_connection_ids.clear(); 
	m_connection_id_counter = 1000;
	m_connection_count = 0;
	m_connection_dir = 1;
	m_connection_area = 9;
	m_connection_radius = 1.5f;
}

void ModuleNavigation::UpdateConnectionsModel()
{
	m_connections_renderable->Clear();

	NavigationDebugDraw dd;
	dd.model = m_connections_renderable.get();
	
	unsigned int baseColor = duRGBA(0, 0, 0, 256);

	dd.begin(DU_DRAW_LINES, 2.0f);
	for (unsigned int i = 0; i < m_connection_count; ++i) {
		auto &v1 = m_connection_verts[i * 2];
		auto &v2 = m_connection_verts[i * 2 + 1];

		dd.vertex(v1[0], v1[1], v1[2], baseColor);
		dd.vertex(v1[0], v1[1] + 0.2f, v1[2], baseColor);

		dd.vertex(v2[0], v2[1], v2[2], baseColor);
		dd.vertex(v2[0], v2[1] + 0.2f, v2[2], baseColor);

		duAppendCircle(&dd, v1[0], v1[1] + 0.1f, v1[2], m_connection_rads[i], baseColor);
		duAppendCircle(&dd, v2[0], v2[1] + 0.1f, v2[2], m_connection_rads[i], baseColor);

		duAppendArc(&dd, v1[0], v1[1], v1[2], v2[0], v2[1], v2[2], 0.25f,
			(m_connection_dirs[i] & 1) ? 0.6f : 0.0f, 0.6f, baseColor);
	}
	dd.end();

	if (m_conn_start_set) {
		dd.begin(DU_DRAW_LINES, 1.0f);
		duAppendCircle(&dd, m_conn_start.x, m_conn_start.y + 0.1f, m_conn_start.z, m_connection_radius, baseColor);
		dd.end();
	}

	m_connections_renderable->Update();
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
	auto idx = verts_in_use;
	verts[idx] = glm::vec3(pos[0], pos[1], pos[2]);
	vert_colors[idx] = glm::vec3(color && 0xFF000000U >> 24, color && 0x00FF0000U >> 16, color && 0x0000FF00 >> 8);
	++verts_in_use;
	CreatePrimitive();
}

void NavigationDebugDraw::vertex(const float x, const float y, const float z, unsigned int color) {
	auto idx = verts_in_use;
	verts[idx] = glm::vec3(x, y, z);
	vert_colors[idx] = glm::vec3(color && 0xFF000000U >> 24, color && 0x00FF0000U >> 16, color && 0x0000FF00 >> 8);
	++verts_in_use;
	CreatePrimitive();
}

void NavigationDebugDraw::vertex(const float* pos, unsigned int color, const float* uv) {
	auto idx = verts_in_use;
	verts[idx] = glm::vec3(pos[0], pos[1], pos[2]);
	vert_colors[idx] = glm::vec3(color && 0xFF000000U >> 24, color && 0x00FF0000U >> 16, color && 0x0000FF00 >> 8);
	++verts_in_use;
	CreatePrimitive();
}

void NavigationDebugDraw::vertex(const float x, const float y, const float z, unsigned int color, const float u, const float v) {
	auto idx = verts_in_use;
	verts[idx] = glm::vec3(x, y, z);
	vert_colors[idx] = glm::vec3(color && 0xFF000000U >> 24, color && 0x00FF0000U >> 16, color && 0x0000FF00 >> 8);
	++verts_in_use;
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
		model->GetPointsVertColors().push_back(vert_colors[0]);
		model->GetPointsInds().push_back(index);
	}
	break;
	case 2:
	{
		unsigned int index = (unsigned int)model->GetLinesVerts().size();
		model->GetLinesVerts().push_back(verts[0]);
		model->GetLinesVerts().push_back(verts[1]);
		model->GetLinesVertColors().push_back(vert_colors[0]);
		model->GetLinesVertColors().push_back(vert_colors[1]);
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
		model->GetTrianglesVertColors().push_back(vert_colors[0]);
		model->GetTrianglesVertColors().push_back(vert_colors[1]);
		model->GetTrianglesVertColors().push_back(vert_colors[2]);
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
		model->GetTrianglesVertColors().push_back(vert_colors[0]);
		model->GetTrianglesVertColors().push_back(vert_colors[1]);
		model->GetTrianglesVertColors().push_back(vert_colors[2]);
		model->GetTrianglesVertColors().push_back(vert_colors[3]);
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

