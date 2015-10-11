#ifndef EQEMU_MAP_VIEW_VOLUME_NAVIGATION_H
#define EQEMU_MAP_VIEW_VOLUME_NAVIGATION_H

#include "aligned_bounding_box.h"
#include "line_model.h"

enum VolumeNavigationType
{
	VolumeNavOutOfBounds,
	VolumeNavWalkable,
	VolumeNavSwimmable,
	VolumeNavFlyable,
};

class VolumeNavigationNode
{
public:
	VolumeNavigationNode() {
		m_children = nullptr;
		m_parent = nullptr;
		m_type = VolumeNavOutOfBounds;
	}

	~VolumeNavigationNode() { 
		if(m_children) {
			delete[] m_children;
		}
	}

	void AppendToModel(LineModel *vb) {
		vb->AddBox(m_aabb.min_, m_aabb.max_);

		if(m_children) {
			for(int i = 0; i < 8; ++i) {
				m_children[i].AppendToModel(vb);
			}
		}
	}

	void SubDivide(int n, int depth)
	{
		if(n <= 0) {
			return;
		}

		if(!m_children) {
			m_children = new VolumeNavigationNode[8];

			for(int i = 0; i < 8; ++i) {
				VolumeNavigationNode *child = &m_children[i];

				glm::vec3 center = m_aabb.center_;
				center.x += (m_aabb.radius_.x / (i & 4 ? 2.0f : -2.0f));
				center.y += (m_aabb.radius_.y / (i & 2 ? 2.0f : -2.0f));
				center.z += (m_aabb.radius_.z / (i & 1 ? 2.0f : -2.0f));
				child->m_aabb.center_ = center;

				float rad_x = m_aabb.radius_.x / 2.0f;
				float rad_y = m_aabb.radius_.y / 2.0f;
				float rad_z = m_aabb.radius_.z / 2.0f;

				child->m_aabb.radius_ = glm::vec3(rad_x, rad_y, rad_z);
				child->m_aabb.min_ = glm::vec3(center.x - rad_x, center.y - rad_y, center.z - rad_z);
				child->m_aabb.max_ = glm::vec3(center.x + rad_x, center.y + rad_y, center.z + rad_z);

				child->m_parent = this;
				child->m_depth = depth + 1;
			}
		}

		for(int i = 0; i < 8; ++i) {
			m_children[i].SubDivide(n - 1, depth + 1);
		}
	}

	inline bool IsLeaf() const { 
		return m_children == nullptr;
	}

private:
	friend class VolumeNavigationGraph;

	AlignedBoundingBox m_aabb;
	VolumeNavigationType m_type;
	VolumeNavigationNode *m_children;
	VolumeNavigationNode *m_parent;
	int m_depth;
};

class VolumeNavigationGraph
{
public:
	VolumeNavigationGraph(const glm::vec3 &min, const glm::vec3 &max) 
	{
		m_root = new VolumeNavigationNode();
		m_root->m_aabb = AlignedBoundingBox(min, max);
		m_root->m_depth = 0;
	}

	~VolumeNavigationGraph() 
	{
		delete m_root;
	}

	void Subdivide(int n) {
		m_root->SubDivide(n, 0);
	}

	void CreateModel(LineModel *vb) {
		m_root->AppendToModel(vb);
	}

private:
	VolumeNavigationNode *m_root;
};

#endif
