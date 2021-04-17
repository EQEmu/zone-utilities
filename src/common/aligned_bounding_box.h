#ifndef EQEMU_ALIGNED_BOUNDNG_BOX_H
#define EQEMU_ALIGNED_BOUNDNG_BOX_H

#include "eq_math.h"

class AlignedBoundingBox {
public:
    AlignedBoundingBox() : center_(0.0), radius_(0.0), min_(0.0), max_(0.0) {}
    AlignedBoundingBox(const glm::vec3& min, const glm::vec3& max)
        : center_((max.x + min.x) / 2, (max.y + min.y) / 2, (max.z + min.z) / 2),
          radius_(max.x - center_.x, max.y - center_.y, max.z - center_.z), min_(min), max_(max) {}
    AlignedBoundingBox(const glm::vec3& c, const float r)
        : center_(c), radius_(r, r, r), min_(c.x - r, c.y - r, c.z - r), max_(c.x + r, c.y + r, c.z + r) {}
    AlignedBoundingBox(const glm::vec3& c, const float rx, float ry, float rz)
        : center_(c), radius_(rx, ry, rz), min_(c.x - rx, c.y - ry, c.z - rz), max_(c.x + rx, c.y + ry, c.z + rz) {}
    ~AlignedBoundingBox() {}

    bool Contains(const glm::vec3& p) const {
        return p.x >= min_.x && p.x <= max_.x && p.y >= min_.y && p.y <= max_.y && p.z >= min_.z && p.z <= max_.z;
    }

    bool IntersectsAABB(const AlignedBoundingBox& b) const {
        if(max_.x < b.min_.x)
            return false;
        if(min_.x > b.max_.x)
            return false;
        if(max_.y < b.min_.y)
            return false;
        if(min_.y > b.max_.y)
            return false;
        if(max_.z < b.min_.z)
            return false;
        if(min_.z > b.max_.z)
            return false;
        return true;
    }

    bool IntersectsSphere(const glm::vec3& center, const float diameter) const {
        float dist = 0.0;

        if(center.x < min_.x)
            dist += (center.x - min_.x) * (center.x - min_.x);
        else if(center.x > max_.x)
            dist += (center.x - max_.x) * (center.x - max_.x);
        if(center.y < min_.y)
            dist += (center.y - min_.y) * (center.y - min_.y);
        else if(center.y > max_.y)
            dist += (center.y - max_.y) * (center.y - max_.y);
        if(center.z < min_.z)
            dist += (center.z - min_.z) * (center.z - min_.z);
        else if(center.z > max_.z)
            dist += (center.z - max_.z) * (center.z - max_.z);
        return dist <= diameter;
    }

    glm::vec3 center_;
    glm::vec3 radius_;
    glm::vec3 max_;
    glm::vec3 min_;
};

#endif
