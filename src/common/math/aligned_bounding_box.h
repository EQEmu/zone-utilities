#pragma once

#include <glm.hpp>

namespace eqemu {
    namespace math {
        class aligned_bounding_box {
        public:
            aligned_bounding_box() : _center(0.0), _radius(0.0), _min(0.0), _max(0.0) {}
            aligned_bounding_box(const glm::vec3& min, const glm::vec3& max)
                : _center((max.x + min.x) / 2, (max.y + min.y) / 2, (max.z + min.z) / 2),
                  _radius(max.x - _center.x, max.y - _center.y, max.z - _center.z), _min(min), _max(max) {}
            aligned_bounding_box(const glm::vec3& c, const float r)
                : _center(c), _radius(r, r, r), _min(c.x - r, c.y - r, c.z - r), _max(c.x + r, c.y + r, c.z + r) {}
            aligned_bounding_box(const glm::vec3& c, const float rx, float ry, float rz)
                : _center(c), _radius(rx, ry, rz), _min(c.x - rx, c.y - ry, c.z - rz),
                  _max(c.x + rx, c.y + ry, c.z + rz) {}
            ~aligned_bounding_box() {}

            bool contains(const glm::vec3& p) const {
                return p.x >= _min.x && p.x <= _max.x && p.y >= _min.y && p.y <= _max.y && p.z >= _min.z &&
                       p.z <= _max.z;
            }

            bool intersects_aabb(const aligned_bounding_box& b) const {
                if(_max.x < b._min.x)
                    return false;
                if(_min.x > b._max.x)
                    return false;
                if(_max.y < b._min.y)
                    return false;
                if(_min.y > b._max.y)
                    return false;
                if(_max.z < b._min.z)
                    return false;
                if(_min.z > b._max.z)
                    return false;
                return true;
            }

            bool intersects_sphere(const glm::vec3& center, const float diameter) const {
                float dist = 0.0;

                if(center.x < _min.x)
                    dist += (center.x - _min.x) * (center.x - _min.x);
                else if(center.x > _max.x)
                    dist += (center.x - _max.x) * (center.x - _max.x);
                if(center.y < _min.y)
                    dist += (center.y - _min.y) * (center.y - _min.y);
                else if(center.y > _max.y)
                    dist += (center.y - _max.y) * (center.y - _max.y);
                if(center.z < _min.z)
                    dist += (center.z - _min.z) * (center.z - _min.z);
                else if(center.z > _max.z)
                    dist += (center.z - _max.z) * (center.z - _max.z);
                return dist <= diameter;
            }

            glm::vec3 _center;
            glm::vec3 _radius;
            glm::vec3 _max;
            glm::vec3 _min;
        };
    }    // namespace math
}    // namespace eqemu
