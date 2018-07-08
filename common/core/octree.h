#ifndef EQEMU_COMMON_OCTREE_H
#define EQEMU_COMMON_OCTREE_H

#include "vec3.hpp"
#include <unordered_map>
#include <functional>
#include <map>

template<typename T, int elem_t = 4, int max_depth = 7>
class Octree
{
public:
	typedef std::function<void(const glm::vec3&, T*)> TraverseCallback;

	class AABB
	{
	public:
		AABB() 
			: center_(0.0), 
			radius_(0.0), 
			min_(0.0), 
			max_(0.0) { }
		AABB(const glm::vec3 &min, const glm::vec3 &max) 
			: center_((max.x + min.x) / 2, (max.y + min.y) / 2, (max.z + min.z) / 2), 
			radius_(max.x - center_.x, max.y - center_.y, max.z - center_.z),
			min_(min), 
			max_(max) { }
		AABB(const glm::vec3 &c, const float r) 
			: center_(c), 
			radius_(r, r, r), 
			min_(c.x - r, c.y - r, c.z - r), 
			max_(c.x + r, c.y + r, c.z + r) { }
		AABB(const glm::vec3 &c, const float rx, float ry, float rz) 
			: center_(c), 
			radius_(rx, ry, rz), 
			min_(c.x - rx, c.y - ry, c.z - rz), 
			max_(c.x + rx, c.y + ry, c.z + rz) { }
		~AABB() { }

		bool Contains(const glm::vec3 &p) const {
			return p.x >= min_.x && p.x <= max_.x && p.y >= min_.y && p.y <= max_.y && p.z >= min_.z && p.z <= max_.z;
		}
		
		bool IntersectsAABB(const AABB &b) const {
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
		
		bool IntersectsSphere(const glm::vec3 &center, const float diameter) const {
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
		friend class OctreeNode;
	};

	class OctreeNode
	{
	public:
		OctreeNode() {
			parent_ = nullptr;
			children_ = nullptr;
			tree_ = nullptr;
		}

		~OctreeNode() {
			if(children_) {
				delete[] children_;
			}
		}

	private:
		inline bool IsLeaf() const { 
			return children_ == nullptr;
		}

		int WhichChildToCheck(const glm::vec3 &pos) const {
			int ret = 0;
			auto &center = bounds_.center_;
			if(pos.x >= center.x) {
				ret |= 4;
			}

			if(pos.y >= center.y) {
				ret |= 2;
			}

			if(pos.z >= center.z) {
				ret |= 1;
			}

			return ret;
		}

		void Split() {
			children_ = new OctreeNode[8];
			for(int i = 0; i < 8; ++i) {
				OctreeNode *child = &children_[i];

				glm::vec3 center = bounds_.center_;
				center.x += (bounds_.radius_.x / (i & 4 ? 2.0f : -2.0f));
				center.y += (bounds_.radius_.y / (i & 2 ? 2.0f : -2.0f));
				center.z += (bounds_.radius_.z / (i & 1 ? 2.0f : -2.0f));
				child->bounds_.center_ = center;

				float rad_x = bounds_.radius_.x / 2.0f;
				float rad_y = bounds_.radius_.y / 2.0f;
				float rad_z = bounds_.radius_.z / 2.0f;

				child->bounds_.radius_ = glm::vec3(rad_x, rad_y, rad_z);
				child->bounds_.min_ = glm::vec3(center.x - rad_x, center.y - rad_y, center.z - rad_z);
				child->bounds_.max_ = glm::vec3(center.x + rad_x, center.y + rad_y, center.z + rad_z);

				child->tree_ = tree_;
				child->parent_ = this;
				child->depth_ = depth_ + 1;
			}

			for(auto &element : elements_) {
				int child = WhichChildToCheck(element.second);
				children_[child].elements_[element.first] = element.second;
				tree_->data_node_lookup_[element.first] = &children_[child];
			}

			elements_.clear();
		}

		void Consolidate() {
			int c = 0;
			for(int i = 0; i < 8; ++i) {
				c += (int)children_[i].elements_.size();
			}

			if(c == 0) {
				delete[] children_;
				children_ = nullptr;
				if(parent_) {
					parent_->Consolidate();
				}
			}
		}

		const OctreeNode *Get(const glm::vec3 &pos) const {
			if(IsLeaf()) {
				return this;
			}

			int child = WhichChildToCheck(pos);
			return children_[child].Get(pos);
		}

		OctreeNode* Insert(const glm::vec3 &pos, T* val) {
			if(!IsLeaf()) {
				int child = WhichChildToCheck(pos);
				return children_[child].Insert(pos, val);
			}

			if(elements_.size() < elem_t  || depth_ >= max_depth) {
				elements_[val] = pos;
				return this;
			} else {
				Split();
				int child = WhichChildToCheck(pos);
				return children_[child].Insert(pos, val);
			}
		}

		OctreeNode* Relocate(const glm::vec3 &pos, T* val) {
			if(bounds_.Contains(pos)) {
				int child = WhichChildToCheck(pos);
				return children_[child].Insert(pos, val);
			}

			if(parent_) {
				return parent_->Relocate(pos, val);
			}
			return nullptr;
		}

		void TraverseSelection(const AABB &bb, const TraverseCallback &cb) {
			if(!IsLeaf()) {
				for(int i = 0; i < 8; ++i) {
					if(children_[i].bounds_.IntersectsAABB(bb)) {
						children_[i].TraverseSelection(bb, cb);
					}
				}
			} else {
				for(auto &element : elements_) {
					if(bb.Contains(element.second)) {
						cb(element.second, element.first);
					}
				}
			}
		}

		void TraverseRange(const glm::vec3 &pos, float range, const TraverseCallback &cb) {
			if(!IsLeaf()) {
				for(int i = 0; i < 8; ++i) {
					if(children_[i].bounds_.IntersectsSphere(pos, range)) {
						children_[i].TraverseRange(pos, range, cb);
					}
				}
			}
			else {
				for(auto &element : elements_) {
					float dist_x = element.second.x - pos.x;
					float dist_y = element.second.y - pos.y;
					float dist_z = element.second.z - pos.z;
					float dist = (dist_x * dist_x) + (dist_y * dist_y) + (dist_z * dist_z);

					if(dist <= range) {
						cb(element.second, element.first);
					}
				}
			}
		}

		AABB bounds_;
		OctreeNode * parent_;
		OctreeNode * children_;
		Octree * tree_;
		int depth_;
		friend class Octree;

	public:
		std::map<T*, glm::vec3> elements_;
	};

	Octree() {
		root_ = nullptr;
	}

	Octree(const AABB &bounds) {
		root_ = nullptr;
		bounds_ = bounds;
	}

	~Octree() {
		if(root_) {
			delete root_;
		}
	}

	void Reset(const AABB &bounds) {
		data_node_lookup_.clear();
		if(root_) {
			delete root_;
		}

		bounds_ = bounds;
	}

	const OctreeNode *GetNode(const glm::vec3 &pos) const {
		if(root_) {
			return root_->Get(pos);
		}

		return nullptr;
	}

	void TraverseSelection(const AABB &bb, const TraverseCallback &cb) {
		if(root_) {
			root_->TraverseSelection(bb, cb);
		}
	}

	void TraverseRange(const glm::vec3 &pos, float range, const TraverseCallback &cb) {
		if(root_) {
			root_->TraverseRange(pos, range * range, cb);
		}
	}

	void Insert(const glm::vec3 &pos, T* val) {
		if(root_) {
			//check if we fit within the root node
			//if not uh oh we can't store this!
			if(!root_->bounds_.Contains(pos)) {
				return;
			}

			OctreeNode *node = root_->Insert(pos, val);
			if(node) {
				data_node_lookup_[val] = node;
			}
		} else {
			root_ = new OctreeNode;
			root_->bounds_ = bounds_;
			root_->tree_ = this;
			root_->elements_[val] = pos;
			root_->depth_ = 0;
			data_node_lookup_[val] = root_;
		}
	}

	void Update(const glm::vec3 &pos, T* val) {
		//find val
		//check to see if val has moved out of it's node
		//if so we need to call a special function to relocate it
		auto &iter = data_node_lookup_.find(val);
		if(iter != data_node_lookup_.end()) {
			OctreeNode *cur = iter->second;
			if(cur->bounds_.Contains(pos)) {
				cur->elements_[val] = pos;
				return;
			}
			else {
				cur->elements_.erase(val);
				OctreeNode *node = nullptr;
				if(cur->parent_) {
					node = cur->parent_->Relocate(pos, val);
				}
				if(node) {
					data_node_lookup_[val] = node;
				} else {
					data_node_lookup_.erase(val);
				}
				return;
			}
		} else {
			//doesn't exist in tree, might as well add it
			Insert(pos, val);
		}
	}

	void Delete(T* val) {
		auto &iter = data_node_lookup_.find(val);
		if(iter != data_node_lookup_.end()) {
			OctreeNode *node = iter->second;
			node->elements_.erase(val);
			data_node_lookup_.erase(val);
			if(node->parent_)
				node->parent_->Consolidate();
		}
	}

private:
	OctreeNode *root_;
	AABB bounds_;
	std::unordered_map<T*, OctreeNode*> data_node_lookup_;
};

#endif
