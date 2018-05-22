#include "module_volume.h"
#include <algorithm>

const int HotkeyXNeg = 50;
const int HotkeyXPos = 51;
const int HotkeyYNeg = 52;
const int HotkeyYPos = 53;
const int HotkeyZNeg = 54;
const int HotkeyZPos = 55;
const int HotkeyMode = 56;

ModuleVolume::ModuleVolume()
{
	m_render_volume = true;
	//m_region_list = nullptr;
	//m_region_list_size = 0;
	//m_selected_region = -1;
	//m_volume_entity.reset(new DynamicGeometry());
	//m_volume_entity->SetBlend(true);
	//m_volume_entity->SetTint(glm::vec4(1.0f, 1.0f, 1.0f, 0.7f));
	//m_volume_entity->SetDoublePass(true);
}

ModuleVolume::~ModuleVolume()
{
	//FreeRegionList();
}

void ModuleVolume::OnLoad(Scene *s)
{
	m_scene = s;
	m_scene->RegisterHotkey(this, HotkeyXNeg, GLFW_KEY_LEFT, false, false, false);
	m_scene->RegisterHotkey(this, HotkeyXPos, GLFW_KEY_RIGHT, false, false, false);
	m_scene->RegisterHotkey(this, HotkeyZNeg, GLFW_KEY_UP, false, false, false);
	m_scene->RegisterHotkey(this, HotkeyZPos, GLFW_KEY_DOWN, false, false, false);
	m_scene->RegisterHotkey(this, HotkeyYNeg, GLFW_KEY_PAGE_UP, false, false, false);
	m_scene->RegisterHotkey(this, HotkeyYPos, GLFW_KEY_PAGE_DOWN, false, false, false);
	m_scene->RegisterHotkey(this, HotkeyMode, GLFW_KEY_F5, false, false, false);
}

void ModuleVolume::OnShutdown()
{
	m_scene->UnregisterEntitiesByModule(this);
}

void ModuleVolume::OnDrawMenu()
{
	//if (ImGui::BeginMenu("Volume"))
	//{
	//	ImGui::EndMenu();
	//}
}

void ModuleVolume::OnDrawUI()
{
	/*ImGui::Begin("Volume");
	if (ImGui::ListBox("Volumes", &m_selected_region, (const char**)m_region_list, m_region_list_size, 5)) {
		BuildRegionModels();
	}

	if (ImGui::Button("Add")) {
		Region t;
		t.area_type = RegionTypeWater;
		auto &camera_loc = m_scene->GetCameraLoc();
		t.pos = glm::vec3(camera_loc.z, camera_loc.x, camera_loc.y);
		t.rot = glm::vec3(0.0f);
		t.scale = glm::vec3(1.0f);
		t.extents = glm::vec3(10.0f);
		t.obb = OrientedBoundingBox(t.pos, t.rot, t.scale, t.extents);

		m_regions.push_back(t);
		BuildRegionList();
		BuildRegionModels();

		m_modified = true;
	}

	ImGui::SameLine();

	if (ImGui::Button("Delete")) {
		if (m_selected_region != -1) {
			m_regions.erase(m_regions.begin() + m_selected_region);
			BuildRegionList();
			BuildRegionModels();

			m_selected_region = -1;
			m_modified = true;
		}
	}

	ImGui::Separator();
	if (ImGui::Button("Reset All")) {
		m_regions = m_regions_orig;
		BuildRegionList();
		BuildRegionModels();

		m_selected_region = -1;
		m_modified = true;
	}

	ImGui::Separator();
	if (ImGui::Button("Create region from water map approx")) {
		BuildFromWatermap(m_scene->GetCameraLoc());
	}

	if (m_selected_region >= 0 && m_selected_region < m_region_list_size) {
		auto &region = m_regions[m_selected_region];
		ImGui::Text("Selected Region: %d", m_selected_region);
		bool needs_update = false;

		const char* region_identifiers[] = { "Normal", "Water", "Lava", "ZoneLine", "PVP", "Slime", "Ice", "V Water", "Generic Area", "Prefer Pathing", "Disable NavigationMesh" };
		if (ImGui::Combo("Area type", (int*)&region.area_type, region_identifiers, 11)) {
			m_modified = true;
		}

		if (ImGui::DragFloat("X", &region.pos.x)) {
			m_modified = true;
			needs_update = true;
		}

		if (ImGui::DragFloat("Y", &region.pos.y)) {
			m_modified = true;
			needs_update = true;
		}

		if (ImGui::DragFloat("Z", &region.pos.z)) {
			m_modified = true;
			needs_update = true;
		}

		ImGui::Separator();

		if (ImGui::DragFloat("X rot", &region.rot.x)) {
			m_modified = true;
			needs_update = true;
		}

		if (ImGui::DragFloat("Y rot", &region.rot.y)) {
			m_modified = true;
			needs_update = true;
		}

		if (ImGui::DragFloat("Z rot", &region.rot.z)) {
			m_modified = true;
			needs_update = true;
		}

		ImGui::Separator();

		if (ImGui::DragFloat("X ext", &region.extents.x)) {
			m_modified = true;
			needs_update = true;
		}

		if (ImGui::DragFloat("Y ext", &region.extents.y)) {
			m_modified = true;
			needs_update = true;
		}

		if (ImGui::DragFloat("Z ext", &region.extents.z)) {
			m_modified = true;
			needs_update = true;
		}

		if (needs_update) {
			region.obb = OrientedBoundingBox(region.pos, region.rot, region.scale, region.extents);
			BuildRegionModels();
		}
	}
	ImGui::End();*/
}

void ModuleVolume::OnDrawOptions()
{
	//ImGui::Separator();
	//ImGui::Text("Volume");
	//if (ImGui::Checkbox("Render Volumes", &m_render_volume)) {
	//	if (m_render_volume) {
	//		m_scene->RegisterEntity(this, m_volume_entity.get(), true);
	//	}
	//	else {
	//		m_scene->UnregisterEntity(this, m_volume_entity.get());
	//	}
	//}
	//ImGui::Separator();
}

void ModuleVolume::OnSceneLoad(const char *zone_name)
{
	//m_modified = false;
	//if (!LoadVolumes("save/")) {
	//	LoadVolumes("maps/water/");
	//}
	//BuildRegionList();
	//BuildRegionModels();
	//if (m_render_volume) {
	//	m_scene->RegisterEntity(this, m_volume_entity.get());
	//}
}

void ModuleVolume::OnSuspend()
{
}

void ModuleVolume::OnResume()
{
}

bool ModuleVolume::HasWork()
{
	return false;
}

bool ModuleVolume::CanSave()
{
	return m_modified;
}

void ModuleVolume::Save()
{
	//if (CanSave()) {
	//	std::string filename = "save/" + m_scene->GetZoneName() + ".wtr";
	//	FILE *f = fopen(filename.c_str(), "wb");
	//	if (f) {
	//		fwrite("EQEMUWATER", 10, 1, f);
	//		
	//		uint32_t version = 2;
	//		fwrite(&version, sizeof(uint32_t), 1, f);
	//
	//		uint32_t region_count = (uint32_t)m_regions.size();
	//		fwrite(&region_count, sizeof(uint32_t), 1, f);
	//		for (auto &region : m_regions) {
	//			uint32_t region_type = (uint32_t)region.area_type;
	//			fwrite(&region_type, sizeof(uint32_t), 1, f);
	//			fwrite(&region.pos.x, sizeof(float), 1, f);
	//			fwrite(&region.pos.y, sizeof(float), 1, f);
	//			fwrite(&region.pos.z, sizeof(float), 1, f);
	//			fwrite(&region.rot.x, sizeof(float), 1, f);
	//			fwrite(&region.rot.y, sizeof(float), 1, f);
	//			fwrite(&region.rot.z, sizeof(float), 1, f);
	//			fwrite(&region.scale.x, sizeof(float), 1, f);
	//			fwrite(&region.scale.y, sizeof(float), 1, f);
	//			fwrite(&region.scale.z, sizeof(float), 1, f);
	//			fwrite(&region.extents.x, sizeof(float), 1, f);
	//			fwrite(&region.extents.y, sizeof(float), 1, f);
	//			fwrite(&region.extents.z, sizeof(float), 1, f);
	//		}
	//		fclose(f);
	//		m_modified = false;
	//		m_regions_orig = m_regions;
	//
	//		WaterMap *wm = WaterMap::LoadWaterMapfile("save/", m_scene->GetZoneName());
	//		if (wm) {
	//			m_scene->GetZonePhysics()->SetWaterMap(wm);
	//		}
	//	}
	//}
}

void ModuleVolume::OnHotkey(int ident)
{
	//switch (ident) {
	//	case HotkeyXNeg:
	//		break;
	//	case HotkeyXPos:
	//		break;
	//	case HotkeyZNeg:
	//		break;
	//	case HotkeyZPos:
	//		break;
	//	case HotkeyYNeg:
	//		break;
	//	case HotkeyYPos:
	//		break;
	//	case HotkeyMode:
	//		break;
	//	default:
	//		break;
	//}
}

void ModuleVolume::OnClick(int mouse_button, const glm::vec3 *collide_hit, const glm::vec3 *non_collide_hit, const glm::vec3 *select_hit, Entity *selected)
{
}
/*
bool ModuleVolume::LoadVolumes(const char *dir)
{
	m_regions.clear();
	m_regions_orig.clear();
	std::string filename = dir + m_scene->GetZoneName() + ".wtr";
	FILE *f = fopen(filename.c_str(), "rb");
	if (f) {

		char magic[10];
		uint32_t version;
		if (fread(magic, 10, 1, f) != 1) {
			fclose(f);
			return false;
		}

		if (strncmp(magic, "EQEMUWATER", 10)) {
			fclose(f);
			return false;
		}

		if (fread(&version, sizeof(version), 1, f) != 1) {
			fclose(f);
			return false;
		}

		if (version == 2) {
			uint32_t region_count;
			if (fread(&region_count, sizeof(region_count), 1, f) != 1) {
				fclose(f);
				return false;
			}

			for (uint32_t i = 0; i < region_count; ++i) {
				uint32_t region_type;
				float x;
				float y;
				float z;
				float x_rot;
				float y_rot;
				float z_rot;
				float x_scale;
				float y_scale;
				float z_scale;
				float x_extent;
				float y_extent;
				float z_extent;

				if (fread(&region_type, sizeof(region_type), 1, f) != 1) {
					fclose(f);
					return false;
				}

				if (fread(&x, sizeof(x), 1, f) != 1) {
					fclose(f);
					return false;
				}

				if (fread(&y, sizeof(y), 1, f) != 1) {
					fclose(f);
					return false;
				}

				if (fread(&z, sizeof(z), 1, f) != 1) {
					fclose(f);
					return false;
				}

				if (fread(&x_rot, sizeof(x_rot), 1, f) != 1) {
					fclose(f);
					return false;
				}

				if (fread(&y_rot, sizeof(y_rot), 1, f) != 1) {
					fclose(f);
					return false;
				}

				if (fread(&z_rot, sizeof(z_rot), 1, f) != 1) {
					fclose(f);
					return false;
				}

				if (fread(&x_scale, sizeof(x_scale), 1, f) != 1) {
					fclose(f);
					return false;
				}

				if (fread(&y_scale, sizeof(y_scale), 1, f) != 1) {
					fclose(f);
					return false;
				}

				if (fread(&z_scale, sizeof(z_scale), 1, f) != 1) {
					fclose(f);
					return false;
				}

				if (fread(&x_extent, sizeof(x_extent), 1, f) != 1) {
					fclose(f);
					return false;
				}

				if (fread(&y_extent, sizeof(y_extent), 1, f) != 1) {
					fclose(f);
					return false;
				}

				if (fread(&z_extent, sizeof(z_extent), 1, f) != 1) {
					fclose(f);
					return false;
				}

				Region r;
				r.area_type = region_type;
				r.pos = glm::vec3(x, y, z);
				r.rot = glm::vec3(x_rot, y_rot, z_rot);
				r.scale = glm::vec3(x_scale, y_scale, z_scale);
				r.extents = glm::vec3(x_extent, y_extent, z_extent);
				r.obb = OrientedBoundingBox(r.pos, r.rot, r.scale, r.extents);
				m_regions.push_back(r);
				m_regions_orig.push_back(r);
			}

			fclose(f);
			return true;
		}
		else {
			fclose(f);
			return false;
		}
	}

	return false;
}

void ModuleVolume::FreeRegionList()
{
	if (m_region_list) {
		for (int i = 0; i < m_region_list_size; ++i) {
			delete[] m_region_list[i];
		}

		delete[] m_region_list;
		m_region_list = nullptr;
		m_region_list_size = 0;
		m_selected_region = -1;
	}
}

void ModuleVolume::BuildRegionList()
{
	FreeRegionList();

	m_region_list_size = (int)m_regions.size();
	if (m_region_list_size > 0) {
		m_region_list = new char*[m_region_list_size];
		for (int i = 0; i < m_region_list_size; ++i) {
			m_region_list[i] = new char[16];
			sprintf(m_region_list[i], "Region %u", i);
		}
	}
}

void ModuleVolume::BuildRegionModels()
{
	m_volume_entity->Clear();
	auto &inds = m_volume_entity->GetInds();
	auto &verts = m_volume_entity->GetVerts();
	auto &colors = m_volume_entity->GetVertColors();

	for (size_t i = 0; i < m_regions.size(); ++i) {
		glm::vec3 color;
		if (i == m_selected_region) {
			color = glm::vec3(1.0f, 1.0f, 0.0f);
		}
		else {
			color = glm::vec3(0.0f, 0.0f, 1.0f);
		}

		auto &region = m_regions[i];

		float min_x = region.obb.GetMinX();
		float min_y = region.obb.GetMinY();
		float min_z = region.obb.GetMinZ();
		float max_x = region.obb.GetMaxX();
		float max_y = region.obb.GetMaxY();
		float max_z = region.obb.GetMaxZ();

		glm::vec4 v1(min_x, max_y, min_z, 1.0f);
		glm::vec4 v2(min_x, max_y, max_z, 1.0f);
		glm::vec4 v3(max_x, max_y, max_z, 1.0f);
		glm::vec4 v4(max_x, max_y, min_z, 1.0f);
		glm::vec4 v5(min_x, min_y, min_z, 1.0f);
		glm::vec4 v6(min_x, min_y, max_z, 1.0f);
		glm::vec4 v7(max_x, min_y, max_z, 1.0f);
		glm::vec4 v8(max_x, min_y, min_z, 1.0f);

		v1 = region.obb.GetTransformation() * v1;
		v2 = region.obb.GetTransformation() * v2;
		v3 = region.obb.GetTransformation() * v3;
		v4 = region.obb.GetTransformation() * v4;
		v5 = region.obb.GetTransformation() * v5;
		v6 = region.obb.GetTransformation() * v6;
		v7 = region.obb.GetTransformation() * v7;
		v8 = region.obb.GetTransformation() * v8;

		uint32_t current_index = (uint32_t)verts.size();
		verts.push_back(glm::vec3(v1.y, v1.z, v1.x));
		verts.push_back(glm::vec3(v2.y, v2.z, v2.x));
		verts.push_back(glm::vec3(v3.y, v3.z, v3.x));
		verts.push_back(glm::vec3(v4.y, v4.z, v4.x));
		verts.push_back(glm::vec3(v5.y, v5.z, v5.x));
		verts.push_back(glm::vec3(v6.y, v6.z, v6.x));
		verts.push_back(glm::vec3(v7.y, v7.z, v7.x));
		verts.push_back(glm::vec3(v8.y, v8.z, v8.x));

		colors.push_back(color);
		colors.push_back(color);
		colors.push_back(color);
		colors.push_back(color);
		colors.push_back(color);
		colors.push_back(color);
		colors.push_back(color);
		colors.push_back(color);

		//top
		inds.push_back(current_index + 0);
		inds.push_back(current_index + 1);
		inds.push_back(current_index + 2);
		inds.push_back(current_index + 2);
		inds.push_back(current_index + 3);
		inds.push_back(current_index + 0);

		inds.push_back(current_index + 2);
		inds.push_back(current_index + 1);
		inds.push_back(current_index + 0);
		inds.push_back(current_index + 0);
		inds.push_back(current_index + 3);
		inds.push_back(current_index + 2);

		//back
		inds.push_back(current_index + 1);
		inds.push_back(current_index + 2);
		inds.push_back(current_index + 6);
		inds.push_back(current_index + 6);
		inds.push_back(current_index + 5);
		inds.push_back(current_index + 1);

		inds.push_back(current_index + 6);
		inds.push_back(current_index + 2);
		inds.push_back(current_index + 1);
		inds.push_back(current_index + 1);
		inds.push_back(current_index + 5);
		inds.push_back(current_index + 6);

		//bottom
		inds.push_back(current_index + 4);
		inds.push_back(current_index + 5);
		inds.push_back(current_index + 6);
		inds.push_back(current_index + 6);
		inds.push_back(current_index + 7);
		inds.push_back(current_index + 4);

		inds.push_back(current_index + 6);
		inds.push_back(current_index + 5);
		inds.push_back(current_index + 4);
		inds.push_back(current_index + 4);
		inds.push_back(current_index + 7);
		inds.push_back(current_index + 6);

		//front
		inds.push_back(current_index + 0);
		inds.push_back(current_index + 3);
		inds.push_back(current_index + 7);
		inds.push_back(current_index + 7);
		inds.push_back(current_index + 4);
		inds.push_back(current_index + 0);

		inds.push_back(current_index + 7);
		inds.push_back(current_index + 3);
		inds.push_back(current_index + 0);
		inds.push_back(current_index + 0);
		inds.push_back(current_index + 4);
		inds.push_back(current_index + 7);

		//left
		inds.push_back(current_index + 0);
		inds.push_back(current_index + 1);
		inds.push_back(current_index + 5);
		inds.push_back(current_index + 5);
		inds.push_back(current_index + 4);
		inds.push_back(current_index + 0);

		inds.push_back(current_index + 5);
		inds.push_back(current_index + 1);
		inds.push_back(current_index + 0);
		inds.push_back(current_index + 0);
		inds.push_back(current_index + 4);
		inds.push_back(current_index + 5);

		//right
		inds.push_back(current_index + 3);
		inds.push_back(current_index + 2);
		inds.push_back(current_index + 6);
		inds.push_back(current_index + 6);
		inds.push_back(current_index + 7);
		inds.push_back(current_index + 3);

		inds.push_back(current_index + 6);
		inds.push_back(current_index + 2);
		inds.push_back(current_index + 3);
		inds.push_back(current_index + 3);
		inds.push_back(current_index + 7);
		inds.push_back(current_index + 6);
	}

	m_volume_entity->Update();
}

bool RegionPointEqual(const glm::vec4 &a, const glm::vec4 &b) {
	const float eps = 0.01f;
	const float n_eps = -0.01f;

	float x = a.x - b.x;
	float y = a.y - b.y;
	float z = a.z - b.z;

	if (x > eps || x < n_eps) {
		return false;
	}

	if (y > eps || y < n_eps) {
		return false;
	}

	if (z > eps || z < n_eps) {
		return false;
	}

	return true;
}

void ModuleVolume::BuildFromWatermap(const glm::vec3 &pos)
{
	auto physics = m_scene->GetZonePhysics();
	if (!physics) {
		return;
	}
	
	auto region_type = physics->ReturnRegionType(pos);
	if (region_type == RegionTypeNormal || region_type == RegionTypeUntagged || region_type == RegionTypeUnsupported) {
		return;
	}

	glm::vec3 new_region_min = pos;
	glm::vec3 new_region_max = pos;
	
	//X+
	const float step_size = 0.1f;
	for (float x = pos.x; x < 30000.0f; x += step_size) {
		auto temp_region_type = physics->ReturnRegionType(glm::vec3(x, pos.y, pos.z));
		if (temp_region_type != region_type) {
			break;
		}
	
		new_region_max.x += step_size;
	}
	
	//X-
	for (float x = pos.x; x > -30000.0f; x -= step_size) {
		auto temp_region_type = physics->ReturnRegionType(glm::vec3(x, pos.y, pos.z));
		if (temp_region_type != region_type) {
			break;
		}
	
		new_region_min.x -= step_size;
	}
	
	//Y+
	for (float y = pos.y; y < 30000.0f; y += step_size) {
		auto temp_region_type = physics->ReturnRegionType(glm::vec3(pos.x, y, pos.z));
		if (temp_region_type != region_type) {
			break;
		}
	
		new_region_max.y += step_size;
	}
	
	//Y-
	for (float y = pos.y; y > -30000.0f; y -= step_size) {
		auto temp_region_type = physics->ReturnRegionType(glm::vec3(pos.x, y, pos.z));
		if (temp_region_type != region_type) {
			break;
		}
	
		new_region_min.y -= step_size;
	}
	
	//Z+
	for (float z = pos.z; z < 30000.0f; z += step_size) {
		auto temp_region_type = physics->ReturnRegionType(glm::vec3(pos.x, pos.y, z));
		if (temp_region_type != region_type) {
			break;
		}
	
		new_region_max.z += step_size;
	}
	
	//Z-
	for (float z = pos.z; z > -30000.0f; z -= step_size) {
		auto temp_region_type = physics->ReturnRegionType(glm::vec3(pos.x, pos.y, z));
		if (temp_region_type != region_type) {
			break;
		}
	
		new_region_min.z -= step_size;
	}

	Region t;
	t.area_type = (uint32_t)region_type;
	t.pos = glm::vec3((new_region_max.z + new_region_min.z) / 2.0f, (new_region_max.x + new_region_min.x) / 2.0f, (new_region_max.y + new_region_min.y) / 2.0f);
	t.scale = glm::vec3(1.0f);
	t.extents = glm::vec3((new_region_max.z - new_region_min.z) / 2.0f, (new_region_max.x - new_region_min.x) / 2.0f, (new_region_max.y - new_region_min.y) / 2.0f); // calc extents
	t.obb = OrientedBoundingBox(t.pos, t.rot, t.scale, t.extents);
	m_regions.push_back(t);
	
	m_modified = true;
	BuildRegionList();
	BuildRegionModels();
}
*/