#ifndef EQEMU_MAP_H
#define EQEMU_MAP_H

#include <map>
#include <stdint.h>
#include <string>
#include <tuple>
#include <vector>

#include <glm/glm.hpp>

#include "eqg_loader.h"
#include "eqg_v4_loader.h"
#include "s3d_loader.h"

class Map {
public:
    Map();
    ~Map();

    bool Build(std::string zone_name, bool ignore_collide_tex);
    bool Write(std::string filename);

private:
    void TraverseBone(std::shared_ptr<EQEmu::S3D::SkeletonTrack::Bone> bone,
                      glm::vec3 parent_trans,
                      glm::vec3 parent_rot,
                      glm::vec3 parent_scale);

    bool CompileS3D(std::vector<EQEmu::S3D::WLDFragment>& zone_frags,
                    std::vector<EQEmu::S3D::WLDFragment>& zone_object_frags,
                    std::vector<EQEmu::S3D::WLDFragment>& object_frags,
                    bool ignore_collide_tex);
    bool CompileEQG(std::vector<std::shared_ptr<EQEmu::EQG::Geometry>>& models,
                    std::vector<std::shared_ptr<EQEmu::Placeable>>& placeables,
                    std::vector<std::shared_ptr<EQEmu::EQG::Region>>& regions,
                    std::vector<std::shared_ptr<EQEmu::Light>>& lights);
    bool CompileEQGv4();
    void LoadIgnore(std::string zone_name);

    void AddFace(glm::vec3& v1, glm::vec3& v2, glm::vec3& v3, bool collidable);

    void RotateVertex(glm::vec3& v, float rx, float ry, float rz);
    void ScaleVertex(glm::vec3& v, float sx, float sy, float sz);
    void TranslateVertex(glm::vec3& v, float tx, float ty, float tz);

    std::vector<glm::vec3> collide_verts;
    std::vector<uint32_t> collide_indices;

    std::vector<glm::vec3> non_collide_verts;
    std::vector<uint32_t> non_collide_indices;

    uint32_t current_collide_index;
    uint32_t current_non_collide_index;

    std::map<std::tuple<float, float, float>, uint32_t> collide_vert_to_index;
    std::map<std::tuple<float, float, float>, uint32_t> non_collide_vert_to_index;

    std::shared_ptr<EQEmu::EQG::Terrain> terrain;
    std::map<std::string, std::shared_ptr<EQEmu::S3D::Geometry>> map_models;
    std::map<std::string, std::shared_ptr<EQEmu::EQG::Geometry>> map_eqg_models;
    std::vector<std::shared_ptr<EQEmu::Placeable>> map_placeables;
    std::vector<std::shared_ptr<EQEmu::PlaceableGroup>> map_group_placeables;
    std::map<std::string, bool> ignore_placs;
};

#endif
