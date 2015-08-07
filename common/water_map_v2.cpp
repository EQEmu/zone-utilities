#include "water_map_v2.h"

WaterMapV2::WaterMapV2() {
}

WaterMapV2::~WaterMapV2() {
}

WaterRegionType WaterMapV2::ReturnRegionType(float y, float x, float z) const {
	size_t sz = regions.size();
	for(size_t i = 0; i < sz; ++i) {
		auto const &region = regions[i];
		if (region.second.ContainsPoint(glm::vec3(x, y, z))) {
			return region.first;
		}
	}
	return RegionTypeNormal;
}

bool WaterMapV2::InWater(float y, float x, float z) const {
	return ReturnRegionType(y, x, z) == RegionTypeWater;
}

bool WaterMapV2::InVWater(float y, float x, float z) const {
	return ReturnRegionType(y, x, z) == RegionTypeVWater;
}

bool WaterMapV2::InLava(float y, float x, float z) const {
	return ReturnRegionType(y, x, z) == RegionTypeLava;
}

bool WaterMapV2::InLiquid(float y, float x, float z) const {
	return InWater(y, x, z) || InLava(y, x, z);
}

bool WaterMapV2::Load(FILE *fp) {
	uint32_t region_count;
	if (fread(&region_count, sizeof(region_count), 1, fp) != 1) {
		return false;
	}

	for(uint32_t i = 0; i < region_count; ++i) {
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

		if (fread(&region_type, sizeof(region_type), 1, fp) != 1) {
			return false;
		}

		if (fread(&x, sizeof(x), 1, fp) != 1) {
			return false;
		}

		if (fread(&y, sizeof(y), 1, fp) != 1) {
			return false;
		}

		if (fread(&z, sizeof(z), 1, fp) != 1) {
			return false;
		}

		if (fread(&x_rot, sizeof(x_rot), 1, fp) != 1) {
			return false;
		}

		if (fread(&y_rot, sizeof(y_rot), 1, fp) != 1) {
			return false;
		}

		if (fread(&z_rot, sizeof(z_rot), 1, fp) != 1) {
			return false;
		}

		if (fread(&x_scale, sizeof(x_scale), 1, fp) != 1) {
			return false;
		}

		if (fread(&y_scale, sizeof(y_scale), 1, fp) != 1) {
			return false;
		}

		if (fread(&z_scale, sizeof(z_scale), 1, fp) != 1) {
			return false;
		}

		if (fread(&x_extent, sizeof(x_extent), 1, fp) != 1) {
			return false;
		}

		if (fread(&y_extent, sizeof(y_extent), 1, fp) != 1) {
			return false;
		}

		if (fread(&z_extent, sizeof(z_extent), 1, fp) != 1) {
			return false;
		}

		regions.push_back(std::make_pair((WaterRegionType)region_type, 
			OrientedBoundingBox(glm::vec3(x, y, z), glm::vec3(x_rot, y_rot, z_rot), glm::vec3(x_scale, y_scale, z_scale), glm::vec3(x_extent, y_extent, z_extent))));
	}

	return true;
}

void WaterMapV2::CreateMeshFrom(std::vector<glm::vec3> &verts, std::vector<unsigned int> &inds) {
	verts.clear();
	inds.clear();

	for(auto &region : regions) {
		float min_x = region.second.GetMinX();
		float min_y = region.second.GetMinY();
		float min_z = region.second.GetMinZ();
		float max_x = region.second.GetMaxX();
		float max_y = region.second.GetMaxY();
		float max_z = region.second.GetMaxZ();

		glm::vec4 v1(min_x, max_y, min_z, 1.0f);
		glm::vec4 v2(min_x, max_y, max_z, 1.0f);
		glm::vec4 v3(max_x, max_y, max_z, 1.0f);
		glm::vec4 v4(max_x, max_y, min_z, 1.0f);
		glm::vec4 v5(min_x, min_y, min_z, 1.0f);
		glm::vec4 v6(min_x, min_y, max_z, 1.0f);
		glm::vec4 v7(max_x, min_y, max_z, 1.0f);
		glm::vec4 v8(max_x, min_y, min_z, 1.0f);

		v1 = region.second.GetTransformation() * v1;
		v2 = region.second.GetTransformation() * v2;
		v3 = region.second.GetTransformation() * v3;
		v4 = region.second.GetTransformation() * v4;
		v5 = region.second.GetTransformation() * v5;
		v6 = region.second.GetTransformation() * v6;
		v7 = region.second.GetTransformation() * v7;
		v8 = region.second.GetTransformation() * v8;

		uint32_t current_index = (uint32_t)verts.size();
		verts.push_back(glm::vec3(v1.y, v1.z, v1.x));
		verts.push_back(glm::vec3(v2.y, v2.z, v2.x));
		verts.push_back(glm::vec3(v3.y, v3.z, v3.x));
		verts.push_back(glm::vec3(v4.y, v4.z, v4.x));
		verts.push_back(glm::vec3(v5.y, v5.z, v5.x));
		verts.push_back(glm::vec3(v6.y, v6.z, v6.x));
		verts.push_back(glm::vec3(v7.y, v7.z, v7.x));
		verts.push_back(glm::vec3(v8.y, v8.z, v8.x));

		//top
		inds.push_back(current_index + 0);
		inds.push_back(current_index + 1);
		inds.push_back(current_index + 2);
		inds.push_back(current_index + 2);
		inds.push_back(current_index + 3);
		inds.push_back(current_index + 0);

		//back
		inds.push_back(current_index + 1);
		inds.push_back(current_index + 2);
		inds.push_back(current_index + 6);
		inds.push_back(current_index + 6);
		inds.push_back(current_index + 5);
		inds.push_back(current_index + 1);

		//bottom
		inds.push_back(current_index + 4);
		inds.push_back(current_index + 5);
		inds.push_back(current_index + 6);
		inds.push_back(current_index + 6);
		inds.push_back(current_index + 7);
		inds.push_back(current_index + 4);

		//front
		inds.push_back(current_index + 0);
		inds.push_back(current_index + 3);
		inds.push_back(current_index + 7);
		inds.push_back(current_index + 7);
		inds.push_back(current_index + 4);
		inds.push_back(current_index + 0);

		//left
		inds.push_back(current_index + 0);
		inds.push_back(current_index + 1);
		inds.push_back(current_index + 5);
		inds.push_back(current_index + 5);
		inds.push_back(current_index + 4);
		inds.push_back(current_index + 0);

		//right
		inds.push_back(current_index + 3);
		inds.push_back(current_index + 2);
		inds.push_back(current_index + 6);
		inds.push_back(current_index + 6);
		inds.push_back(current_index + 7);
		inds.push_back(current_index + 3);
	}
}