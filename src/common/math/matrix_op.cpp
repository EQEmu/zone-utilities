#include "matrix_op.h"

#include <gtc/matrix_transform.hpp>

glm::mat4 eqemu::math::create_rotate_matrix(float rx, float ry, float rz) {
    glm::mat4 rot_x(1.0f);
    rot_x[1][1] = cos(rx);
    rot_x[2][1] = -sin(rx);
    rot_x[1][2] = sin(rx);
    rot_x[2][2] = cos(rx);

    glm::mat4 rot_y(1.0f);
    rot_y[0][0] = cos(ry);
    rot_y[2][0] = sin(ry);
    rot_y[0][2] = -sin(ry);
    rot_y[2][2] = cos(ry);

    glm::mat4 rot_z(1.0f);
    rot_z[0][0] = cos(rz);
    rot_z[1][0] = -sin(rz);
    rot_z[0][1] = sin(rz);
    rot_z[1][1] = cos(rz);

    return rot_z * rot_y * rot_x;
}

glm::mat4 eqemu::math::create_translate_matrix(float tx, float ty, float tz) {
    glm::mat4 trans(1.0f);
    trans[3][0] = tx;
    trans[3][1] = ty;
    trans[3][2] = tz;

    return trans;
}

glm::mat4 eqemu::math::create_scale_matrix(float sx, float sy, float sz) {
    glm::mat4 scale(1.0f);
    scale[0][0] = sx;
    scale[1][1] = sy;
    scale[2][2] = sz;
    return scale;
}
