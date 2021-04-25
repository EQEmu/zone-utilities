#pragma once

#include <glm/glm.hpp>

namespace eqemu {
    namespace math {
        float distance(const glm::vec3& a, const glm::vec3& b);
        float distance_no_root(const glm::vec3& a, const glm::vec3& b);
        float distance_no_z(const glm::vec3& a, const glm::vec3& b);
        float distance_no_root_no_z(const glm::vec3& a, const glm::vec3& b);
    }    // namespace math
}    // namespace eqemu
