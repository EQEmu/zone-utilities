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

namespace eqemu {
    namespace azone {

        class map {
        public:
            map();
            ~map();

            bool build(const std::string& zone_name, bool ignore_collide_tex);
            bool write(const std::string& filename);

        private:
            void traverse_bone(std::shared_ptr<EQEmu::S3D::SkeletonTrack::Bone> bone,
                               glm::vec3 parent_trans,
                               glm::vec3 parent_rot,
                               glm::vec3 parent_scale);

            bool compile_s3d(std::vector<EQEmu::S3D::WLDFragment>& zone_frags,
                             std::vector<EQEmu::S3D::WLDFragment>& zone_object_frags,
                             std::vector<EQEmu::S3D::WLDFragment>& object_frags,
                             bool ignore_collide_tex);
            bool compile_eqg(std::vector<std::shared_ptr<EQEmu::EQG::Geometry>>& models,
                             std::vector<std::shared_ptr<EQEmu::Placeable>>& placeables,
                             std::vector<std::shared_ptr<EQEmu::EQG::Region>>& regions,
                             std::vector<std::shared_ptr<EQEmu::Light>>& lights);
            bool compile_eqgv4();
            void load_ignore(const std::string& zone_name);

            void add_face(glm::vec3& v1, glm::vec3& v2, glm::vec3& v3, bool collidable);

            void rotate_vertex(glm::vec3& v, float rx, float ry, float rz);
            void scale_vertex(glm::vec3& v, float sx, float sy, float sz);
            void translate_vertex(glm::vec3& v, float tx, float ty, float tz);

            std::vector<glm::vec3> _collide_verts;
            std::vector<uint32_t> _collide_indices;

            std::vector<glm::vec3> _non_collide_verts;
            std::vector<uint32_t> _non_collide_indices;

            uint32_t _current_collide_index;
            uint32_t _current_non_collide_index;

            std::map<std::tuple<float, float, float>, uint32_t> _collide_vert_to_index;
            std::map<std::tuple<float, float, float>, uint32_t> _non_collide_vert_to_index;

            std::shared_ptr<EQEmu::EQG::Terrain> _terrain;
            std::map<std::string, std::shared_ptr<EQEmu::S3D::Geometry>> _map_models;
            std::map<std::string, std::shared_ptr<EQEmu::EQG::Geometry>> _map_eqg_models;
            std::vector<std::shared_ptr<EQEmu::Placeable>> _map_placeables;
            std::vector<std::shared_ptr<EQEmu::PlaceableGroup>> _map_group_placeables;
            std::map<std::string, bool> _ignore_placs;
        };

    }    // namespace azone
}    // namespace eqemu

#endif
