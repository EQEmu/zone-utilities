#ifndef EQEMU_COMMON_WLD_STRUCTS_H
#define EQEMU_COMMON_WLD_STRUCTS_H

#include <stdint.h>

#pragma pack(1)

namespace EQEmu {

    struct wld_header {
        uint32_t magic;
        uint32_t version;
        uint32_t fragments;
        uint32_t unk1;
        uint32_t unk2;
        uint32_t hash_length;
        uint32_t unk3;
    };

    struct wld_fragment_header {
        uint32_t size;
        uint32_t id;
        uint32_t name_ref;
    };

    struct wld_fragment_reference {
        int32_t id;
    };

    struct wld_fragment03 {
        uint32_t texture_count;
    };

    struct wld_fragment04 {
        uint32_t flags;
        uint32_t texture_count;
    };

    struct wld_fragment10 {
        uint32_t flag;
        uint32_t track_ref_count;
        uint32_t polygon_anim_frag;
    };

    struct wld_fragment10_track_ref_entry {
        int32_t name_ref;
        uint32_t flag;
        int32_t frag_ref;
        int32_t frag_ref2;
        uint32_t tree_piece_count;
    };

    struct wld_fragment12 {
        uint32_t flag;
        uint32_t size;
        int16_t rot_denom;
        int16_t rot_x_num;
        int16_t rot_y_num;
        int16_t rot_z_num;
        int16_t shift_x_num;
        int16_t shift_y_num;
        int16_t shift_z_num;
        int16_t shift_denom;
    };

    struct wld_fragment14 {
        uint32_t flag;
        int32_t ref;
        uint32_t entries;
        uint32_t entries2;
        int32_t ref2;
    };

    struct wld_fragment15 {
        uint32_t flags;
        uint32_t player_fragment_ref;
        float x;
        float y;
        float z;
        float rotate_z;
        float rotate_y;
        float rotate_x;
        float unk;
        float scale_y;
        float scale_x;
    };

    struct wld_fragment1B {
        uint32_t flags;
        uint32_t params1;
        float color[3];
    };

    struct wld_fragment21 {
        uint32_t count;
    };

    struct wld_fragment21_data {
        float normal[3];
        float split_dist;
        uint32_t region;
        uint32_t node[2];
    };

    struct wld_fragment_28 {
        uint32_t flags;
        float x;
        float y;
        float z;
        float rad;
    };

    struct wld_fragment_29 {
        uint32_t flags;
        uint32_t region_count;
    };

    struct wld_fragment30 {
        uint32_t flags;
        uint32_t params1;
        uint32_t params2;
        float params3[2];
    };

    struct wld_fragment31 {
        uint32_t unk;
        uint32_t count;
    };

    struct wld_fragment36 {
        uint32_t flags;
        uint32_t frag1;
        uint32_t frag2;
        uint32_t frag3;
        uint32_t frag4;
        float center_x;
        float center_y;
        float center_z;
        uint32_t params2[3];
        float max_dist;
        float min_x;
        float min_y;
        float min_z;
        float max_x;
        float max_y;
        float max_z;
        uint16_t vertex_count;
        uint16_t tex_coord_count;
        uint16_t normal_count;
        uint16_t color_count;
        uint16_t polygon_count;
        uint16_t size6;
        uint16_t polygon_tex_count;
        uint16_t vertex_tex_count;
        uint16_t size9;
        int16_t scale;
    };

    struct wld_fragment36_vert {
        int16_t x;
        int16_t y;
        int16_t z;
    };

    struct wld_fragment36_tex_coords_new {
        float u;
        float v;
    };

    struct wld_fragment36_tex_coords_old {
        uint16_t u;
        uint16_t v;
    };

    struct wld_fragment36_normal {
        uint8_t x;
        uint8_t y;
        uint8_t z;
    };

    struct wld_fragment36_poly {
        uint16_t flags;
        uint16_t index[3];
    };

    struct wld_fragment36_tex_map {
        uint16_t poly_count;
        uint16_t tex;
    };

}    // namespace EQEmu

#pragma pack()

#endif
