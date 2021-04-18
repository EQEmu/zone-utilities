#include "distance.h"

float eqemu::math::distance(const glm::vec3& a, const glm::vec3& b) {
    float xdiff = a.x - b.x;
    float ydiff = a.y - b.y;
    float zdiff = a.z - b.z;
    return ::sqrt((xdiff * xdiff) + (ydiff * ydiff) + (zdiff * zdiff));
}

float eqemu::math::distance_no_root(const glm::vec3& a, const glm::vec3& b) {
    float xdiff = a.x - b.x;
    float ydiff = a.y - b.y;
    float zdiff = a.z - b.z;
    return (xdiff * xdiff) + (ydiff * ydiff) + (zdiff * zdiff);
}

float eqemu::math::distance_no_z(const glm::vec3& a, const glm::vec3& b) {
    float xdiff = a.x - b.x;
    float ydiff = a.y - b.y;
    return ::sqrt((xdiff * xdiff) + (ydiff * ydiff));
}

float eqemu::math::distance_no_root_no_z(const glm::vec3& a, const glm::vec3& b) {
    float xdiff = a.x - b.x;
    float ydiff = a.y - b.y;
    return (xdiff * xdiff) + (ydiff * ydiff);
}
