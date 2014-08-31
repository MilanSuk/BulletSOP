/**
BulletSOP v2.0
Copyright (c)2010-2013 Suk Milan. All Rights Reserved.
This software is provided 'as-is', without any express or implied warranty.
In no event will the authors be held liable for any damages arising from the use of this software.
This file may not be redistributed in whole or significant part.
This notice may not be removed or altered from any source distribution.
*/
#pragma once
#include "btBulletDynamicsCommon.h"
//#include "BulletSoftBody/btSoftRigidDynamicsWorld.h"

#include "BImpact.h"
#include "BConstraint.h"
#include "BRigidBody.h"
#include "BCollisionFilter.h"

#include "SOP_Solver.h"


/**
Class represent Bullet World
Includes objects, constraints, forces and simulation settings
*/
class BWorld
{
private:
	bool m_deleteThread;
	//btSoftRigidDynamicsWorld* m_dynamicsWorld;	
	btDiscreteDynamicsWorld* m_dynamicsWorld;

	btDefaultCollisionConfiguration* m_collisionConfiguration;
	btCollisionDispatcher* m_dispatcher;
	btBroadphaseInterface* m_broadphase;
	btConstraintSolver* m_solver;
	class btThreadSupportInterface* m_threadSupportSolver;

	BImpact* m_impact;
	BCollisionFilter* m_collisionFilter;

	BRigidBody* m_floorBody;
	bool m_floorBodyInWorld;

	btAlignedObjectArray<BRigidBody*> m_objs;
	btAlignedObjectArray<BConstraint*> m_constrs;
	btAlignedObjectArray<bool> m_remove_constrs;	///stores If constraint exists and has removed	/////when constraint is broken, this is true

	btAlignedObjectArray<btCollisionShape*> m_shapes;
	bool m_share_shapes;

	static btVector3 s_TOLER_VEC;
	static float s_TOLER;

public:
	BWorld(	bool deleteThread,
			bool share_shapes,
			int collisionConfiguration_defaultMaxPersistentManifoldPoolSize = 4096,
			int collisionConfiguration_defaultMaxCollisionAlgorithmPoolSize = 4096,

			int use_broadphase = SOP_Solver::BROAD_DBVT,
			UT_Vector3 broadphase_WorldCenter = UT_Vector3(0, 0, 0),
			UT_Vector3 broadphase_WorldSize = UT_Vector3(100, 100, 100),
			int broadphase_maxSmallProxies = 100000,
			int broadphase_maxLargeProxies = 10,
			int broadphase_maxPairsPerBody = 24,
			int broadphase_maxBodiesPerCell = 8,
			float broadphase_cellFactorAABB = 1.0f,

			int use_solver = SOP_Solver::SOLVER_SEQ,
			int m_solver_numThreads = 4);

	virtual ~BWorld(void);


public:
	BImpact* getImpact();
	BCollisionFilter* getCollisionFilter();

	BRigidBody* addBox(int index, UT_Vector3 t, UT_Vector3 r, UT_Vector3 cog_r, UT_Vector3 half_box_size, BRigidBody* compound);
	BRigidBody* addSphere(int index, UT_Vector3 t, UT_Vector3 r, UT_Vector3 cog_r, float rad, BRigidBody* compound);
	BRigidBody* addCapsule(int index, UT_Vector3 t, UT_Vector3 r, UT_Vector3 cog_r, float rad, float h, BRigidBody* compound);
	BRigidBody* addCone(int index, UT_Vector3 t, UT_Vector3 r, UT_Vector3 cog_r, float rad, float h, BRigidBody* compound);
	BRigidBody* addCylinder(int index, UT_Vector3 t, UT_Vector3 r, UT_Vector3 cog_r, float rad, float h, BRigidBody* compound);
	BRigidBody* addCompound(int index, UT_Vector3 t, UT_Vector3 r, UT_Vector3 cog_r, BRigidBody* compound);
	BRigidBody* addConvex(int index, UT_Vector3 t, UT_Vector3 r, UT_Vector3 cog_r, UT_Vector3 cog, const SObject &object, BRigidBody* compound, bool save_vertexes);
	BRigidBody* addDeform(int index, UT_Vector3 t, UT_Vector3 r, UT_Vector3 cog_r, UT_Vector3 cog, const SObject &object, BRigidBody* compound, bool save_vertexes);
	//btSoftBody* addSoft(int index, UT_Vector3 t, UT_Vector3 r, UT_Vector3 cog_r, UT_Vector3 cog, const SObject &object);

	BRigidBody* getBody(int i);
	void startUpdateObjects();	///sets all objects update to false
	void endUpdateObjects();	///iterate over all objects and deletes object with false update

	BConstraint* addConstraints(int index, BRigidBody* body_1, BRigidBody* body_2, UT_Vector3 wpos_1, UT_Vector3 wpos_2);
	BConstraint* getConstraints(int i, BRigidBody* body_1, BRigidBody* body_2);
	bool hasConstraintBroken(int i);
	void startUpdateConstraints();	///sets all constraints update to false
	void endUpdateConstraints();	///iterate over all constraints and deletes constraint with false update
	void breakConstraints(int fps, int substeps);	///iterate over all constraints and deletes constraints which have been break it(are weak)

	void applyForceRad(UT_Vector3 world_pos, int type, UT_Vector3 dir_normalized, float size, float rad, float fade);	//apply force on all bodies in radius
	void addObjectForce(BRigidBody* obj, UT_Vector3 world_pos, int type, UT_Vector3 dir_normalized, float size);	///adds force to "m_forces" array
	void clearForces();				///clear "m_forces" array
	void uploadForcesToObjects();	///send forces from "m_forces" array to bullet

	void stepWorld(float stepSize, int i_substep, int num_substep);
	void setGravity(UT_Vector3 gravity);
	void backUpVelocities();	//save current velocities to old

	void convert_world_to_parent(BRigidBody* body, BRigidBody* compound);
	void convert_parent_to_world(BRigidBody* body);
	void convert_parent_to_parent(BRigidBody* body, BRigidBody* compound);

	void cleanCollisionWithDeformBody(BRigidBody* body);	//clears pair in our world(needs to be call after updating points in deform body)

	void addFloor();
	void removeFloor();


	void addCollisionShape(btCollisionShape* sh);

	btCollisionShape* findBoxShape(btVector3 size) const;
	btCollisionShape* findSphereShape(float rad) const;
	btCollisionShape* findCylinderShape(btVector3 s) const;


private:
	void addOBJ(int index, BRigidBody* body);	///adds body to "m_objs" array
	void deleteOBJ(int i);						///deletes body from "m_objs" array

	void addCSTR(int index, BConstraint* c);	///adds constaint to "m_constrs" array
	void deleteCSTR(int i);						///deletes constaint from "m_constrs" array

	void setKinematicPos(float rel);			///aproximate static bodies
	void blendConvexVertexes(int num_substep);
	
	///add rigid body to world or parent body(compound)
	///@return new rigid body
	BRigidBody* addRigidObject(int type, btCollisionShape* colShape, int index, UT_Vector3 t, UT_Vector3 r, UT_Vector3 cog_r, BRigidBody* compound);

	void createFloor();
	void deleteFloor();
};



///converts btVector3 to UT_Vector3
inline UT_Vector3 get_hdk_V3(btVector3 v)
{
	return UT_Vector3(v[0], v[1], v[2]);
}
///converts UT_Vector3 to btVector3
inline btVector3 get_bullet_V3(UT_Vector3 v)
{
	return btVector3(v[0], v[1], v[2]);
}

///check If "a" is inside "b" with toler
inline bool insideVector(btVector3 a, btVector3 b, btVector3 tolerB)
{
	btVector3 bmin = b-tolerB;
	btVector3 bmax = b+tolerB;

	return	a[0] > bmin[0] && a[1] > bmin[1] && a[2] > bmin[2] &&
			a[0] < bmax[0] && a[1] < bmax[1] && a[2] < bmax[2];
}
