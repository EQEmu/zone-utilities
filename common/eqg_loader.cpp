#include "eqg_loader.h"
#include <stdio.h>
#include "eqg_structs.h"
#include "safe_alloc.h"
#include "eqg_model_loader.h"

EQGLoader::EQGLoader() {
}

EQGLoader::~EQGLoader() {
}

bool EQGLoader::Load(std::string file, std::vector<std::shared_ptr<EQG::Geometry>> &models, std::vector<std::shared_ptr<Placeable>> &placeables,
	std::vector<std::shared_ptr<EQG::Region>> &regions, std::vector<std::shared_ptr<Light>> &lights) {
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

	if (!ParseZon(archive, zon, models, placeables, regions, lights)) {
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

bool EQGLoader::ParseZon(EQEmu::PFS::Archive &archive, std::vector<char> &buffer, std::vector<std::shared_ptr<EQG::Geometry>> &models, std::vector<std::shared_ptr<Placeable>> &placeables,
	std::vector<std::shared_ptr<EQG::Region>> &regions, std::vector<std::shared_ptr<Light>> &lights) {
	uint32_t idx = 0;
	SafeStructAllocParse(zon_header, header);

	if (header->magic[0] != 'E' || header->magic[1] != 'Q' || header->magic[2] != 'G' || header->magic[3] != 'Z')
	{
		return false;
	}

	idx += header->list_length;
	std::vector<std::string> model_names;
	for(uint32_t i = 0; i < header->model_count; ++i) {
		SafeVarAllocParse(uint32_t, model);

		std::string mod = &buffer[sizeof(zon_header) + model];
		for(size_t j = 0; j < mod.length(); ++j) {
			if(mod[j] == ')')
				mod[j] = '_';
		}
		model_names.push_back(mod);
	}

	//Need to load all the models
	EQGModelLoader model_loader;
	for (size_t i = 0; i < model_names.size(); ++i) {
		std::string mod = model_names[i];
		std::shared_ptr<EQG::Geometry> m(new EQG::Geometry());
		m->SetName(mod);
		if(model_loader.Load(archive, mod, m)) {
			models.push_back(m);
		} 
		else {
			m->GetMaterials().clear();
			m->GetPolygons().clear();
			m->GetVertices().clear();
			models.push_back(m);
		}
	}

	//load placables
	float rot_change = 180.0f / 3.14159f;
	for (uint32_t i = 0; i < header->object_count; ++i) {
		SafeStructAllocParse(zon_placable, plac);

		std::shared_ptr<Placeable> p(new Placeable());
		p->SetName(&buffer[sizeof(zon_header) + plac->loc]);
		if (plac->id >= 0 && plac->id < models.size()) {
			p->SetFileName(model_names[plac->id]);
		}

		p->SetLocation(plac->x, plac->y, plac->z);
		p->SetRotation(plac->rx * rot_change, plac->ry * rot_change, plac->rz * rot_change);
		p->SetScale(plac->scale, plac->scale, plac->scale);

		if(header->version > 1) {
			//don't know what this is but it relates to the underlying model
			SafeVarAllocParse(uint32_t, unk_size);
			idx += (unk_size) * sizeof(uint32_t);
		}

		placeables.push_back(p);
	}

	for(uint32_t i = 0; i < header->region_count; ++i) {
		SafeStructAllocParse(zon_region, reg);

		std::shared_ptr<EQG::Region> region(new EQG::Region());
		region->SetName(&buffer[sizeof(zon_header) + reg->loc]);
		region->SetLocation(reg->center_x, reg->center_y, reg->center_z);
		region->SetExtents(reg->extend_x, reg->extend_y, reg->extend_z);
		region->SetFlags(reg->flag_unknown020, reg->flag_unknown024);

		regions.push_back(region);
	}

	for(uint32_t i = 0; i < header->light_count; ++i) {
		SafeStructAllocParse(zon_light, light);
		std::shared_ptr<Light> l(new Light());
		l->SetName(&buffer[sizeof(zon_header) + light->loc]);
		l->SetLocation(light->x, light->y, light->z);
		l->SetColor(light->r, light->g, light->b);
		l->SetRadius(light->radius);

		lights.push_back(l);
	}

	return true;
}
