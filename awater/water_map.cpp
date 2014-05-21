#include "water_map.h"
#include "log_macros.h"
#include "s3d_loader.h"
#include "eqg_loader.h"
#include "eqg_v4_loader.h"
#include <string.h>

uint32_t BSPMarkRegion(std::shared_ptr<EQEmu::S3D::BSPTree> tree, uint32_t node_number, uint32_t region, int32_t region_type);

WaterMap::WaterMap() {
}

WaterMap::~WaterMap() {
}

bool WaterMap::BuildAndWrite(std::string zone_name) {
	if (BuildAndWriteEQG(zone_name)) {
		return true;
	}

	if (BuildAndWriteEQG4(zone_name)) {
		return true;
	}
	
	if(BuildAndWriteS3D(zone_name)) {
		return true;
	}
	
	return false;
}

bool WaterMap::BuildAndWriteS3D(std::string zone_name) {
	eqLogMessage(LogTrace, "Loading %s.s3d", zone_name.c_str());

	EQEmu::S3DLoader s3d;
	std::vector<EQEmu::S3D::WLDFragment> zone_frags;
	if (!s3d.ParseWLDFile(zone_name + ".s3d", zone_name + ".wld", zone_frags)) {
		return false;
	}

	eqLogMessage(LogTrace, "Loaded %s.s3d.", zone_name.c_str());
	std::shared_ptr<EQEmu::S3D::BSPTree> tree;
	for(uint32_t i = 0; i < zone_frags.size(); ++i) {
		if(zone_frags[i].type == 0x21) {
			EQEmu::S3D::WLDFragment21 &frag = reinterpret_cast<EQEmu::S3D::WLDFragment21&>(zone_frags[i]);
			tree = frag.GetData();
		}
		else if (zone_frags[i].type == 0x29) {
			if(!tree)
				continue;

			EQEmu::S3D::WLDFragment29 &frag = reinterpret_cast<EQEmu::S3D::WLDFragment29&>(zone_frags[i]);
			auto region = frag.GetData();

			auto regions = region->GetRegions();
			WaterMapRegionType region_type = RegionTypeUntagged;

			eqLogMessage(LogTrace, "Processing region %s for s3d.", region->GetName().c_str());

			if (!strncmp(region->GetName().c_str(), "WT", 2)) {
				region_type = RegionTypeWater;
			}
			else if (!strncmp(region->GetName().c_str(), "LA", 2)) {
				region_type = RegionTypeLava;
			}
			else if (!strncmp(region->GetName().c_str(), "DRNTP", 5)) {
				region_type = RegionTypeZoneLine;
			}
			else if (!strncmp(region->GetName().c_str(), "DRP_", 4)) {
				region_type = RegionTypePVP;
			}
			else if (!strncmp(region->GetName().c_str(), "SL", 2)) {
				region_type = RegionTypeSlime;
			}
			else if (!strncmp(region->GetName().c_str(), "DRN", 3)) {
				region_type = RegionTypeIce;
			}
			else if (!strncmp(region->GetName().c_str(), "VWA", 3)) {
				region_type = RegionTypeVWater;
			}

			for(size_t j = 0; j < regions.size(); ++j) {
				BSPMarkRegion(tree, 1, regions[j] + 1, (int32_t)region_type);
			}
		}
	}

	if (!tree) {
		return false;
	}

	eqLogMessage(LogTrace, "Writing v1 map file out.");
	std::string filename = zone_name + ".wtr";
	FILE *f = fopen(filename.c_str(), "wb");
	if(f) {
		char *magic = "EQEMUWATER";
		uint32_t version = 1;

		if (fwrite(magic, strlen(magic), 1, f) != 1) {
			fclose(f);
			return false;
		}

		if (fwrite(&version, sizeof(version), 1, f) != 1) {
			fclose(f);
			return false;
		}

		uint32_t bsp_size = (uint32_t)tree->GetNodes().size();
		if (fwrite(&bsp_size, sizeof(bsp_size), 1, f) != 1) {
			fclose(f);
			return false;
		}

		for(uint32_t i = 0; i < bsp_size; ++i) {
			uint32_t number = tree->GetNodes()[i].number;
			float normal1 = tree->GetNodes()[i].normal[0];
			float normal2 = tree->GetNodes()[i].normal[1];
			float normal3 = tree->GetNodes()[i].normal[2];
			float split_dist = tree->GetNodes()[i].split_dist;
			uint32_t region = tree->GetNodes()[i].region;
			int32_t special = tree->GetNodes()[i].special;
			uint32_t left = tree->GetNodes()[i].left;
			uint32_t right = tree->GetNodes()[i].right;

			if (fwrite(&number, sizeof(number), 1, f) != 1) {
				fclose(f);
				return false;
			}

			if (fwrite(&normal1, sizeof(normal1), 1, f) != 1) {
				fclose(f);
				return false;
			}

			if (fwrite(&normal2, sizeof(normal2), 1, f) != 1) {
				fclose(f);
				return false;
			}

			if (fwrite(&normal3, sizeof(normal3), 1, f) != 1) {
				fclose(f);
				return false;
			}

			if (fwrite(&split_dist, sizeof(split_dist), 1, f) != 1) {
				fclose(f);
				return false;
			}

			if (fwrite(&region, sizeof(region), 1, f) != 1) {
				fclose(f);
				return false;
			}

			if (fwrite(&special, sizeof(special), 1, f) != 1) {
				fclose(f);
				return false;
			}

			if (fwrite(&left, sizeof(left), 1, f) != 1) {
				fclose(f);
				return false;
			}

			if (fwrite(&right, sizeof(right), 1, f) != 1) {
				fclose(f);
				return false;
			}

		}

		fclose(f);
		return true;
	} else {
		return false;
	}

	return false;
}

bool WaterMap::BuildAndWriteEQG(std::string zone_name) {
	eqLogMessage(LogTrace, "Loading standard eqg %s.eqg", zone_name.c_str());

	EQEmu::EQGLoader eqg;
	std::vector<std::shared_ptr<EQEmu::EQG::Geometry>> models;
	std::vector<std::shared_ptr<EQEmu::Placeable>> placables;
	std::vector<std::shared_ptr<EQEmu::EQG::Region>> regions;
	std::vector<std::shared_ptr<EQEmu::Light>> lights;
	if(!eqg.Load(zone_name, models, placables, regions, lights)) {
		return false;
	}

	eqLogMessage(LogTrace, "Loaded standard eqg %s.eqg", zone_name.c_str());
	std::string filename = zone_name + ".wtr";
	FILE *f = fopen(filename.c_str(), "wb");
	if (f) {
		char *magic = "EQEMUWATER";
		uint32_t version = 2;

		if (fwrite(magic, strlen(magic), 1, f) != 1) {
			fclose(f);
			return false;
		}

		if (fwrite(&version, sizeof(version), 1, f) != 1) {
			fclose(f);
			return false;
		}

		uint32_t region_count = (uint32_t)regions.size();
		if (fwrite(&region_count, sizeof(region_count), 1, f) != 1) {
			fclose(f);
			return false;
		}

		for (uint32_t i = 0; i < region_count; ++i) {
			auto &region = regions[i];

			eqLogMessage(LogTrace, "Writing region %s.", region->GetName().c_str());
			uint32_t region_type = 0;
			float x = region->GetX();
			float y = region->GetY();
			float z = region->GetZ();
			float x_rot = region->GetRotationX();
			float y_rot = region->GetRotationY();
			float z_rot = region->GetRotationZ();
			float x_scale = region->GetScaleX();
			float y_scale = region->GetScaleY();
			float z_scale = region->GetScaleZ();
			float x_extent = region->GetExtentX();
			float y_extent = region->GetExtentY();
			float z_extent = region->GetExtentZ();

			if (region->GetName().length() >= 3) {
				std::string region_code = region->GetName().substr(0, 3);
				if(region_code.compare("AWT") == 0) {
					region_type = RegionTypeWater;
				}
				else if (region_code.compare("ALV") == 0) {
					region_type = RegionTypeLava;
				}
				else if (region_code.compare("APK") == 0) {
					region_type = RegionTypePVP;
				}
				else if (region_code.compare("ATP") == 0) {
					region_type = RegionTypeZoneLine;
				}
				else {
					eqLogMessage(LogWarn, "Unsupported region type %s (%s).", region->GetName().c_str(), region_code.c_str());
					region_type = RegionTypeUnsupported;
				}
			}

			if (fwrite(&region_type, sizeof(region_type), 1, f) != 1) {
				fclose(f);
				return false;
			}

			if (fwrite(&x, sizeof(x), 1, f) != 1) {
				fclose(f);
				return false;
			}

			if (fwrite(&y, sizeof(y), 1, f) != 1) {
				fclose(f);
				return false;
			}

			if (fwrite(&z, sizeof(z), 1, f) != 1) {
				fclose(f);
				return false;
			}

			if (fwrite(&x_rot, sizeof(x_rot), 1, f) != 1) {
				fclose(f);
				return false;
			}

			if (fwrite(&y_rot, sizeof(y_rot), 1, f) != 1) {
				fclose(f);
				return false;
			}

			if (fwrite(&z_rot, sizeof(z_rot), 1, f) != 1) {
				fclose(f);
				return false;
			}

			if (fwrite(&x_scale, sizeof(x_scale), 1, f) != 1) {
				fclose(f);
				return false;
			}

			if (fwrite(&y_scale, sizeof(y_scale), 1, f) != 1) {
				fclose(f);
				return false;
			}

			if (fwrite(&z_scale, sizeof(z_scale), 1, f) != 1) {
				fclose(f);
				return false;
			}

			if (fwrite(&x_extent, sizeof(x_extent), 1, f) != 1) {
				fclose(f);
				return false;
			}

			if (fwrite(&y_extent, sizeof(y_extent), 1, f) != 1) {
				fclose(f);
				return false;
			}

			if (fwrite(&z_extent, sizeof(z_extent), 1, f) != 1) {
				fclose(f);
				return false;
			}

		}

		fclose(f);
		return true;
	}
	else {
		return false;
	}

	return false;
}

bool WaterMap::BuildAndWriteEQG4(std::string zone_name) {
	eqLogMessage(LogTrace, "Loading standard eqg %s.eqg", zone_name.c_str());

	EQEmu::EQG4Loader eqg;
	std::shared_ptr<EQEmu::EQG::Terrain> terrain;
	if (!eqg.Load(zone_name, terrain)) {
		return false;
	}

	eqLogMessage(LogTrace, "Loaded v4 eqg %s.eqg", zone_name.c_str());
	std::string filename = zone_name + ".wtr";
	FILE *f = fopen(filename.c_str(), "wb");
	if (f) {
		char *magic = "EQEMUWATER";
		uint32_t version = 2;

		if (fwrite(magic, strlen(magic), 1, f) != 1) {
			fclose(f);
			return false;
		}

		if (fwrite(&version, sizeof(version), 1, f) != 1) {
			fclose(f);
			return false;
		}

		auto &regions = terrain->GetRegions();
		uint32_t region_count = (uint32_t)regions.size();
		if (fwrite(&region_count, sizeof(region_count), 1, f) != 1) {
			fclose(f);
			return false;
		}

		for (uint32_t i = 0; i < region_count; ++i) {
			auto &region = regions[i];

			eqLogMessage(LogTrace, "Writing region %s.", region->GetName().c_str());
			uint32_t region_type = 0;
			float x = region->GetX();
			float y = region->GetY();
			float z = region->GetZ();
			float x_rot = region->GetRotationX();
			float y_rot = region->GetRotationY();
			float z_rot = region->GetRotationZ();
			float x_scale = region->GetScaleX();
			float y_scale = region->GetScaleY();
			float z_scale = region->GetScaleZ();
			float x_extent = region->GetExtentX();
			float y_extent = region->GetExtentY();
			float z_extent = region->GetExtentZ();

			if (region->GetName().length() >= 3) {
				std::string region_code = region->GetName().substr(0, 3);
				if (region_code.compare("AWT") == 0) {
					region_type = RegionTypeWater;
				}
				else if (region_code.compare("ALV") == 0) {
					region_type = RegionTypeLava;
				}
				else if (region_code.compare("APK") == 0) {
					region_type = RegionTypePVP;
				}
				else if (region_code.compare("ATP") == 0) {
					region_type = RegionTypeZoneLine;
				}
				else {
					eqLogMessage(LogWarn, "Unsupported region type %s (%s).", region->GetName().c_str(), region_code.c_str());
					region_type = RegionTypeUnsupported;
				}
			}

			if (fwrite(&region_type, sizeof(region_type), 1, f) != 1) {
				fclose(f);
				return false;
			}

			if (fwrite(&x, sizeof(x), 1, f) != 1) {
				fclose(f);
				return false;
			}

			if (fwrite(&y, sizeof(y), 1, f) != 1) {
				fclose(f);
				return false;
			}

			if (fwrite(&z, sizeof(z), 1, f) != 1) {
				fclose(f);
				return false;
			}

			if (fwrite(&x_rot, sizeof(x_rot), 1, f) != 1) {
				fclose(f);
				return false;
			}

			if (fwrite(&y_rot, sizeof(y_rot), 1, f) != 1) {
				fclose(f);
				return false;
			}

			if (fwrite(&z_rot, sizeof(z_rot), 1, f) != 1) {
				fclose(f);
				return false;
			}

			if (fwrite(&x_scale, sizeof(x_scale), 1, f) != 1) {
				fclose(f);
				return false;
			}

			if (fwrite(&y_scale, sizeof(y_scale), 1, f) != 1) {
				fclose(f);
				return false;
			}

			if (fwrite(&z_scale, sizeof(z_scale), 1, f) != 1) {
				fclose(f);
				return false;
			}

			if (fwrite(&x_extent, sizeof(x_extent), 1, f) != 1) {
				fclose(f);
				return false;
			}

			if (fwrite(&y_extent, sizeof(y_extent), 1, f) != 1) {
				fclose(f);
				return false;
			}

			if (fwrite(&z_extent, sizeof(z_extent), 1, f) != 1) {
				fclose(f);
				return false;
			}

		}

		fclose(f);
		return true;
	}
	else {
		return false;
	}

	return false;
}

uint32_t BSPMarkRegion(std::shared_ptr<EQEmu::S3D::BSPTree> tree, uint32_t node_number, uint32_t region, int32_t region_type) {
	if (node_number < 1) {
		eqLogMessage(LogError, "BSPMarkRegion was passed a node < 1.");
		return 0;
	}

	auto &nodes = tree->GetNodes();
	if ((nodes[node_number - 1].left == 0) && (nodes[node_number - 1].right == 0))  {
		if (nodes[node_number - 1].region == region) {
			nodes[node_number - 1].special = region_type;
			return node_number;
		}
	}

	uint32_t ret_node = 0;
	if (nodes[node_number - 1].left != 0) {
		ret_node = BSPMarkRegion(tree, nodes[node_number - 1].left, region, region_type);
		if(ret_node != 0)
			return ret_node;
	}

	if (nodes[node_number - 1].right != 0) {
		return BSPMarkRegion(tree, nodes[node_number - 1].right, region, region_type);
	}

	return 0;
}