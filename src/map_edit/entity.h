#ifndef EQEMU_MAP_EDIT_ENTITY_H
#define EQEMU_MAP_EDIT_ENTITY_H

#include <glm/glm.hpp>
#include <vector>

class Entity {
public:
    Entity() {
        m_tint = glm::vec4(1.0);
        m_loc = glm::vec3(0);
    }

    ~Entity() {}

    virtual void Draw() = 0;
    virtual void GetCollisionMesh(std::vector<glm::vec3>& verts, std::vector<unsigned int>& inds) {
        verts.clear();
        inds.clear();
    };

    void SetLocation(const glm::vec3& pos) { m_loc = pos; }
    const glm::vec3& GetLocation() const { return m_loc; }

    void SetTint(const glm::vec4& tint) { m_tint = tint; }
    const glm::vec4& GetTint() const { return m_tint; }

protected:
    glm::vec3 m_loc;
    glm::vec4 m_tint;
};

#endif
