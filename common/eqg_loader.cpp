#include "eqg_loader.h"
#include <stdio.h>
#include "pfs.h"
#include "eqg_structs.h"

EQGLoader::EQGLoader() {
}

EQGLoader::~EQGLoader() {
}

bool EQGLoader::Load(std::string file) {
	// find zon file
	EQEmu::PFS::Archive archive;
	if(!archive.Open(file + ".eqg")) {
		return false;
	}

	std::vector<char> zon;
	bool zon_found = true;
	std::list<std::string> files;
	archive.GetFilenames("zon", files);

	if(files.size() == 0) {
		if (!GetZon(file + ".zon", zon)) {
			zon_found = false;
		}
	} else {
		auto iter = files.begin();
		if (!archive.Get(*iter, zon)) {
			zon_found = false;
		}
	}

	//If no zon file existed then this is probably a model we don't handle that yet
	//Need to reverse the model layout file sometime but atm not important.
	if (!zon_found)
		return false;

	if(!ParseZon(zon)) {
		//if we couldn't parse the zon file then it's probably eqg4
		return false;
	}

	return true;
}

bool EQGLoader::GetZon(std::string file, std::vector<char> &buffer) {
	buffer.clear();
	FILE *f = fopen(file.c_str(), "rb");
	if(f) {
		fseek(f, 0, SEEK_END);
		size_t sz = ftell(f);
		rewind(f);

		if(sz != 0) {
			buffer.resize(sz);

			size_t bytes_read = fread(&buffer[0], 1, sz, f);

			if(bytes_read != sz) {
				fclose(f);
				return false;
			}
			fclose(f);
		} else {
			fclose(f);
			return false;
		}
		return true;
	}
	return false;
}

#define SafeStructAllocParse(type, var_name) if(idx + sizeof(type) > buffer.size()) { return false; } \
	type *var_name = (type*)&buffer[idx]; \
	idx += sizeof(type);

#define SafeBufferAllocParse(var_name, length) if(idx + length > buffer.size()) { return false; } \
	var_name = (char*)&buffer[idx]; \
	idx += length;

bool EQGLoader::ParseZon(std::vector<char> &buffer) {
	uint32_t idx = 0;
	SafeStructAllocParse(zon_header, header);

	if (header->magic[0] != 'E' || header->magic[1] != 'Q' || header->magic[2] != 'G' || header->magic[3] != 'Z')
	{
		return false;
	}

	idx += header->list_length;
	std::vector<std::string> models;
	for(uint32_t i = 0; i < header->model_count; ++i) {
		SafeStructAllocParse(uint32_t, model);

		std::string mod = &buffer[sizeof(zon_header)+*model];
		for(size_t j = 0; j < mod.length(); ++j) {
			if(mod[j] == ')')
				mod[j] = '_';
		}
		models.push_back(mod);
	}

	//Need to load all the models

	//load placables
	for (uint32_t i = 0; i < header->object_count; ++i) {
		SafeStructAllocParse(zon_placable, plac);
		std::string name = &buffer[sizeof(zon_header) + plac->loc];

		if(header->version > 1) {
			SafeStructAllocParse(uint32_t, unk_size);
			idx += (*unk_size) * sizeof(uint32_t); //don't know what this is but it relates to the underlying model
		}
	}

	for(uint32_t i = 0; i < header->region_count; ++i) {
		SafeStructAllocParse(zon_region, azp);
	}

	return false;
}

bool EQGLoader::LoadModel(std::string file) {
	return false;
}