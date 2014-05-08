#include "eqg_model_loader.h"
#include "eqg_structs.h"
#include "safe_alloc.h"
#include <algorithm>
#include <functional>
#include <cctype> 

EQGModelLoader::EQGModelLoader() {
}

EQGModelLoader::~EQGModelLoader() {
}

bool EQGModelLoader::Load(EQEmu::PFS::Archive &archive, std::string model) {
	std::transform(model.begin(), model.end(), model.begin(), std::tolower);

	std::vector<char> buffer;
	if(!archive.Get(model, buffer)) {
		return false;
	}

	uint32_t idx = 0;
	SafeStructAllocParse(mod_header, header);
	uint32_t bone_count = 0;

	if (header->magic[0] != 'E' || header->magic[1] != 'Q' || header->magic[2] != 'G')
		return false;

	if (header->magic[3] == 'M')
	{
		bone_count = *(uint32_t*)&buffer[idx];
		idx += sizeof(uint32_t);
	}
	else if(header->magic[3] != 'T')
	{
		return false;
	}
	
	uint32_t list_loc = idx;
	idx += header->list_length;

	std::vector<EQG::Material> materials;
	for(uint32_t i = 0; i < header->material_count; ++i) {
		SafeStructAllocParse(mod_material, mat);
		EQG::Material m;
		m.SetName(&buffer[list_loc + mat->name_offset]);
		m.SetShader(&buffer[list_loc + mat->shader_offset]);

		for(uint32_t j = 0; j < mat->property_count; ++j) {
			SafeStructAllocParse(mod_material_property, prop);
			EQG::Material::Property p;
			p.name = &buffer[list_loc + prop->name_offset];

			if (prop->type == 2) {
				p.value_s = &buffer[list_loc + prop->i_value];
				p.value_f = 0.0f;
				p.value_i = 0;
			} else if(prop->type == 0) {
				p.value_f = prop->f_value;
				p.value_i = 0;
			} else {
				p.value_i = prop->i_value;
				p.value_f = 0.0f;
			}

			m.AddProperty(p);
		}
	}

	return false;
}