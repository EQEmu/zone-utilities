#include "water_map.h"
#include <string.h>

void BSPMarkRegion(std::shared_ptr<EQEmu::S3D::BSPTree> tree, uint32_t node_number, uint32_t region, int32_t region_type);

WaterMap::WaterMap() {
}

WaterMap::~WaterMap() {
}

bool WaterMap::BuildAndWrite(std::string zone_name) {
	bool status = BuildAndWriteS3D(zone_name);
	if(status) {
		return true;
	}
	
	//try eqg3 and eqg4 here
	
	return false;
}

bool WaterMap::BuildAndWriteS3D(std::string zone_name) {
	EQEmu::S3DLoader s3d;
	std::vector<EQEmu::S3D::WLDFragment> zone_frags;
	if (!s3d.ParseWLDFile(zone_name + ".s3d", zone_name + ".wld", zone_frags)) {
		return false;
	}

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
		if (fwrite(&version, sizeof(version), 1, f) != 1) {
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

void BSPMarkRegion(std::shared_ptr<EQEmu::S3D::BSPTree> tree, uint32_t node_number, uint32_t region, int32_t region_type) {
	if (node_number < 1) {
		return;
	}

	auto &nodes = tree->GetNodes();
	if ((nodes[node_number - 1].left == 0) && (nodes[node_number - 1].right == 0))  {
		if (nodes[node_number - 1].region == region) {
			nodes[node_number - 1].special = region_type;
		}
	}

	if (nodes[node_number - 1].left != 0) {
		BSPMarkRegion(tree, nodes[node_number - 1].left, region, region_type);

	}

	if (nodes[node_number - 1].right != 0) {
		BSPMarkRegion(tree, nodes[node_number - 1].right, region, region_type);
	}
}