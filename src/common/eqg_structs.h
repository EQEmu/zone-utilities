#ifndef EQEMU_COMMON_EQG_STRUCTS_H
#define EQEMU_COMMON_EQG_STRUCTS_H

#include <stdint.h>

#pragma pack(1)

namespace EQEmu
{

struct eqg_v4_zone_magic_header
{
	char magic[5];
};

struct zon_header
{
	char magic[4];
	uint32_t version;
	uint32_t list_length;
	uint32_t model_count;
	uint32_t object_count;
	uint32_t region_count;
	uint32_t light_count;
};

struct zon_placable
{
	int32_t id;
	uint32_t loc;
	float x;
	float y;
	float z;
	float rx;
	float ry;
	float rz;
	float scale;
};

struct zon_region
{
	uint32_t loc;
	float center_x; 
	float center_y; 
	float center_z; 
	float rotation; //z rotation in EQ heading units eg out of 512 instead of 360
	uint32_t flag_unknown020; //0 about half the time, non-zero other half
	uint32_t flag_unknown024; //almost always 0
	float extend_x; 
	float extend_y;
	float extend_z;
};

struct zon_light
{
	uint32_t loc;
	float x;
	float y;
	float z;
	float r;
	float g;
	float b;
	float radius;
};

struct mod_header
{
	char magic[4];
	uint32_t version;
	uint32_t list_length;
	uint32_t material_count;
	uint32_t vert_count;
	uint32_t tri_count;
};

struct mod_material
{
	uint32_t index;
	uint32_t name_offset;
	uint32_t shader_offset;
	uint32_t property_count;
};

struct mod_material_property
{
	uint32_t name_offset;
	uint32_t type;
	union
	{
		uint32_t i_value;
		float f_value;
	};
};

struct mod_vertex
{
	float x;
	float y;
	float z;
	float i;
	float j;
	float k;
	float u;
	float v;
};

struct mod_vertex3
{
	float x;
	float y;
	float z;
	float i;
	float j;
	float k;
	uint32_t color;
	float unknown036;
	float unknown040;
	float u;
	float v;
};

struct mod_polygon
{
	uint32_t v1;
	uint32_t v2;
	uint32_t v3;
	int32_t material;
	uint32_t flags;
};

struct v4_zone_dat_header
{
	uint32_t unk000;
	uint32_t unk004;
	uint32_t unk008;
};

}

#pragma pack()

#endif