#include "eqg_model_loader.h"
#include "eqg_structs.h"
#include "safe_alloc.h"
#include <algorithm>
#include <functional>
#include <cctype> 

EQEmu::EQGModelLoader::EQGModelLoader() {
}

EQEmu::EQGModelLoader::~EQGModelLoader() {
}

bool EQEmu::EQGModelLoader::Load(EQEmu::PFS::Archive &archive, std::string model, std::shared_ptr<EQG::Geometry> model_out) {
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

	auto &mats = model_out->GetMaterials();
	mats.resize(header->material_count);
	for(uint32_t i = 0; i < header->material_count; ++i) {
		SafeStructAllocParse(mod_material, mat);
		auto &m = mats[i];
		m.SetName(&buffer[list_loc + mat->name_offset]);
		m.SetShader(&buffer[list_loc + mat->shader_offset]);

		auto &props = m.GetProperties();
		props.resize(mat->property_count);
		for(uint32_t j = 0; j < mat->property_count; ++j) {
			SafeStructAllocParse(mod_material_property, prop);
			auto &p = props[j];
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
		}
	}

	auto &verts = model_out->GetVertices();
	verts.resize(header->vert_count);
	for(uint32_t i = 0; i < header->vert_count; ++i) {
		if(header->version < 3) {
			SafeStructAllocParse(mod_vertex, vert);

			auto &v = verts[i];
			v.pos.x = vert->x;
			v.pos.y = vert->y;
			v.pos.z = vert->z;
			v.nor.x = vert->i;
			v.nor.y = vert->j;
			v.nor.z = vert->k;
			v.nor.x = vert->u;
			v.nor.y = vert->v;
			v.col = 4294967295;
		} else {
			SafeStructAllocParse(mod_vertex3, vert);

			auto &v = verts[i];
			v.pos.x = vert->x;
			v.pos.y = vert->y;
			v.pos.z = vert->z;
			v.nor.x = vert->i;
			v.nor.y = vert->j;
			v.nor.z = vert->k;
			v.nor.x = vert->u;
			v.nor.y = vert->v;
			v.col = vert->color;
		}
	}

	auto &polys = model_out->GetPolygons();
	polys.resize(header->tri_count);
	for(uint32_t i = 0; i < header->tri_count; ++i) {
		SafeStructAllocParse(mod_polygon, poly);
		auto &p = polys[i];
		p.verts[0] = poly->v1;
		p.verts[1] = poly->v2;
		p.verts[2] = poly->v3;
		p.material = poly->material;
		p.flags = poly->flags;
	}

	return true;
}