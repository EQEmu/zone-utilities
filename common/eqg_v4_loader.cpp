#include "eqg_v4_loader.h"
#include <stdio.h>
#include "eqg_structs.h"
#include "safe_alloc.h"
#include "eqg_model_loader.h"
#include "string_util.h"

EQEmu::EQG4Loader::EQG4Loader() {
}

EQEmu::EQG4Loader::~EQG4Loader() {
}

bool EQEmu::EQG4Loader::Load(std::string file, std::shared_ptr<Terrain> &terrain)
{
	EQEmu::PFS::Archive archive;
	if (!archive.Open(file + ".eqg")) {
		return false;
	}

	std::vector<char> zon;
	bool zon_found = true;
	std::list<std::string> files;
	archive.GetFilenames("zon", files);

	if (files.size() == 0) {
		if (!GetZon(file + ".zon", zon)) {
			zon_found = false;
		}
	}
	else {
		auto iter = files.begin();
		if (!archive.Get(*iter, zon)) {
			zon_found = false;
		}
	}

	if (!zon_found)
		return false;

	ZoneOptions opts;
	if (!ParseZon(archive, zon, opts)) {
		return false;
	}

	terrain.reset(new Terrain());
	if(!ParseZoneDat(archive, opts, terrain)) {
		return false;
	}

	return true;
}

bool EQEmu::EQG4Loader::ParseZoneDat(EQEmu::PFS::Archive &archive, ZoneOptions &opts, std::shared_ptr<Terrain> &terrain) {
	std::string filename = opts.name + ".dat";

	
	std::vector<char> buffer;
	if(!archive.Get(filename, buffer)) {
		return false;
	}

	uint32_t idx = 0;

	SafeVarAllocParse(v4_zone_dat_header, header);

	SafeStringAllocParse(base_tile_texture);
	SafeVarAllocParse(uint32_t, tile_count);

	float lat_range = float(opts.max_lat - opts.min_lat);
	float lng_range = float(opts.max_lng - opts.min_lng);

	float zone_min_x = (float)opts.min_lat * (float)opts.quads_per_tile * (float)opts.units_per_vert;
	float zone_max_x = (float)(opts.max_lat + 1) * (float)opts.quads_per_tile * (float)opts.units_per_vert;

	float zone_min_y = (float)opts.min_lng * (float)opts.quads_per_tile * opts.units_per_vert;
	float zone_max_y = (float)(opts.max_lng + 1) * (float)opts.quads_per_tile * opts.units_per_vert;

	uint32_t quad_count = (opts.quads_per_tile * opts.quads_per_tile);
	uint32_t vert_count = ((opts.quads_per_tile + 1) * (opts.quads_per_tile + 1));

	terrain->SetQuadsPerTile(opts.quads_per_tile);
	terrain->SetUnitsPerVertex(opts.units_per_vert);

	for(uint32_t i = 0; i < tile_count; ++i) {
		std::shared_ptr<TerrainTile> tile(new TerrainTile());
		terrain->AddTile(tile);

		SafeVarAllocParse(int32_t, tile_lng);
		SafeVarAllocParse(int32_t, tile_lat);
		SafeVarAllocParse(int32_t, tile_unk);

		float tile_start_y = zone_min_y + (tile_lng - 100000 - opts.min_lng) * opts.units_per_vert * opts.quads_per_tile;
		float tile_start_x = zone_min_x + (tile_lat - 100000 - opts.min_lat) * opts.units_per_vert * opts.quads_per_tile;
	
		bool floats_all_the_same = true;
		tile->GetFloats().resize(vert_count);
		tile->GetColors().resize(vert_count);
		tile->GetColors2().resize(vert_count);
		tile->GetFlags().resize(quad_count);

		for (uint32_t j = 0; j < vert_count; ++j) {
			SafeVarAllocParse(float, t);
			tile->GetFloats()[j] = t;

			if ((j > 0) && (tile->GetFloats()[j] != tile->GetFloats()[0])) {
				floats_all_the_same = false;
			}
		}

		for (uint32_t j = 0; j < vert_count; ++j) {
			SafeVarAllocParse(uint32_t, color);
			tile->GetColors()[j] = color;
		}

		for (uint32_t j = 0; j < vert_count; ++j) {
			SafeVarAllocParse(uint32_t, color);
			tile->GetColors2()[j] = color;
		}

		for (uint32_t j = 0; j < quad_count; ++j) {
			SafeVarAllocParse(uint8_t, flag);
			tile->GetFlags()[j] = flag;

			if(flag & 0x01)
				floats_all_the_same = false;
		}

		if (floats_all_the_same)
			tile->SetFlat(true);

		SafeVarAllocParse(float, unkFloat);
		SafeVarAllocParse(int32_t, unk_unk);
		idx -= sizeof(int32_t);
		SafeVarAllocParse(float, unk_unk2);

		if (unk_unk > 0) {
			SafeVarAllocParse(int8_t, unk_byte);
			if(unk_byte > 0) {
				SafeVarAllocParse(float, f1);
				SafeVarAllocParse(float, f2);
				SafeVarAllocParse(float, f3);
				SafeVarAllocParse(float, f4);
			}

			SafeVarAllocParse(float, f1);
		}

		SafeVarAllocParse(uint32_t, layer_count);
		SafeStringAllocParse(base_material);
		//tile.SetBaseMaterial(base_material);

		uint32_t overlay_count = 0;
		for (uint32_t layer = 1; layer < layer_count; ++layer) {
			SafeStringAllocParse(material);

			SafeVarAllocParse(uint32_t, detail_mask_dim);
			uint32_t sz_m = detail_mask_dim * detail_mask_dim;

			//TerrainTileLayer layer;
			//DetailMask dm;
			for (uint32_t b = 0; b < sz_m; ++b)
			{
				SafeVarAllocParse(uint8_t, detail_mask_byte);
				//dm.AddByte(detail_mask_byte);
			}

			//layer.AddDetailMask(dm);
			//tile.AddLayer(layer);
			++overlay_count;
		}

		SafeVarAllocParse(uint32_t, single_placeable_count);
		for(uint32_t j = 0; j < single_placeable_count; ++j) {
			SafeStringAllocParse(model_name);
			SafeStringAllocParse(s);

			SafeVarAllocParse(uint32_t, longitude);
			SafeVarAllocParse(uint32_t, latitude);

			SafeVarAllocParse(float, x);
			SafeVarAllocParse(float, y);
			SafeVarAllocParse(float, z);

			SafeVarAllocParse(float, rot_x);
			SafeVarAllocParse(float, rot_y);
			SafeVarAllocParse(float, rot_z);

			SafeVarAllocParse(float, scale_x);
			SafeVarAllocParse(float, scale_y);
			SafeVarAllocParse(float, scale_z);
		
			SafeVarAllocParse(uint8_t, unk);

			//Placable p;

			//terrain.AddPlaceable(p);

			//PlaceableGroup pg;

			//terrain.AddPlaceableGroup(pg);

			//do offset stuff derision worked out
		}

		SafeVarAllocParse(uint32_t, areas_count);
		for (uint32_t j = 0; j < areas_count; ++j) {
			SafeStringAllocParse(s);
			SafeVarAllocParse(int32_t, unk);
			SafeStringAllocParse(s2);

			SafeVarAllocParse(uint32_t, longitude);
			SafeVarAllocParse(uint32_t, latitude);

			SafeVarAllocParse(float, x);
			SafeVarAllocParse(float, y);
			SafeVarAllocParse(float, z);

			SafeVarAllocParse(float, rot_x);
			SafeVarAllocParse(float, rot_y);
			SafeVarAllocParse(float, rot_z);

			SafeVarAllocParse(float, scale_x);
			SafeVarAllocParse(float, scale_y);
			SafeVarAllocParse(float, scale_z);

			SafeVarAllocParse(float, size_x);
			SafeVarAllocParse(float, size_y);
			SafeVarAllocParse(float, size_z);

			//TerrainArea ta;

			//tile.AddArea(ta);
		}

		SafeVarAllocParse(uint32_t, Light_effects_count);
		for (uint32_t j = 0; j < Light_effects_count; ++j) {
			SafeStringAllocParse(s);
			SafeStringAllocParse(s2);

			SafeVarAllocParse(int8_t, unk);

			SafeVarAllocParse(uint32_t, longitude);
			SafeVarAllocParse(uint32_t, latitude);

			SafeVarAllocParse(float, x);
			SafeVarAllocParse(float, y);
			SafeVarAllocParse(float, z);

			SafeVarAllocParse(float, rot_x);
			SafeVarAllocParse(float, rot_y);
			SafeVarAllocParse(float, rot_z);

			SafeVarAllocParse(float, scale_x);
			SafeVarAllocParse(float, scale_y);
			SafeVarAllocParse(float, scale_z);

			SafeVarAllocParse(float, unk_float);
		}

		SafeVarAllocParse(uint32_t, tog_ref_count);
		for (uint32_t j = 0; j < tog_ref_count; ++j) {
			SafeStringAllocParse(tog_name);

			SafeVarAllocParse(uint32_t, longitude);
			SafeVarAllocParse(uint32_t, latitude);

			SafeVarAllocParse(float, x);
			SafeVarAllocParse(float, y);
			SafeVarAllocParse(float, z);

			SafeVarAllocParse(float, rot_x);
			SafeVarAllocParse(float, rot_y);
			SafeVarAllocParse(float, rot_z);

			SafeVarAllocParse(float, scale_x);
			SafeVarAllocParse(float, scale_y);
			SafeVarAllocParse(float, scale_z);

			SafeVarAllocParse(float, z_adjust);
		}

		tile->SetLocation(tile_start_x, tile_start_y);
	}

	return true;
}

bool EQEmu::EQG4Loader::GetZon(std::string file, std::vector<char> &buffer) {
	buffer.clear();
	FILE *f = fopen(file.c_str(), "rb");
	if (f) {
		fseek(f, 0, SEEK_END);
		size_t sz = ftell(f);
		rewind(f);

		if (sz != 0) {
			buffer.resize(sz);

			size_t bytes_read = fread(&buffer[0], 1, sz, f);

			if (bytes_read != sz) {
				fclose(f);
				return false;
			}
			fclose(f);
		}
		else {
			fclose(f);
			return false;
		}
		return true;
	}
	return false;
}

bool EQEmu::EQG4Loader::ParseZon(EQEmu::PFS::Archive &archive, std::vector<char> &buffer, ZoneOptions &opts) {
	if (buffer.size() < 5)
		return false;

	v4_zon_header *header = (v4_zon_header*)&buffer[0];

	if (header->magic[0] != 'E' || header->magic[1] != 'Q' || header->magic[2] != 'T' || header->magic[3] != 'Z' || header->magic[4] != 'P')
		return false;

	std::string cur;
	std::vector<std::string> tokens;
	for (size_t i = 0; i < buffer.size(); ++i) {
		char c = buffer[i];
		if (c == '\n' || c == '\r') {
			if (cur.size() > 0) {
				auto sp = EQEmu::SplitString(cur, ' ');
				tokens.insert(tokens.end(), sp.begin(), sp.end());
				cur.clear();
			}
		}
		else {
			cur.push_back(c);
		}
	}

	for (size_t i = 0; i < tokens.size();) {
		auto token = tokens[i];
		if (token.compare("*NAME") == 0) {
			if (i + 1 >= tokens.size()) {
				break;
			}

			opts.name = tokens[i + 1];
			i += 2;
		}
		else if (token.compare("*MINLNG") == 0) {
			if (i + 1 >= tokens.size()) {
				break;
			}

			opts.min_lng = std::stoi(tokens[i + 1]);
			i += 2;
		}
		else if (token.compare("*MAXLNG") == 0) {
			if (i + 1 >= tokens.size()) {
				break;
			}

			opts.max_lng = std::stoi(tokens[i + 1]);
			i += 2;
		}
		else if (token.compare("*MINLAT") == 0) {
			if (i + 1 >= tokens.size()) {
				break;
			}

			opts.min_lat = std::stoi(tokens[i + 1]);
			i += 2;
		}
		else if (token.compare("*MAXLAT") == 0) {
			if (i + 1 >= tokens.size()) {
				break;
			}

			opts.max_lat = std::stoi(tokens[i + 1]);
			i += 2;
		}
		else if (token.compare("*MIN_EXTENTS") == 0) {
			if (i + 3 >= tokens.size()) {
				break;
			}

			opts.min_extents[0] = std::stof(tokens[i + 1]);
			opts.min_extents[1] = std::stof(tokens[i + 2]);
			opts.min_extents[2] = std::stof(tokens[i + 3]);
			i += 4;
		}
		else if (token.compare("*MAX_EXTENTS") == 0) {
			if (i + 3 >= tokens.size()) {
				break;
			}

			opts.max_extents[0] = std::stof(tokens[i + 1]);
			opts.max_extents[1] = std::stof(tokens[i + 2]);
			opts.max_extents[2] = std::stof(tokens[i + 3]);
			i += 4;
		}
		else if (token.compare("*UNITSPERVERT") == 0) {
			if (i + 1 >= tokens.size()) {
				break;
			}

			opts.units_per_vert = std::stof(tokens[i + 1]);
			i += 2;
		}
		else if (token.compare("*QUADSPERTILE") == 0) {
			if (i + 1 >= tokens.size()) {
				break;
			}

			opts.quads_per_tile = std::stoi(tokens[i + 1]);
			i += 2;
		}
		else if (token.compare("*COVERMAPINPUTSIZE") == 0) {
			if (i + 1 >= tokens.size()) {
				break;
			}

			opts.cover_map_input_size = std::stoi(tokens[i + 1]);
			i += 2;
		}
		else if (token.compare("*LAYERINGMAPINPUTSIZE") == 0) {
			if (i + 1 >= tokens.size()) {
				break;
			}

			opts.layer_map_input_size = std::stoi(tokens[i + 1]);
			i += 2;
		}
		else {
			++i;
		}
	}

	return true;
}