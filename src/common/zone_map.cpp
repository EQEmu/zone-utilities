#include <algorithm>
#include <locale>
#include <map>
#include <memory>
#include <tuple>

#include "zone_map.h"

#include "core/compression.h"
#include "core/config.h"

struct ZoneMap::impl {
    std::vector<glm::vec3> verts;
    std::vector<unsigned int> inds;
    glm::vec3 min;
    glm::vec3 max;

    std::vector<glm::vec3> nc_verts;
    std::vector<unsigned int> nc_inds;
    glm::vec3 nc_min;
    glm::vec3 nc_max;
};

ZoneMap::ZoneMap() {
    imp = new impl;

    imp->min = glm::vec3(0.0f);
    imp->max = glm::vec3(0.0f);
    imp->nc_min = glm::vec3(0.0f);
    imp->nc_max = glm::vec3(0.0f);
}

ZoneMap::~ZoneMap() {
    delete imp;
}

ZoneMap* ZoneMap::LoadMapFile(std::string file) {
    std::string filename = eqemu::core::config::instance().get_path("base", "maps/base") + "/";
    std::transform(file.begin(), file.end(), file.begin(), ::tolower);
    filename += file;
    filename += ".map";

    ZoneMap* m = new ZoneMap();
    if(m->Load(filename)) {
        return m;
    }

    delete m;
    return nullptr;
}

bool ZoneMap::Load(std::string filename) {
    FILE* f = fopen(filename.c_str(), "rb");
    if(f) {
        uint32_t version;
        if(fread(&version, sizeof(version), 1, f) != 1) {
            fclose(f);
            return false;
        }

        if(version == 0x01000000) {
            bool v = LoadV1(f);
            fclose(f);
            return v;
        } else if(version == 0x02000000) {
            bool v = LoadV2(f);
            fclose(f);
            return v;
        } else {
            fclose(f);
            return false;
        }
    }

    return false;
}

bool ZoneMap::LoadV1(FILE* f) {
    uint32_t face_count;
    uint16_t node_count;
    uint32_t facelist_count;

    if(fread(&face_count, sizeof(face_count), 1, f) != 1) {
        return false;
    }

    if(fread(&node_count, sizeof(node_count), 1, f) != 1) {
        return false;
    }

    if(fread(&facelist_count, sizeof(facelist_count), 1, f) != 1) {
        return false;
    }

    for(uint32_t i = 0; i < face_count; ++i) {
        glm::vec3 a;
        glm::vec3 b;
        glm::vec3 c;
        float normals[4];
        if(fread(&a, sizeof(glm::vec3), 1, f) != 1) {
            return false;
        }

        if(fread(&b, sizeof(glm::vec3), 1, f) != 1) {
            return false;
        }

        if(fread(&c, sizeof(glm::vec3), 1, f) != 1) {
            return false;
        }

        if(fread(normals, sizeof(normals), 1, f) != 1) {
            return false;
        }

        size_t sz = imp->verts.size();
        imp->verts.push_back(a);
        imp->verts.push_back(b);
        imp->verts.push_back(c);

        imp->inds.push_back((uint32_t)sz + 0);
        imp->inds.push_back((uint32_t)sz + 1);
        imp->inds.push_back((uint32_t)sz + 2);
    }

    float t;
    for(auto& v : imp->verts) {
        t = v.y;
        v.y = v.z;
        v.z = t;
    }

    for(auto& vert : imp->verts) {
        if(vert.x < imp->min.x) {
            imp->min.x = vert.x;
        }

        if(vert.y < imp->min.y) {
            imp->min.y = vert.y;
        }

        if(vert.z < imp->min.z) {
            imp->min.z = vert.z;
        }

        if(vert.x > imp->max.x) {
            imp->max.x = vert.x;
        }

        if(vert.y > imp->max.y) {
            imp->max.y = vert.y;
        }

        if(vert.z > imp->max.z) {
            imp->max.z = vert.z;
        }
    }

    return true;
}

struct ModelEntry {
    struct Poly {
        uint32_t v1, v2, v3;
        uint8_t vis;
    };
    std::vector<glm::vec3> verts;
    std::vector<Poly> polys;
};

bool ZoneMap::LoadV2(FILE* f) {
    uint32_t data_size;
    if(fread(&data_size, sizeof(data_size), 1, f) != 1) {
        return false;
    }

    uint32_t buffer_size;
    if(fread(&buffer_size, sizeof(buffer_size), 1, f) != 1) {
        return false;
    }

    std::vector<char> data;
    data.resize(data_size);
    if(fread(&data[0], data_size, 1, f) != 1) {
        return false;
    }

    std::vector<char> buffer;
    buffer.resize(buffer_size);
    uint32_t v = eqemu::core::inflate_data(&data[0], data_size, &buffer[0], buffer_size);

    char* buf = &buffer[0];
    uint32_t vert_count;
    uint32_t ind_count;
    uint32_t nc_vert_count;
    uint32_t nc_ind_count;
    uint32_t model_count;
    uint32_t plac_count;
    uint32_t plac_group_count;
    uint32_t tile_count;
    uint32_t quads_per_tile;
    float units_per_vertex;

    vert_count = *(uint32_t*)buf;
    buf += sizeof(uint32_t);

    ind_count = *(uint32_t*)buf;
    buf += sizeof(uint32_t);

    nc_vert_count = *(uint32_t*)buf;
    buf += sizeof(uint32_t);

    nc_ind_count = *(uint32_t*)buf;
    buf += sizeof(uint32_t);

    model_count = *(uint32_t*)buf;
    buf += sizeof(uint32_t);

    plac_count = *(uint32_t*)buf;
    buf += sizeof(uint32_t);

    plac_group_count = *(uint32_t*)buf;
    buf += sizeof(uint32_t);

    tile_count = *(uint32_t*)buf;
    buf += sizeof(uint32_t);

    quads_per_tile = *(uint32_t*)buf;
    buf += sizeof(uint32_t);

    units_per_vertex = *(float*)buf;
    buf += sizeof(float);

    for(uint32_t i = 0; i < vert_count; ++i) {
        float x;
        float y;
        float z;

        x = *(float*)buf;
        buf += sizeof(float);

        y = *(float*)buf;
        buf += sizeof(float);

        z = *(float*)buf;
        buf += sizeof(float);

        glm::vec3 vert(x, y, z);

        imp->verts.push_back(vert);
    }

    for(uint32_t i = 0; i < ind_count; ++i) {
        uint32_t index;
        index = *(uint32_t*)buf;
        buf += sizeof(uint32_t);

        imp->inds.push_back(index);
    }

    for(uint32_t i = 0; i < nc_vert_count; ++i) {
        float x;
        float y;
        float z;

        x = *(float*)buf;
        buf += sizeof(float);

        y = *(float*)buf;
        buf += sizeof(float);

        z = *(float*)buf;
        buf += sizeof(float);

        glm::vec3 vert(x, y, z);

        imp->nc_verts.push_back(vert);
    }

    for(uint32_t i = 0; i < nc_ind_count; ++i) {
        uint32_t index;
        index = *(uint32_t*)buf;
        buf += sizeof(uint32_t);

        imp->nc_inds.push_back(index);
    }

    std::map<std::string, std::shared_ptr<ModelEntry>> models;
    for(uint32_t i = 0; i < model_count; ++i) {
        std::shared_ptr<ModelEntry> me(new ModelEntry);
        std::string name = buf;
        buf += name.length() + 1;

        uint32_t vert_count = *(uint32_t*)buf;
        buf += sizeof(uint32_t);

        uint32_t poly_count = *(uint32_t*)buf;
        buf += sizeof(uint32_t);

        me->verts.resize(vert_count);
        for(uint32_t j = 0; j < vert_count; ++j) {
            float x = *(float*)buf;
            buf += sizeof(float);
            float y = *(float*)buf;
            buf += sizeof(float);
            float z = *(float*)buf;
            buf += sizeof(float);

            me->verts[j] = glm::vec3(x, y, z);
        }

        me->polys.resize(poly_count);
        for(uint32_t j = 0; j < poly_count; ++j) {
            uint32_t v1 = *(uint32_t*)buf;
            buf += sizeof(uint32_t);
            uint32_t v2 = *(uint32_t*)buf;
            buf += sizeof(uint32_t);
            uint32_t v3 = *(uint32_t*)buf;
            buf += sizeof(uint32_t);
            uint8_t vis = *(uint8_t*)buf;
            buf += sizeof(uint8_t);

            ModelEntry::Poly p;
            p.v1 = v1;
            p.v2 = v2;
            p.v3 = v3;
            p.vis = vis;
            me->polys[j] = p;
        }

        models[name] = me;
    }

    for(uint32_t i = 0; i < plac_count; ++i) {
        std::string name = buf;
        buf += name.length() + 1;

        float x = *(float*)buf;
        buf += sizeof(float);
        float y = *(float*)buf;
        buf += sizeof(float);
        float z = *(float*)buf;
        buf += sizeof(float);

        float x_rot = *(float*)buf;
        buf += sizeof(float);
        float y_rot = *(float*)buf;
        buf += sizeof(float);
        float z_rot = *(float*)buf;
        buf += sizeof(float);

        float x_scale = *(float*)buf;
        buf += sizeof(float);
        float y_scale = *(float*)buf;
        buf += sizeof(float);
        float z_scale = *(float*)buf;
        buf += sizeof(float);

        if(models.count(name) == 0)
            continue;

        auto model = models[name];
        auto& mod_polys = model->polys;
        auto& mod_verts = model->verts;
        for(uint32_t j = 0; j < mod_polys.size(); ++j) {
            auto& current_poly = mod_polys[j];
            auto v1 = mod_verts[current_poly.v1];
            auto v2 = mod_verts[current_poly.v2];
            auto v3 = mod_verts[current_poly.v3];

            RotateVertex(v1, x_rot, y_rot, z_rot);
            RotateVertex(v2, x_rot, y_rot, z_rot);
            RotateVertex(v3, x_rot, y_rot, z_rot);

            ScaleVertex(v1, x_scale, y_scale, z_scale);
            ScaleVertex(v2, x_scale, y_scale, z_scale);
            ScaleVertex(v3, x_scale, y_scale, z_scale);

            TranslateVertex(v1, x, y, z);
            TranslateVertex(v2, x, y, z);
            TranslateVertex(v3, x, y, z);

            float t = v1.x;
            v1.x = v1.y;
            v1.y = t;

            t = v2.x;
            v2.x = v2.y;
            v2.y = t;

            t = v3.x;
            v3.x = v3.y;
            v3.y = t;

            if(current_poly.vis != 0) {
                imp->verts.push_back(v1);
                imp->verts.push_back(v2);
                imp->verts.push_back(v3);

                imp->inds.push_back((uint32_t)imp->verts.size() - 3);
                imp->inds.push_back((uint32_t)imp->verts.size() - 2);
                imp->inds.push_back((uint32_t)imp->verts.size() - 1);
            } else {
                imp->nc_verts.push_back(v1);
                imp->nc_verts.push_back(v2);
                imp->nc_verts.push_back(v3);

                imp->nc_inds.push_back((uint32_t)imp->nc_verts.size() - 3);
                imp->nc_inds.push_back((uint32_t)imp->nc_verts.size() - 2);
                imp->nc_inds.push_back((uint32_t)imp->nc_verts.size() - 1);
            }
        }
    }

    for(uint32_t i = 0; i < plac_group_count; ++i) {
        float x = *(float*)buf;
        buf += sizeof(float);
        float y = *(float*)buf;
        buf += sizeof(float);
        float z = *(float*)buf;
        buf += sizeof(float);

        float x_rot = *(float*)buf;
        buf += sizeof(float);
        float y_rot = *(float*)buf;
        buf += sizeof(float);
        float z_rot = *(float*)buf;
        buf += sizeof(float);

        float x_scale = *(float*)buf;
        buf += sizeof(float);
        float y_scale = *(float*)buf;
        buf += sizeof(float);
        float z_scale = *(float*)buf;
        buf += sizeof(float);

        float x_tile = *(float*)buf;
        buf += sizeof(float);
        float y_tile = *(float*)buf;
        buf += sizeof(float);
        float z_tile = *(float*)buf;
        buf += sizeof(float);

        uint32_t p_count = *(uint32_t*)buf;
        buf += sizeof(uint32_t);

        for(uint32_t j = 0; j < p_count; ++j) {
            std::string name = buf;
            buf += name.length() + 1;

            float p_x = *(float*)buf;
            buf += sizeof(float);
            float p_y = *(float*)buf;
            buf += sizeof(float);
            float p_z = *(float*)buf;
            buf += sizeof(float);

            float p_x_rot = *(float*)buf * 3.14159f / 180;
            buf += sizeof(float);
            float p_y_rot = *(float*)buf * 3.14159f / 180;
            buf += sizeof(float);
            float p_z_rot = *(float*)buf * 3.14159f / 180;
            buf += sizeof(float);

            float p_x_scale = *(float*)buf;
            buf += sizeof(float);
            float p_y_scale = *(float*)buf;
            buf += sizeof(float);
            float p_z_scale = *(float*)buf;
            buf += sizeof(float);

            if(models.count(name) == 0)
                continue;

            auto& model = models[name];

            for(size_t k = 0; k < model->polys.size(); ++k) {
                auto& poly = model->polys[k];
                glm::vec3 v1, v2, v3;

                v1 = model->verts[poly.v1];
                v2 = model->verts[poly.v2];
                v3 = model->verts[poly.v3];

                ScaleVertex(v1, p_x_scale, p_y_scale, p_z_scale);
                ScaleVertex(v2, p_x_scale, p_y_scale, p_z_scale);
                ScaleVertex(v3, p_x_scale, p_y_scale, p_z_scale);

                TranslateVertex(v1, p_x, p_y, p_z);
                TranslateVertex(v2, p_x, p_y, p_z);
                TranslateVertex(v3, p_x, p_y, p_z);

                RotateVertex(v1, x_rot * 3.14159f / 180.0f, 0, 0);
                RotateVertex(v2, x_rot * 3.14159f / 180.0f, 0, 0);
                RotateVertex(v3, x_rot * 3.14159f / 180.0f, 0, 0);

                RotateVertex(v1, 0, y_rot * 3.14159f / 180.0f, 0);
                RotateVertex(v2, 0, y_rot * 3.14159f / 180.0f, 0);
                RotateVertex(v3, 0, y_rot * 3.14159f / 180.0f, 0);

                glm::vec3 correction(p_x, p_y, p_z);

                RotateVertex(correction, x_rot * 3.14159f / 180.0f, 0, 0);

                TranslateVertex(v1, -correction.x, -correction.y, -correction.z);
                TranslateVertex(v2, -correction.x, -correction.y, -correction.z);
                TranslateVertex(v3, -correction.x, -correction.y, -correction.z);

                RotateVertex(v1, p_x_rot, 0, 0);
                RotateVertex(v2, p_x_rot, 0, 0);
                RotateVertex(v3, p_x_rot, 0, 0);

                RotateVertex(v1, 0, -p_y_rot, 0);
                RotateVertex(v2, 0, -p_y_rot, 0);
                RotateVertex(v3, 0, -p_y_rot, 0);

                RotateVertex(v1, 0, 0, p_z_rot);
                RotateVertex(v2, 0, 0, p_z_rot);
                RotateVertex(v3, 0, 0, p_z_rot);

                TranslateVertex(v1, correction.x, correction.y, correction.z);
                TranslateVertex(v2, correction.x, correction.y, correction.z);
                TranslateVertex(v3, correction.x, correction.y, correction.z);

                RotateVertex(v1, 0, 0, z_rot * 3.14159f / 180.0f);
                RotateVertex(v2, 0, 0, z_rot * 3.14159f / 180.0f);
                RotateVertex(v3, 0, 0, z_rot * 3.14159f / 180.0f);

                ScaleVertex(v1, x_scale, y_scale, z_scale);
                ScaleVertex(v2, x_scale, y_scale, z_scale);
                ScaleVertex(v3, x_scale, y_scale, z_scale);

                TranslateVertex(v1, x_tile, y_tile, z_tile);
                TranslateVertex(v2, x_tile, y_tile, z_tile);
                TranslateVertex(v3, x_tile, y_tile, z_tile);

                TranslateVertex(v1, x, y, z);
                TranslateVertex(v2, x, y, z);
                TranslateVertex(v3, x, y, z);

                float t = v1.x;
                v1.x = v1.y;
                v1.y = t;

                t = v2.x;
                v2.x = v2.y;
                v2.y = t;

                t = v3.x;
                v3.x = v3.y;
                v3.y = t;

                if(poly.vis != 0) {
                    imp->verts.push_back(v1);
                    imp->verts.push_back(v2);
                    imp->verts.push_back(v3);

                    imp->inds.push_back((uint32_t)imp->verts.size() - 3);
                    imp->inds.push_back((uint32_t)imp->verts.size() - 2);
                    imp->inds.push_back((uint32_t)imp->verts.size() - 1);
                } else {
                    imp->nc_verts.push_back(v1);
                    imp->nc_verts.push_back(v2);
                    imp->nc_verts.push_back(v3);

                    imp->nc_inds.push_back((uint32_t)imp->nc_verts.size() - 3);
                    imp->nc_inds.push_back((uint32_t)imp->nc_verts.size() - 2);
                    imp->nc_inds.push_back((uint32_t)imp->nc_verts.size() - 1);
                }
            }
        }
    }

    uint32_t ter_quad_count = (quads_per_tile * quads_per_tile);
    uint32_t ter_vert_count = ((quads_per_tile + 1) * (quads_per_tile + 1));
    std::vector<uint8_t> flags;
    std::vector<float> floats;
    flags.resize(ter_quad_count);
    floats.resize(ter_vert_count);
    for(uint32_t i = 0; i < tile_count; ++i) {
        bool flat;
        flat = *(bool*)buf;
        buf += sizeof(bool);

        float x;
        x = *(float*)buf;
        buf += sizeof(float);

        float y;
        y = *(float*)buf;
        buf += sizeof(float);

        if(flat) {
            float z;
            z = *(float*)buf;
            buf += sizeof(float);

            float QuadVertex1X = x;
            float QuadVertex1Y = y;
            float QuadVertex1Z = z;

            float QuadVertex2X = QuadVertex1X + (quads_per_tile * units_per_vertex);
            float QuadVertex2Y = QuadVertex1Y;
            float QuadVertex2Z = QuadVertex1Z;

            float QuadVertex3X = QuadVertex2X;
            float QuadVertex3Y = QuadVertex1Y + (quads_per_tile * units_per_vertex);
            float QuadVertex3Z = QuadVertex1Z;

            float QuadVertex4X = QuadVertex1X;
            float QuadVertex4Y = QuadVertex3Y;
            float QuadVertex4Z = QuadVertex1Z;

            uint32_t current_vert = (uint32_t)imp->verts.size() + 3;
            imp->verts.push_back(glm::vec3(QuadVertex1X, QuadVertex1Y, QuadVertex1Z));
            imp->verts.push_back(glm::vec3(QuadVertex2X, QuadVertex2Y, QuadVertex2Z));
            imp->verts.push_back(glm::vec3(QuadVertex3X, QuadVertex3Y, QuadVertex3Z));
            imp->verts.push_back(glm::vec3(QuadVertex4X, QuadVertex4Y, QuadVertex4Z));

            imp->inds.push_back(current_vert - 0);
            imp->inds.push_back(current_vert - 1);
            imp->inds.push_back(current_vert - 2);

            imp->inds.push_back(current_vert - 2);
            imp->inds.push_back(current_vert - 3);
            imp->inds.push_back(current_vert - 0);
        } else {
            // read flags
            for(uint32_t j = 0; j < ter_quad_count; ++j) {
                uint8_t f;
                f = *(uint8_t*)buf;
                buf += sizeof(uint8_t);

                flags[j] = f;
            }

            // read floats
            for(uint32_t j = 0; j < ter_vert_count; ++j) {
                float f;
                f = *(float*)buf;
                buf += sizeof(float);

                floats[j] = f;
            }

            int row_number = -1;
            std::map<std::tuple<float, float, float>, uint32_t> cur_verts;
            for(uint32_t quad = 0; quad < ter_quad_count; ++quad) {
                if((quad % quads_per_tile) == 0) {
                    ++row_number;
                }

                if(flags[quad] & 0x01)
                    continue;

                float QuadVertex1X = x + (row_number * units_per_vertex);
                float QuadVertex1Y = y + (quad % quads_per_tile) * units_per_vertex;
                float QuadVertex1Z = floats[quad + row_number];

                float QuadVertex2X = QuadVertex1X + units_per_vertex;
                float QuadVertex2Y = QuadVertex1Y;
                float QuadVertex2Z = floats[quad + row_number + quads_per_tile + 1];

                float QuadVertex3X = QuadVertex1X + units_per_vertex;
                float QuadVertex3Y = QuadVertex1Y + units_per_vertex;
                float QuadVertex3Z = floats[quad + row_number + quads_per_tile + 2];

                float QuadVertex4X = QuadVertex1X;
                float QuadVertex4Y = QuadVertex1Y + units_per_vertex;
                float QuadVertex4Z = floats[quad + row_number + 1];

                uint32_t i1, i2, i3, i4;
                std::tuple<float, float, float> t = std::make_tuple(QuadVertex1X, QuadVertex1Y, QuadVertex1Z);
                auto iter = cur_verts.find(t);
                if(iter != cur_verts.end()) {
                    i1 = iter->second;
                } else {
                    i1 = (uint32_t)imp->verts.size();
                    imp->verts.push_back(glm::vec3(QuadVertex1X, QuadVertex1Y, QuadVertex1Z));
                    cur_verts[std::make_tuple(QuadVertex1X, QuadVertex1Y, QuadVertex1Z)] = i1;
                }

                t = std::make_tuple(QuadVertex2X, QuadVertex2Y, QuadVertex2Z);
                iter = cur_verts.find(t);
                if(iter != cur_verts.end()) {
                    i2 = iter->second;
                } else {
                    i2 = (uint32_t)imp->verts.size();
                    imp->verts.push_back(glm::vec3(QuadVertex2X, QuadVertex2Y, QuadVertex2Z));
                    cur_verts[std::make_tuple(QuadVertex2X, QuadVertex2Y, QuadVertex2Z)] = i2;
                }

                t = std::make_tuple(QuadVertex3X, QuadVertex3Y, QuadVertex3Z);
                iter = cur_verts.find(t);
                if(iter != cur_verts.end()) {
                    i3 = iter->second;
                } else {
                    i3 = (uint32_t)imp->verts.size();
                    imp->verts.push_back(glm::vec3(QuadVertex3X, QuadVertex3Y, QuadVertex3Z));
                    cur_verts[std::make_tuple(QuadVertex3X, QuadVertex3Y, QuadVertex3Z)] = i3;
                }

                t = std::make_tuple(QuadVertex4X, QuadVertex4Y, QuadVertex4Z);
                iter = cur_verts.find(t);
                if(iter != cur_verts.end()) {
                    i4 = iter->second;
                } else {
                    i4 = (uint32_t)imp->verts.size();
                    imp->verts.push_back(glm::vec3(QuadVertex4X, QuadVertex4Y, QuadVertex4Z));
                    cur_verts[std::make_tuple(QuadVertex4X, QuadVertex4Y, QuadVertex4Z)] = i4;
                }

                imp->inds.push_back(i4);
                imp->inds.push_back(i3);
                imp->inds.push_back(i2);

                imp->inds.push_back(i2);
                imp->inds.push_back(i1);
                imp->inds.push_back(i4);
            }
        }
    }

    float t;
    for(auto& v : imp->verts) {
        t = v.y;
        v.y = v.z;
        v.z = t;
    }

    for(auto& vert : imp->verts) {
        if(vert.x < imp->min.x) {
            imp->min.x = vert.x;
        }

        if(vert.y < imp->min.y && vert.y > -15000) {
            imp->min.y = vert.y;
        }

        if(vert.z < imp->min.z) {
            imp->min.z = vert.z;
        }

        if(vert.x > imp->max.x) {
            imp->max.x = vert.x;
        }

        if(vert.y > imp->max.y) {
            imp->max.y = vert.y;
        }

        if(vert.z > imp->max.z) {
            imp->max.z = vert.z;
        }
    }

    for(auto& v : imp->nc_verts) {
        t = v.y;
        v.y = v.z;
        v.z = t;
    }

    for(auto& vert : imp->nc_verts) {
        if(vert.x < imp->nc_min.x) {
            imp->nc_min.x = vert.x;
        }

        if(vert.y < imp->nc_min.y) {
            imp->nc_min.y = vert.y;
        }

        if(vert.z < imp->nc_min.z) {
            imp->nc_min.z = vert.z;
        }

        if(vert.x > imp->nc_max.x) {
            imp->nc_max.x = vert.x;
        }

        if(vert.y > imp->nc_max.y) {
            imp->nc_max.y = vert.y;
        }

        if(vert.z > imp->nc_max.z) {
            imp->nc_max.z = vert.z;
        }
    }

    return true;
}

const std::vector<glm::vec3>& ZoneMap::GetCollidableVerts() const {
    return imp->verts;
}

const std::vector<unsigned int>& ZoneMap::GetCollidableInds() const {
    return imp->inds;
}

const glm::vec3& ZoneMap::GetCollidableMax() const {
    return imp->max;
}

const glm::vec3& ZoneMap::GetCollidableMin() const {
    return imp->min;
}

const std::vector<glm::vec3>& ZoneMap::GetNonCollidableVerts() const {
    return imp->nc_verts;
}

const std::vector<unsigned int>& ZoneMap::GetNonCollidableInds() const {
    return imp->nc_inds;
}

const glm::vec3& ZoneMap::GetNonCollidableMax() const {
    return imp->nc_max;
}

const glm::vec3& ZoneMap::GetNonCollidableMin() const {
    return imp->nc_min;
}

void ZoneMap::RotateVertex(glm::vec3& v, float rx, float ry, float rz) {
    glm::vec3 nv = v;

    nv.y = (cos(rx) * v.y) - (sin(rx) * v.z);
    nv.z = (sin(rx) * v.y) + (cos(rx) * v.z);

    v = nv;

    nv.x = (cos(ry) * v.x) + (sin(ry) * v.z);
    nv.z = -(sin(ry) * v.x) + (cos(ry) * v.z);

    v = nv;

    nv.x = (cos(rz) * v.x) - (sin(rz) * v.y);
    nv.y = (sin(rz) * v.x) + (cos(rz) * v.y);

    v = nv;
}

void ZoneMap::ScaleVertex(glm::vec3& v, float sx, float sy, float sz) {
    v.x = v.x * sx;
    v.y = v.y * sy;
    v.z = v.z * sz;
}

void ZoneMap::TranslateVertex(glm::vec3& v, float tx, float ty, float tz) {
    v.x = v.x + tx;
    v.y = v.y + ty;
    v.z = v.z + tz;
}
