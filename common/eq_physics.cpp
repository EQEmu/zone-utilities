#include <vector>
#include <memory>

#include <btBulletDynamicsCommon.h>

#include "eq_physics.h"

struct EQPhysics::impl {
	std::unique_ptr<WaterMap> water_map;
	std::unique_ptr<btBroadphaseInterface> collision_broadphase;
	std::unique_ptr<btDefaultCollisionConfiguration> collision_config;
	std::unique_ptr<btCollisionDispatcher> collision_dispatch;
	std::unique_ptr<btSequentialImpulseConstraintSolver> collision_solver;
	std::unique_ptr<btDiscreteDynamicsWorld> collision_world;

	std::unique_ptr<btTriangleMesh> collidable_mesh;
	std::unique_ptr<btTriangleMesh> non_collidable_mesh;
	std::unique_ptr<btBvhTriangleMeshShape> collidable_mesh_shape;
	std::unique_ptr<btBvhTriangleMeshShape> non_collidable_mesh_shape;
	std::unique_ptr<btRigidBody> collidable_rb;
	std::unique_ptr<btRigidBody> non_collidable_rb;
};

EQPhysics::EQPhysics() {
	imp = new impl;

	imp->collision_config.reset(new btDefaultCollisionConfiguration());

	imp->collision_dispatch.reset(new btCollisionDispatcher(imp->collision_config.get()));

	imp->collision_broadphase.reset(new btDbvtBroadphase());

	imp->collision_solver.reset(new btSequentialImpulseConstraintSolver());

	imp->collision_world.reset(new btDiscreteDynamicsWorld(imp->collision_dispatch.get(),
		imp->collision_broadphase.get(),
		imp->collision_solver.get(),
		imp->collision_config.get()));

	imp->collision_world->setGravity(btVector3(0, -9.8f, 0.0));
}

EQPhysics::~EQPhysics() {
	for(int i = 0; i < imp->collision_world->getNumCollisionObjects(); ++i) {
		btCollisionObject* obj = imp->collision_world->getCollisionObjectArray()[i];
		btRigidBody* body = btRigidBody::upcast(obj);

		if(body && body->getMotionState()) {
			delete body->getMotionState();
		}

		imp->collision_world->removeCollisionObject(obj);
	}

	imp->collision_world.release();
	imp->collision_solver.release();
	imp->collision_config.release();
	imp->collision_dispatch.release();
	imp->collision_broadphase.release();
	delete imp;
}

void EQPhysics::SetCollidableWorld(const std::vector<glm::vec3>& verts, const std::vector<unsigned int>& inds) {
	if(verts.size() == 0 || imp->collidable_rb) {
		return;
	}

	imp->collidable_mesh.reset(new btTriangleMesh());
	//add our triangles...
	int face_count = (int)inds.size() / 3;
	for(int i = 0; i < face_count; ++i) {
		unsigned int i1 = inds[i * 3 + 0];
		unsigned int i2 = inds[i * 3 + 1];
		unsigned int i3 = inds[i * 3 + 2];

		btVector3 v1(verts[i1].x, verts[i1].y, verts[i1].z);
		btVector3 v2(verts[i2].x, verts[i2].y, verts[i2].z);
		btVector3 v3(verts[i3].x, verts[i3].y, verts[i3].z);

		imp->collidable_mesh->addTriangle(v1, v2, v3);
	}

	imp->collidable_mesh_shape.reset(new btBvhTriangleMeshShape(imp->collidable_mesh.get(), true, true));

	btTransform origin_transform;
	origin_transform.setIdentity();
	origin_transform.setOrigin(btVector3(0.0f, 0.0f, 0.0f));

	btRigidBody::btRigidBodyConstructionInfo rb_info(0.0f, nullptr, imp->collidable_mesh_shape.get(), btVector3(0.0f, 0.0f, 0.0f));
	imp->collidable_rb.reset(new btRigidBody(rb_info));

	imp->collision_world->addRigidBody(imp->collidable_rb.get(), (short)CollidableWorld, (short)CollidableWorld);
}

void EQPhysics::SetNonCollidableWorld(const std::vector<glm::vec3>& verts, const std::vector<unsigned int>& inds) {
	if(verts.size() == 0 || imp->non_collidable_rb) {
		return;
	}

	imp->non_collidable_mesh.reset(new btTriangleMesh());
	//add our triangles...
	int face_count = (int)inds.size() / 3;
	for(int i = 0; i < face_count; ++i) {
		unsigned int i1 = inds[i * 3 + 0];
		unsigned int i2 = inds[i * 3 + 1];
		unsigned int i3 = inds[i * 3 + 2];

		btVector3 v1(verts[i1].x, verts[i1].y, verts[i1].z);
		btVector3 v2(verts[i2].x, verts[i2].y, verts[i2].z);
		btVector3 v3(verts[i3].x, verts[i3].y, verts[i3].z);

		imp->non_collidable_mesh->addTriangle(v1, v2, v3);
	}

	imp->non_collidable_mesh_shape.reset(new btBvhTriangleMeshShape(imp->non_collidable_mesh.get(), true, true));

	btTransform origin_transform;
	origin_transform.setIdentity();
	origin_transform.setOrigin(btVector3(0.0f, 0.0f, 0.0f));

	btRigidBody::btRigidBodyConstructionInfo rb_info(0.0f, nullptr, imp->non_collidable_mesh_shape.get(), btVector3(0.0f, 0.0f, 0.0f));
	imp->non_collidable_rb.reset(new btRigidBody(rb_info));

	imp->collision_world->addRigidBody(imp->non_collidable_rb.get(), (short)NonCollidableWorld, (short)NonCollidableWorld);
}

void EQPhysics::SetWaterMap(WaterMap *w) {
	imp->water_map.reset(w);
}

bool EQPhysics::CheckLOS(const glm::vec3 &src, const glm::vec3 &dest) const {
	btVector3 src_bt(src.x, src.y, src.z);
	btVector3 dest_bt(dest.x, dest.y, dest.z);

	btCollisionWorld::ClosestRayResultCallback los_hit(src_bt, dest_bt);
	los_hit.m_collisionFilterGroup = (short)CollidableWorld;
	los_hit.m_collisionFilterMask = (short)CollidableWorld;

	imp->collision_world->rayTest(src_bt, dest_bt, los_hit);

	if(los_hit.hasHit()) {
		return false;
	}

	return true;
}

float EQPhysics::FindBestFloor(const glm::vec3 &start, glm::vec3 *result, glm::vec3 *normal) const {
	btVector3 from(start.x, start.y + 1.0f, start.z);
	btVector3 to(start.x, BEST_FLOOR_INVALID, start.z);

	btCollisionWorld::ClosestRayResultCallback hit_below(from, to);
	hit_below.m_collisionFilterGroup = (short)CollidableWorld;
	hit_below.m_collisionFilterMask = (short)CollidableWorld;

	imp->collision_world->rayTest(from, to, hit_below);

	if(hit_below.hasHit()) {
		if(normal) {
			normal->x = hit_below.m_hitNormalWorld.getX();
			normal->y = hit_below.m_hitNormalWorld.getY();
			normal->z = hit_below.m_hitNormalWorld.getZ();
		}

		btVector3 p = from.lerp(to, hit_below.m_closestHitFraction);

		if(result) {
			result->x = p.getX();
			result->y = p.getY();
			result->z = p.getZ();
		}

		return p.getY();
	}

	to.setY(BEST_CEIL_INVALID);
	btCollisionWorld::ClosestRayResultCallback hit_above(from, to);
	hit_above.m_collisionFilterGroup = (short)CollidableWorld;
	hit_above.m_collisionFilterMask = (short)CollidableWorld;

	imp->collision_world->rayTest(from, to, hit_above);

	if(hit_above.hasHit()) {
		if(normal) {
			normal->x = hit_above.m_hitNormalWorld.getX();
			normal->y = hit_above.m_hitNormalWorld.getY();
			normal->z = hit_above.m_hitNormalWorld.getZ();
		}

		btVector3 p = from.lerp(to, hit_above.m_closestHitFraction);

		if(result) {
			result->x = p.getX();
			result->y = p.getY();
			result->z = p.getZ();
		}

		return p.getY();
	}

	return BEST_FLOOR_INVALID;
}

WaterRegionType EQPhysics::ReturnRegionType(const glm::vec3 &pos) const {
	if(!imp->water_map) {
		return RegionTypeNormal;
	}

	return imp->water_map->ReturnRegionType(pos.x, pos.z, pos.y);
}

bool EQPhysics::InWater(const glm::vec3 &pos) const {
	if(!imp->water_map) {
		return false;
	}

	return imp->water_map->InWater(pos.x, pos.z, pos.y);
}

bool EQPhysics::InVWater(const glm::vec3 &pos) const {
	if(!imp->water_map) {
		return false;
	}

	return imp->water_map->InVWater(pos.x, pos.z, pos.y);
}

bool EQPhysics::InLava(const glm::vec3 &pos) const {
	if(!imp->water_map) {
		return false;
	}

	return imp->water_map->InLava(pos.x, pos.z, pos.y);
}

bool EQPhysics::InLiquid(const glm::vec3 &pos) const {
	if(!imp->water_map) {
		return false;
	}

	return imp->water_map->InLiquid(pos.x, pos.z, pos.y);
}