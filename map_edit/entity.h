#ifndef EQEMU_MAP_EDIT_ENTITY_H
#define EQEMU_MAP_EDIT_ENTITY_H

#define GLM_FORCE_RADIANS
#include <glm.hpp>

class Entity
{
public:
	Entity() { }
	~Entity() { }

	virtual void Draw() = 0;

	void SetLocation(const glm::vec3 &pos) { m_loc = pos; }
	const glm::vec3& GetLocation() const { return m_loc; }

	void SetTint(const glm::vec4 &tint) { m_tint = tint; }
	const glm::vec4& GetTint() const { return m_tint; }
protected:
	glm::vec3 m_loc;
	glm::vec4 m_tint;
};

#endif
