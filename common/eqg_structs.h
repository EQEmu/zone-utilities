#ifndef EQEMU_EQG_STRUCTS_H
#define EQEMU_EQG_STRUCTS_H

#include <stdint.h>

#pragma pack(1)

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
	float unknown016; 
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

#pragma pack()

#endif