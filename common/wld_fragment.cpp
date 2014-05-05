#include "wld_fragment.h"
#include "wld_structs.h"
#include "s3d_loader.h"


WLDFragment03::WLDFragment03(S3DLoader *loader, std::vector<WLDFragment> &out, char *frag_buffer, uint32_t frag_length, uint32_t frag_name, char *hash, bool old) {
	wld_fragment03 *header = (wld_fragment03*)frag_buffer;
	frag_buffer += sizeof(wld_fragment03);
	uint32_t count = header->texture_count;
	if(!count)
		count = 1;
	
	std::shared_ptr<Texture> tex(new Texture);
	for(uint32_t i = 0; i < count; ++i) {
		uint16_t name_len = *(uint16_t*)frag_buffer;
		frag_buffer += sizeof(uint16_t);

		char *texture_name = frag_buffer;
		decode_string_hash(texture_name, name_len);
		tex->AddTextureFrame(texture_name);

		frag_buffer += name_len;
	}

	data = tex;
}

WLDFragment04::WLDFragment04(S3DLoader *loader, std::vector<WLDFragment> &out, char *frag_buffer, uint32_t frag_length, uint32_t frag_name, char *hash, bool old) {
	wld_fragment04 *header = (wld_fragment04*)frag_buffer;
	frag_buffer += sizeof(wld_fragment04);

	if (header->flags & (1 << 2))
		frag_buffer += sizeof(uint32_t);

	if (header->flags & (1 << 3))
		frag_buffer += sizeof(uint32_t);

	uint32_t count = header->texture_count;
	if(!count)
		count = 1;

	std::shared_ptr<TextureBrush> brush(new TextureBrush);
	for (uint32_t i = 0; i < count; ++i) {
		wld_fragment_reference *ref = (wld_fragment_reference*)frag_buffer;
		frag_buffer += sizeof(wld_fragment_reference);

		WLDFragment &frag = out[ref->id - 1];
		try {
			std::shared_ptr<Texture> tex = EQEmu::any_cast<std::shared_ptr<Texture>>(frag.data);
			brush->AddTexture(tex);
		} catch (EQEmu::bad_any_cast&) {
			brush->AddTexture(std::shared_ptr<Texture>(new Texture()));
		}
	}

	data = brush;
}

WLDFragment05::WLDFragment05(S3DLoader *loader, std::vector<WLDFragment> &out, char *frag_buffer, uint32_t frag_length, uint32_t frag_name, char *hash, bool old) {
	wld_fragment_reference *ref = (wld_fragment_reference*)frag_buffer;
	uint32_t frag_id = ref->id - 1;
	data = frag_id;
}

WLDFragment15::WLDFragment15(S3DLoader *loader, std::vector<WLDFragment> &out, char *frag_buffer, uint32_t frag_length, uint32_t frag_name, char *hash, bool old) {
	wld_fragment_reference *ref = (wld_fragment_reference*)frag_buffer;
	frag_buffer += sizeof(wld_fragment_reference);

	wld_fragment15 *header = (wld_fragment15*)frag_buffer;
	if(header->flags == 0x32E) {
		std::shared_ptr<Placeable> plac(new Placeable());
		plac->SetLocation(header->x, header->y, header->z);
		plac->SetRotation(
			header->rotate_x / 512.f * 360.f,
			header->rotate_y / 512.f * 360.f,
			header->rotate_z / 512.f * 360.f
			);
		plac->SetScale(header->scale_x, header->scale_y, header->scale_y);
		
		const char *model_str = (const char*)&hash[-ref->id];
		plac->SetName(model_str);
		data = plac;
	}
}

WLDFragment1B::WLDFragment1B(S3DLoader *loader, std::vector<WLDFragment> &out, char *frag_buffer, uint32_t frag_length, uint32_t frag_name, char *hash, bool old) {
	std::shared_ptr<Light> light(new Light());
	wld_fragment1B *header = (wld_fragment1B*)frag_buffer;

	light->SetLocation(0.0f, 0.0f, 0.0f);
	light->SetRadius(0.0f);
	
	if (header->flags & (1 << 3)) {
		light->SetColor(header->color[0], header->color[1], header->color[2]);
	} else {
		light->SetColor(1.0f, 1.0f, 1.0f);
	}

	data = light;
}

WLDFragment1C::WLDFragment1C(S3DLoader *loader, std::vector<WLDFragment> &out, char *frag_buffer, uint32_t frag_length, uint32_t frag_name, char *hash, bool old) {
	wld_fragment_reference *ref = (wld_fragment_reference*)frag_buffer;
	uint32_t frag_id = ref->id - 1;
	data = frag_id;
}

WLDFragment21::WLDFragment21(S3DLoader *loader, std::vector<WLDFragment> &out, char *frag_buffer, uint32_t frag_length, uint32_t frag_name, char *hash, bool old) {
	wld_fragment21 *header = (wld_fragment21*)frag_buffer;
	frag_buffer += sizeof(wld_fragment21);

	std::shared_ptr<BSPTree> tree(new BSPTree());
	for(uint32_t i = 0; i < header->count; ++i) {
		wld_fragment21_data *data = (wld_fragment21_data*)frag_buffer;
		frag_buffer += sizeof(wld_fragment21_data);

		BSPTree::BSPNode node;
		node.number = i;
		node.normal[0] = data->normal[0];
		node.normal[1] = data->normal[1];
		node.normal[2] = data->normal[2];
		node.split_dist = data->split_dist;
		node.region = data->region;
		node.left = data->node[0];
		node.right = data->node[1];
		tree->AddNode(node);
	}

	data = tree;
}

WLDFragment22::WLDFragment22(S3DLoader *loader, std::vector<WLDFragment> &out, char *frag_buffer, uint32_t frag_length, uint32_t frag_name, char *hash, bool old) {
	//bsp regions
}

WLDFragment28::WLDFragment28(S3DLoader *loader, std::vector<WLDFragment> &out, char *frag_buffer, uint32_t frag_length, uint32_t frag_name, char *hash, bool old) {
	wld_fragment_reference *ref = (wld_fragment_reference*)frag_buffer;
	frag_buffer += sizeof(wld_fragment_reference);

	wld_fragment_28 *header = (wld_fragment_28*)frag_buffer;
	frag_buffer += sizeof(wld_fragment_28);

	if(ref->id == 0)
		return;

	WLDFragment &frag = out[ref->id - 1];
	uint32_t ref_id = 0;
	try {
		ref_id = EQEmu::any_cast<uint32_t>(frag.data);
	}
	catch (EQEmu::bad_any_cast&) { }

	frag = out[ref_id];
	try {
		std::shared_ptr<Light> l = EQEmu::any_cast<std::shared_ptr<Light>>(frag.data);
		l->SetLocation(header->x, header->y, header->z);
		l->SetRadius(header->rad);
	}
	catch (EQEmu::bad_any_cast&) {}
}

WLDFragment29::WLDFragment29(S3DLoader *loader, std::vector<WLDFragment> &out, char *frag_buffer, uint32_t frag_length, uint32_t frag_name, char *hash, bool old) {
	wld_fragment_29 *header = (wld_fragment_29*)frag_buffer;
	frag_buffer += sizeof(wld_fragment_29);

	std::shared_ptr<BSPRegion> region(new BSPRegion());
	int32_t ref_id = (int32_t)frag_name;
	char *region_name = &hash[-ref_id];
	region->SetName(region_name);

	for(uint32_t i = 0; i < header->region_count; ++i) {
		uint32_t ref_id = *(uint32_t*)frag_buffer;
		frag_buffer += sizeof(uint32_t);
		region->AddRegion(ref_id);
	}

	uint32_t str_length = *(uint32_t*)frag_buffer;
	if(str_length > 0) {
		frag_buffer += sizeof(uint32_t);
		char *str = frag_buffer;
		decode_string_hash(str, str_length);
		region->SetExtendedInfo(str);
	}

	data = region;
}

WLDFragment30::WLDFragment30(S3DLoader *loader, std::vector<WLDFragment> &out, char *frag_buffer, uint32_t frag_length, uint32_t frag_name, char *hash, bool old) {
	//texture reference to a 0x05
	wld_fragment30 *header = (wld_fragment30*)frag_buffer;
	frag_buffer += sizeof(wld_fragment30);

	if(!header->flags)
		frag_buffer += sizeof(uint32_t) * 2;

	wld_fragment_reference *ref = (wld_fragment_reference*)frag_buffer;

	if(!header->params1 || !ref->id) {
		std::shared_ptr<TextureBrush> tb(new TextureBrush());
		std::shared_ptr<Texture> t(new Texture());
		t->AddTextureFrame("collide.dds");
		tb->AddTexture(t);
		tb->SetFlags(1);
		data = tb;
		return;
	}
	
	WLDFragment &frag = out[ref->id - 1];
	uint32_t tex_ref = 0;
	try {
		tex_ref = EQEmu::any_cast<uint32_t>(frag.data);
	}
	catch (EQEmu::bad_any_cast&) { }
	
	
	frag = out[tex_ref];
	try {
		std::shared_ptr<TextureBrush> tb = EQEmu::any_cast<std::shared_ptr<TextureBrush>>(frag.data);
		std::shared_ptr<TextureBrush> new_tb(new TextureBrush);

		*new_tb = *tb;

		if (header->params1 & (1 << 1) || header->params1 & (1 << 2) || header->params1 & (1 << 3) || header->params1 & (1 << 4))
			new_tb->SetFlags(1);
		else
			new_tb->SetFlags(0);
	
		data = new_tb;
	}
	catch (EQEmu::bad_any_cast&) { }
}

WLDFragment31::WLDFragment31(S3DLoader *loader, std::vector<WLDFragment> &out, char *frag_buffer, uint32_t frag_length, uint32_t frag_name, char *hash, bool old) {
	wld_fragment31 *header = (wld_fragment31*)frag_buffer;
	frag_buffer += sizeof(wld_fragment31);

	std::shared_ptr<TextureBrushSet> tbs(new TextureBrushSet());

	for(uint32_t i = 0; i < header->count; ++i) {
		uint32_t ref_id = *(uint32_t*)frag_buffer;
		frag_buffer += sizeof(uint32_t);
		WLDFragment &frag = out[ref_id - 1];
		try {
			std::shared_ptr<TextureBrush> tb = EQEmu::any_cast<std::shared_ptr<TextureBrush>>(frag.data);
			tbs->AddTextureSet(tb);
		}
		catch (EQEmu::bad_any_cast&) {}
	}

	data = tbs;
}

WLDFragment36::WLDFragment36(S3DLoader *loader, std::vector<WLDFragment> &out, char *frag_buffer, uint32_t frag_length, uint32_t frag_name, char *hash, bool old) {
	wld_fragment36 *header = (wld_fragment36*)frag_buffer;
	frag_buffer += sizeof(wld_fragment36);

	float scale = 1.0f / (float)(1 << header->scale);
	float recip_255 = 1.0f / 256.0f, recip_127 = 1.0f / 127.0f;

	std::shared_ptr<Geometry> model(new Geometry());
	model->SetName((char*)&hash[-(int32_t)frag_name]);

	WLDFragment &tex_frag = out[header->frag1 - 1];
	try {
		std::shared_ptr<TextureBrushSet> tbs = EQEmu::any_cast<std::shared_ptr<TextureBrushSet>>(tex_frag.data);
		model->SetTextureBrushSet(tbs);
	}
	catch (EQEmu::bad_any_cast&) {}

	for(uint32_t i = 0; i < header->vertex_count; ++i) {
		wld_fragment36_vert *in = (wld_fragment36_vert*)frag_buffer;
		frag_buffer += sizeof(wld_fragment36_vert);

		Geometry::Vertex v;
		v.pos.x = header->center_x + in->x * scale;
		v.pos.y = header->center_y + in->y * scale;
		v.pos.z = header->center_z + in->z * scale;

		model->AddVertex(v);
	}

	if(old) {
		for(uint32_t i = 0; i < header->tex_coord_count; ++i) {
			wld_fragment36_tex_coords_old *in = (wld_fragment36_tex_coords_old*)frag_buffer;
		
			if (i < model->GetVertices().size()) {
				model->GetVertices()[i].tex.x = in->u * recip_255;
				model->GetVertices()[i].tex.y = in->v * recip_255;
			}

			frag_buffer += sizeof(wld_fragment36_tex_coords_old);
		}
	} else {
		for (uint32_t i = 0; i < header->tex_coord_count; ++i) {
			wld_fragment36_tex_coords_new *in = (wld_fragment36_tex_coords_new*)frag_buffer;

			if (i < model->GetVertices().size()) {
				model->GetVertices()[i].tex.x = in->u;
				model->GetVertices()[i].tex.y = in->v;
			}

			frag_buffer += sizeof(wld_fragment36_tex_coords_new);
		}
	}

	for(uint32_t i = 0; i < header->normal_count; ++i) {
		wld_fragment36_normal *in = (wld_fragment36_normal*)frag_buffer;

		// this check is here cause there's literally zones where there's normals than verts (ssratemple for ex). Stupid I know.
		if (i < model->GetVertices().size()) {
			model->GetVertices()[i].nor.x = in->x * recip_127;
			model->GetVertices()[i].nor.y = in->y * recip_127;
			model->GetVertices()[i].nor.z = in->z * recip_127;
		}

		frag_buffer += sizeof(wld_fragment36_normal);
	}

	frag_buffer += sizeof(uint32_t) * header->color_count;

	for(uint32_t i = 0; i < header->polygon_count; ++i) {
		wld_fragment36_poly *in = (wld_fragment36_poly*)frag_buffer;

		Geometry::Polygon p;
		p.flags = in->flags;
		p.verts[0] = in->index[0];
		p.verts[1] = in->index[1];
		p.verts[2] = in->index[2];

		model->AddPolygon(p);
		frag_buffer += sizeof(wld_fragment36_poly);
	}

	frag_buffer += 4 * header->size6;
	uint32_t pc = 0;
	for(uint32_t i = 0; i < header->polygon_tex_count; ++i) {
		wld_fragment36_tex_map *tm = (wld_fragment36_tex_map*)frag_buffer;

		for(uint32_t j = 0; j < tm->poly_count; ++j) {
			if(pc >= model->GetPolygons().size()) {
				break;
			}

			model->GetPolygons()[pc++].tex = tm->tex;
		}

		frag_buffer += sizeof(wld_fragment36_tex_map);
	}

	data = model;
}
