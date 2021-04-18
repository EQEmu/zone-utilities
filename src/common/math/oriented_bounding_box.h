#pragma once

#include "matrix_op.h"

namespace eqemu {
    namespace math {
        class oriented_bounding_box {
        public:
            oriented_bounding_box() = default;
            oriented_bounding_box(const glm::vec3& pos,
                                  const glm::vec3& rot,
                                  const glm::vec3& scale,
                                  const glm::vec3& extents)
                : _min(), _max(), _transformation(), _inverted_transformation() {
                _min.x = -extents.x;
                _max.x = extents.x;

                if(_min.x > _max.x) {
                    float t = _min.x;
                    _min.x = _max.x;
                    _max.x = t;
                }

                _min.y = -extents.y;
                _max.y = extents.y;
                if(_min.y > _max.y) {
                    float t = _min.y;
                    _min.y = _max.y;
                    _max.y = t;
                }

                _min.z = -extents.z;
                _max.z = extents.z;
                if(_min.z > _max.z) {
                    float t = _min.z;
                    _min.z = _max.z;
                    _max.z = t;
                }

                // rotate
                _transformation = create_rotate_matrix(
                    rot.x * 3.14159f / 180.0f, rot.y * 3.14159f / 180.0f, rot.z * 3.14159f / 180.0f);

                // scale
                _transformation = create_scale_matrix(scale.x, scale.y, scale.z) * _transformation;

                // translate
                _transformation = create_translate_matrix(pos.x, pos.y, pos.z) * _transformation;
                _inverted_transformation = glm::inverse(_transformation);
            }
            ~oriented_bounding_box() {}

            bool contains_point(const glm::vec3& p) const {
                glm::vec4 pt(p.x, p.y, p.z, 1);
                glm::vec4 box_space_p = _inverted_transformation * pt;

                if(box_space_p.x >= _min.x && box_space_p.x <= _max.x && box_space_p.y >= _min.y &&
                   box_space_p.y <= _max.y && box_space_p.z >= _min.z && box_space_p.z <= _max.z) {
                    return true;
                }

                return false;
            }

            glm::mat4& get_transformation() { return _transformation; }
            glm::mat4& get_inverted_transformation() { return _inverted_transformation; }

            float get_min_x() const { return _min.x; }
            float get_min_y() const { return _min.y; }
            float get_min_z() const { return _min.z; }
            float get_max_x() const { return _max.x; }
            float get_max_y() const { return _max.y; }
            float get_max_z() const { return _max.z; }

        private:
            glm::vec3 _min;
            glm::vec3 _max;
            glm::mat4 _transformation;
            glm::mat4 _inverted_transformation;
        };
    }    // namespace math
}    // namespace eqemu
