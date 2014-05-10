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

bool EQEmu::EQG4Loader::Load(std::string file)
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

	if(!ParseDat(archive, opts)) {
		return false;
	}

	return true;
}

//#define VARSTRUCT_DECODE_TYPE(Type, Buffer) *(Type *)Buffer; Buffer += sizeof(Type);
//#define VARSTRUCT_DECODE_STRING(String, Buffer) strcpy(String, Buffer); Buffer += strlen(String)+1;
//#define VARSTRUCT_ENCODE_STRING(Buffer, String) sprintf(Buffer, String); Buffer += strlen(String) + 1;
//#define VARSTRUCT_ENCODE_INTSTRING(Buffer, Number) sprintf(Buffer, "%i", Number); Buffer += strlen(Buffer) + 1;
//#define VARSTRUCT_ENCODE_TYPE(Type, Buffer, Value) *(Type *)Buffer = Value; Buffer += sizeof(Type);
//#define VARSTRUCT_SKIP_TYPE(Type, Buffer) Buffer += sizeof(Type);

bool EQEmu::EQG4Loader::ParseDat(EQEmu::PFS::Archive &archive, ZoneOptions &opts) {
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

	float zone_min_x = opts.min_lat * opts.quads_per_tile * opts.units_per_vert;
	float zone_max_x = (opts.max_lat + 1) * opts.quads_per_tile * opts.units_per_vert;

	float zone_min_y = opts.min_lng * opts.quads_per_tile * opts.units_per_vert;
	float zone_max_y = (opts.max_lng + 1) * opts.quads_per_tile * opts.units_per_vert;

	uint32_t quad_count = (opts.quads_per_tile * opts.quads_per_tile);
	uint32_t vert_count = ((opts.quads_per_tile + 1) * (opts.quads_per_tile + 1));

	int vert_number = -1;
	int quad_number = -1;

	EQG::Geometry zone_model;
	zone_model.GetVertices().resize((size_t)(quad_count * tile_count * 4));
	zone_model.GetPolygons().resize((size_t)(quad_count * tile_count * 2));

	std::vector<float> floats(vert_count, 0);
	std::vector<uint32_t> colors(vert_count, 0);
	std::vector<uint32_t> colors2(vert_count, 0);
	std::vector<uint8_t> flags(quad_count, 0);

	for(uint32_t i = 0; i < tile_count; ++i) {
		SafeVarAllocParse(int32_t, tile_lng);
		SafeVarAllocParse(int32_t, tile_lat);
		SafeVarAllocParse(int32_t, tile_unk);

		float tile_start_y = zone_min_y + (tile_lng - 100000 - opts.min_lng) * opts.units_per_vert * opts.quads_per_tile;
		float tile_start_x = zone_min_x + (tile_lat - 100000 - opts.min_lat) * opts.units_per_vert * opts.quads_per_tile;
	
		bool floats_all_the_same = true;
		for (uint32_t j = 0; j < vert_count; ++j) {
			SafeVarAllocParse(float, t);
			floats[j] = t;

			if ((j > 0) && (floats[j] != floats[0])) {
				floats_all_the_same = false;
			}
		}

		for (uint32_t j = 0; j < vert_count; ++j) {
			SafeVarAllocParse(uint32_t, color);
			colors[j] = color;
		}

		for (uint32_t j = 0; j < vert_count; ++j) {
			SafeVarAllocParse(uint32_t, color);
			colors2[j] = color;
		}

		for (uint32_t j = 0; j < quad_count; ++j) {
			SafeVarAllocParse(uint8_t, flag);
			flags[j] = flag;
		}

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
	}

	return false;
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