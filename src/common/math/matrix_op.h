#pragma once

#include <glm.hpp>

namespace eqemu {
    namespace math {
        glm::mat4 create_rotate_matrix(float rx, float ry, float rz);
        glm::mat4 create_translate_matrix(float tx, float ty, float tz);
        glm::mat4 create_scale_matrix(float sx, float sy, float sz);
    }    // namespace math
}    // namespace eqemu
