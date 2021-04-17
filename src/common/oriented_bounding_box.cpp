#include "oriented_bounding_box.h"

OrientedBoundingBox::OrientedBoundingBox(const glm::vec3& pos,
                                         const glm::vec3& rot,
                                         const glm::vec3& scale,
                                         const glm::vec3& extents) {
    min_x = -extents.x;
    max_x = extents.x;

    if(min_x > max_x) {
        float t = min_x;
        min_x = max_x;
        max_x = t;
    }

    min_y = -extents.y;
    max_y = extents.y;
    if(min_y > max_y) {
        float t = min_y;
        min_y = max_y;
        max_y = t;
    }

    min_z = -extents.z;
    max_z = extents.z;
    if(min_z > max_z) {
        float t = min_z;
        min_z = max_z;
        max_z = t;
    }

    // rotate
    transformation =
        CreateRotateMatrix(rot.x * 3.14159f / 180.0f, rot.y * 3.14159f / 180.0f, rot.z * 3.14159f / 180.0f);

    // scale
    transformation = CreateScaleMatrix(scale.x, scale.y, scale.z) * transformation;

    // translate
    transformation = CreateTranslateMatrix(pos.x, pos.y, pos.z) * transformation;
    inverted_transformation = glm::inverse(transformation);
}

bool OrientedBoundingBox::ContainsPoint(const glm::vec3& p) const {
    glm::vec4 pt(p.x, p.y, p.z, 1);
    glm::vec4 box_space_p = inverted_transformation * pt;

    if(box_space_p.x >= min_x && box_space_p.x <= max_x && box_space_p.y >= min_y && box_space_p.y <= max_y &&
       box_space_p.z >= min_z && box_space_p.z <= max_z) {
        return true;
    }

    return false;
}
