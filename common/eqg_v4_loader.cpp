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
	if (!ParseZon(zon, opts)) {
		return false;
	}

	terrain.reset(new Terrain());
	if(!ParseZoneDat(archive, opts, terrain)) {
		return false;
	}

	ParseWaterDat(archive, terrain);
	ParseInvwDat(archive, terrain);

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

bool EQEmu::EQG4Loader::ParseWaterDat(EQEmu::PFS::Archive &archive, std::shared_ptr<Terrain> &terrain) {
	std::vector<char> wat;
	if(!archive.Get("water.dat", wat)) {
		return false;
	}

	std::vector<std::string> tokens;
	ParseConfigFile(wat, tokens);

	std::shared_ptr<WaterSheet> ws;

	for (size_t i = 1; i < tokens.size();) {
		auto token = tokens[i];
		if (token.compare("*WATERSHEET") == 0) {
			ws.reset(new WaterSheet());

			++i;
		}
		else if (token.compare("*END_SHEET") == 0) {
			if(ws) {
				terrain->AddWaterSheet(ws);
			}

			++i;
		} else if (token.compare("*MINX") == 0) {
			if (i + 1 >= tokens.size()) {
				break;
			}

			float min_x = std::stof(tokens[i + 1]);
			ws->SetMinX(min_x);

			i += 2;
		} else if (token.compare("*MINY") == 0) {
			if (i + 1 >= tokens.size()) {
				break;
			}

			float min_y = std::stof(tokens[i + 1]);
			ws->SetMinY(min_y);

			i += 2;
		} else if (token.compare("*MAXX") == 0) {
			if (i + 1 >= tokens.size()) {
				break;
			}

			float max_x = std::stof(tokens[i + 1]);
			ws->SetMaxX(max_x);

			i += 2;
		} else if (token.compare("*MAXY") == 0) {
			if (i + 1 >= tokens.size()) {
				break;
			}

			float max_y = std::stof(tokens[i + 1]);
			ws->SetMaxY(max_y);

			i += 2;
		} else if (token.compare("*ZHEIGHT") == 0) {
			if (i + 1 >= tokens.size()) {
				break;
			}

			float z = std::stof(tokens[i + 1]);
			ws->SetZHeight(z);

			i += 2;
		}
		else {
			++i;
		}
	}

	return true;
}

bool EQEmu::EQG4Loader::ParseInvwDat(EQEmu::PFS::Archive &archive, std::shared_ptr<Terrain> &terrain) {
	std::vector<char> invw;
	if (!archive.Get("invw.dat", invw)) {
		return false;
	}

	char *buf = &invw[0];
	uint32_t count = *(uint32_t*)buf;
	buf += sizeof(uint32_t);

	for(uint32_t i = 0; i < count; ++i) {
		std::string name = buf;
		buf += name.length() + 1;

		uint32_t flag = *(uint32_t*)buf;
		buf += sizeof(uint32_t);

		uint32_t vert_count = *(uint32_t*)buf;
		buf += sizeof(uint32_t);

		std::shared_ptr<InvisWall> w(new InvisWall());
		w->SetName(name);
		auto &verts = w->GetVerts();

		verts.resize(vert_count);
		for(uint32_t j = 0; j < vert_count; ++j) {
			float x = *(float*)buf;
			buf += sizeof(float);

			float y = *(float*)buf;
			buf += sizeof(float);

			float z = *(float*)buf;
			buf += sizeof(float);

			verts[j].x = x;
			verts[j].y = y;
			verts[j].z = z;			
		}

		terrain->AddInvisWall(w);
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

void EQEmu::EQG4Loader::ParseConfigFile(std::vector<char> &buffer, std::vector<std::string> &tokens) {
	tokens.clear();
	std::string cur;
	for (size_t i = 0; i < buffer.size(); ++i) {
		char c = buffer[i];
		if(c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == '\v' || c == '\f') {
			if(cur.size() > 0) {
				tokens.push_back(cur);
				cur.clear();
			}
		} else {
			cur.push_back(c);
		}
	}
}

bool EQEmu::EQG4Loader::ParseZon(std::vector<char> &buffer, ZoneOptions &opts) {
	if (buffer.size() < 5)
		return false;

	std::vector<std::string> tokens;
	ParseConfigFile(buffer, tokens);

	if(tokens.size() < 1) {
		return false;
	}

	if(tokens[0].compare("EQTZP") != 0) {
		return 0;
	}

	for (size_t i = 1; i < tokens.size();) {
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