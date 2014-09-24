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

class SObject;


/**
Class represents Rigid body object
*/
class BRigidBody : public btRigidBody
{
private:
	int m_hId;		///bt_index
	int m_hLine;	///line on houdini properties input -> use for impact output
	bool m_update;
	int m_type;

	BRigidBody* m_compound;	///parent of this body

	UT_Vector3 old_v;
	UT_Vector3 old_av;

	btVector3 m_t_old;
	btVector3 m_t_current;

	btQuaternion m_r_old;
	btQuaternion m_r_current;

	btVector3 m_my_totalForce;
	btVector3 m_my_totalTorque;

	MyVec<btVector3> m_rel_shape;


public:
	BRigidBody(int type, const btRigidBodyConstructionInfo& constructionInfo, int hId, BRigidBody* compound);
	virtual ~BRigidBody();

public:
	void addChildShape(BRigidBody* compound);
	void removeChildShape();	///Removes this body(shape) from parent

	BRigidBody* getCompound() const;
	bool hasCompound() const;
	bool isCompound() const;


	float getMass() const;
	int getLine() const;
	int getIndex() const;
	int getOldCIndex() const;
	void setUpdate(bool update);
	UT_Vector3 getTranslateFull() const;
	UT_Vector3 getTranslateWithoutInitRot() const;
	UT_Vector3 getRotateFull() const;				///@return x,y,z rotation
	UT_Vector3 getRotateWithoutInitRot() const;
	UT_Vector3 getVelocity() const;
	UT_Vector3 getAngVelocity() const;
	UT_Vector3 getPointVelocity(UT_Vector3 world_t) const;	///@return velocity in point(world tranformation)
	UT_Vector3 getOldVelocity() const;
	UT_Vector3 getOldAngVelocity() const;
	btVector3 getAproxPos(float rel) const;		///@return aproximation between old and current position
	btQuaternion getAproxRot(float rel) const;	///@return aproximation between old and current rotation
	bool isAproxChanged() const;					///@return true, If thete is different between old and current transformation

	bool isUpdated();
	void setLine(int v);
	void setMass(float mass);
	void setVelocity(UT_Vector3 v);
	void setAngVelocity(UT_Vector3 av);
	void setCollisionMargin(float colmargin);
	void setSleeping(bool sleep);
	void setStaticBodyTransform(UT_Vector3 t, UT_Vector3 r);
	void setDynamicBodyTransform(UT_Vector3 t, UT_Vector3 r);
	void setForce(UT_Vector3 world_pos, UT_Vector3 force, int type);
	void clearMyForces();
	void uploadMyForces();
	void setPosOld(btVector3 v);
	void setPosCurrent(btVector3 v);
	void setRotOld(btQuaternion &q);
	void setRotCurrent(btQuaternion &q);
	void setOldVelocity(UT_Vector3 v);
	void setOldAngVelocity(UT_Vector3 av);
//	void setInitRotation(btMatrix3x3 &mat);

	///update for object which has hull_type==TYPE_DEFORM
	void updateDeform(UT_Vector3 cog, SObject* object);
	static void updateTriangles(UT_Vector3 cog, const MyVec<GEO_Primitive*> &prims, int num_tri, int* tris, btScalar* vers, btVector3 &aabbMin, btVector3 &aabbMax);

	MyVec<btVector3>& getRelShape();
	bool computeAproxConvexRelVec(MyVec<btVector3> &rel_vec, btConvexHullShape* newShape);
	bool computeAproxDeformRelVec(MyVec<btVector3> &rel_vec, btBvhTriangleMeshShape* newShape);
	bool aproxConvexShape(int num_substeps);
	bool aproxDeformShape(const int num_substeps);

private:
	///return transformation betwee this body and child
	btTransform getChildTransform(const BRigidBody* child) const;	
};




