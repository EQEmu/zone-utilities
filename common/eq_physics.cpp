#include <vector>
#include <memory>
#include <map>

#include <btBulletDynamicsCommon.h>

#include "log_macros.h"
#include "eq_physics.h"

struct btMeshInfo
{
	btMeshInfo(btTriangleMesh* mesh_in, btBvhTriangleMeshShape* mesh_shape_in, btRigidBody* rb_in) {
		mesh.reset(mesh_in);
		mesh_shape.reset(mesh_shape_in);
		rb.reset(rb_in);
	}

	std::unique_ptr<btTriangleMesh> mesh;
	std::unique_ptr<btBvhTriangleMeshShape> mesh_shape;
	std::unique_ptr<btRigidBody> rb;
};

struct EQPhysics::impl {
	std::unique_ptr<WaterMap> water_map;
	std::unique_ptr<btBroadphaseInterface> collision_broadphase;
	std::unique_ptr<btDefaultCollisionConfiguration> collision_config;
	std::unique_ptr<btCollisionDispatcher> collision_dispatch;
	std::unique_ptr<btSequentialImpulseConstraintSolver> collision_solver;
	std::unique_ptr<btDiscreteDynamicsWorld> collision_world;
	std::unique_ptr<std::map<std::string, btMeshInfo>> entity_info;
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

	imp->entity_info.reset(new std::map<std::string, btMeshInfo>());
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

void EQPhysics::SetWaterMap(WaterMap *w) {
	imp->water_map.reset(w);
}

WaterMap *EQPhysics::GetWaterMap()
{
	return imp->water_map.get();
}

void EQPhysics::RegisterMesh(const std::string &ident, const std::vector<glm::vec3>& verts, const std::vector<unsigned int>& inds, const glm::vec3 &pos, EQPhysicsFlags flag) {
	UnregisterMesh(ident);

	if (verts.size() == 0 || inds.size() == 0) {
		return;
	}

	btTriangleMesh *mesh = new btTriangleMesh();

	int face_count = (int)inds.size() / 3;
	for (int i = 0; i < face_count; ++i) {
		unsigned int i1 = inds[i * 3 + 0];
		unsigned int i2 = inds[i * 3 + 1];
		unsigned int i3 = inds[i * 3 + 2];

		btVector3 v1(verts[i1].x, verts[i1].y, verts[i1].z);
		btVector3 v2(verts[i2].x, verts[i2].y, verts[i2].z);
		btVector3 v3(verts[i3].x, verts[i3].y, verts[i3].z);

		mesh->addTriangle(v1, v2, v3);
	}

	btBvhTriangleMeshShape *mesh_shape = new btBvhTriangleMeshShape(mesh, true, true);

	btTransform origin_transform;
	origin_transform.setIdentity();
	origin_transform.setOrigin(btVector3(pos.x, pos.y, pos.z));

	btDefaultMotionState* motionState = new btDefaultMotionState(origin_transform);
	btRigidBody::btRigidBodyConstructionInfo rb_info(0.0f, motionState, mesh_shape, btVector3(0.0f, 0.0f, 0.0f));
	btRigidBody *rb = new btRigidBody(rb_info);

	imp->collision_world->addRigidBody(rb, (short)flag, (short)flag);
	imp->entity_info->insert(std::make_pair(ident, btMeshInfo(mesh, mesh_shape, rb)));
}

void EQPhysics::UnregisterMesh(const std::string &ident) {
	auto iter = imp->entity_info->find(ident);
	if (iter != imp->entity_info->end()) {
		imp->entity_info->erase(iter);

		for (int i = 0; i < imp->collision_world->getNumCollisionObjects(); ++i) {
			btCollisionObject* obj = imp->collision_world->getCollisionObjectArray()[i];
			btRigidBody* body = btRigidBody::upcast(obj);

			if (body == iter->second.rb.get()) {
				if (body && body->getMotionState()) {
					delete body->getMotionState();
				}

				imp->collision_world->removeCollisionObject(obj);
			}
		}
	}
}

void EQPhysics::Step()
{
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

bool EQPhysics::GetRaycastClosestHit(const glm::vec3 & src, const glm::vec3 & dest, glm::vec3 &hit, std::string *name, EQPhysicsFlags flag) const
{
	btVector3 src_bt(src.x, src.y, src.z);
	btVector3 dest_bt(dest.x, dest.y, dest.z);

	btCollisionWorld::ClosestRayResultCallback ray_hit(src_bt, dest_bt);
	ray_hit.m_collisionFilterGroup = (short)flag;
	ray_hit.m_collisionFilterMask = (short)flag;

	imp->collision_world->rayTest(src_bt, dest_bt, ray_hit);

	if (ray_hit.hasHit()) {
		hit.x = ray_hit.m_hitPointWorld.x();
		hit.y = ray_hit.m_hitPointWorld.y();
		hit.z = ray_hit.m_hitPointWorld.z();
		if (name) {
			GetEntityHit(ray_hit.m_collisionObject, *name);
		}
		return true;
	}

	hit.x = 0.0f;
	hit.y = 0.0f;
	hit.z = 0.0f;
	return false;
}

float EQPhysics::FindBestFloor(const glm::vec3 &start, glm::vec3 *result, glm::vec3 *normal) const {
	btVector3 from(start.x, start.y + 1.0f, start.z);
	btVector3 to(start.x, -FLT_MAX, start.z);

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

	to.setY(FLT_MAX);
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

	return -FLT_MAX;
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

bool EQPhysics::IsUnderworld(const glm::vec3 &point) const {
	btVector3 from(point.x, point.y + 1.0f, point.z);
	btVector3 to(point.x, -FLT_MAX, point.z);

	btCollisionWorld::AllHitsRayResultCallback hit_below(from, to);
	hit_below.m_collisionFilterGroup = (short)CollidableWorld;
	hit_below.m_collisionFilterMask = (short)CollidableWorld;

	imp->collision_world->rayTest(from, to, hit_below);

	if(hit_below.hasHit()) {
		return false;
	}

	return true;
}

void EQPhysics::GetEntityHit(const btCollisionObject *obj, std::string &out_ident) const {
	if (!obj) {
		return;
	}

	const btRigidBody* body = btRigidBody::upcast(obj);

	out_ident.clear();
	for (auto iter = imp->entity_info->begin(); iter != imp->entity_info->end(); ++iter) {
		if (iter->second.rb.get() == body) {
			out_ident = iter->first;
			return;
		}
	}
}
