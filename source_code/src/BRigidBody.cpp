/**
BulletSOP v2.0
Copyright (c)2010-2013 Suk Milan. All Rights Reserved.
This software is provided 'as-is', without any express or implied warranty.
In no event will the authors be held liable for any damages arising from the use of this software.
This file may not be redistributed in whole or significant part.
This notice may not be removed or altered from any source distribution.
*/
#include "stdafx.h"
#include "BRigidBody.h"

#include "BWorld.h"
#include "BulletCollision/Gimpact/btGImpactShape.h"

#include "SOP_Build.h"
#include "SObject.h"



BRigidBody::BRigidBody(int type, const btRigidBodyConstructionInfo& constructionInfo, int hId, BRigidBody* compound) : btRigidBody(constructionInfo)
{
	m_hId = hId;
	m_compound = compound;
	m_type = type;

	m_update = false;
}


BRigidBody::~BRigidBody()
{
	btCollisionShape* shape = getCollisionShape();

	if(hasCompound())
		removeChildShape();

	//for deform body, It needs to remove shape geometry
	if(m_type==SOP_Build::TYPE_DEFORM)
	{
		btBvhTriangleMeshShape* bvhShape = static_cast<btBvhTriangleMeshShape*>(shape);
		btStridingMeshInterface* mesh = bvhShape->getMeshInterface();

		btScalar *vers = 0; int numverts; PHY_ScalarType type; int stride;int *tris = 0;int indexstride;int numfaces;PHY_ScalarType indicestype;
		mesh->getLockedVertexIndexBase(reinterpret_cast<unsigned char**>(&vers), numverts, type, stride, reinterpret_cast<unsigned char**>(&tris), indexstride, numfaces, indicestype);

		delete[] vers;
		delete[] tris;

		delete mesh;
	}

//	if(shape)
//		delete shape;

	if(getMotionState())
		delete getMotionState();
}


float BRigidBody::getMass() const
{
	float mass = getInvMass();
	if(mass)
		mass = 1.0f / mass;
	return mass;
}

void BRigidBody::addChildShape(BRigidBody* compound)
{
	btTransform tInit;tInit.setIdentity();
	btTransform trChild;trChild.setIdentity();
	btTransform trFinal;trFinal.setIdentity();

	compound->getMotionState()->getWorldTransform(tInit);
	getMotionState()->getWorldTransform(trFinal);

	trChild = tInit.inverse() * trFinal;	//reletive transform from init to final

	btCompoundShape* compShape = static_cast<btCompoundShape*>( compound->getCollisionShape() );
	compShape->addChildShape(trChild, getCollisionShape());
	compound->setMass( compound->getMass() );	//right rotation !!!
	
	m_compound = compound;
}


void BRigidBody::removeChildShape()
{
	if(!m_compound)
		return;

	//recompute world position
	btTransform tInit;		m_compound->getMotionState()->getWorldTransform(tInit);
	btTransform trChild;	trChild = m_compound->getChildTransform(this);
	btTransform tFinal = tInit*trChild;
	getMotionState()->setWorldTransform(tFinal);	//for SOP_Solver output( getTranslateFull(), ...)
	setWorldTransform(tFinal);						//for bullet

	//remove
	btCompoundShape* compShape = static_cast<btCompoundShape*>( m_compound->getCollisionShape() );
	compShape->removeChildShape( getCollisionShape() );
	m_compound = 0;


}


BRigidBody* BRigidBody::getCompound() const
{
	return m_compound;
}

bool BRigidBody::hasCompound() const
{
	return m_compound != 0;
}

bool BRigidBody::isCompound() const
{
	return m_type==SOP_Build::TYPE_COMPOUND;
}


UT_Vector3 BRigidBody::getPointVelocity(UT_Vector3 world_t) const
{
	UT_Vector3 v = getVelocity();
	UT_Vector3 av = getAngVelocity();

	btVector3 rel = getWorldTransform().inverse() * get_bullet_V3(world_t);	//convert world to relative
	btVector3 relAV = get_bullet_V3(av).cross(rel);							//compute vel
	btVector3 absAV = getWorldTransform().getBasis() * relAV;				//convert vel to world

	return v + get_hdk_V3(absAV);
}


void BRigidBody::setStaticBodyTransform(UT_Vector3 t, UT_Vector3 r)
{
	if(!isStaticOrKinematicObject())
		return;

	btVector3 bt = get_bullet_V3(t);

	btQuaternion br;
	br.setEulerZYX(r[2], r[1], r[0]);

	//switch current to old
	setPosOld(m_t_current);
	setRotOld(m_r_current);

	//save new current
	setPosCurrent(bt);
	setRotCurrent(br);
}


void BRigidBody::setDynamicBodyTransform(UT_Vector3 t, UT_Vector3 r)
{
	if(isStaticOrKinematicObject())
		return;

	btTransform transform;
	transform.setIdentity();
	transform.setOrigin( get_bullet_V3(t) );

	btQuaternion br;
	br.setEulerZYX(r[2], r[1], r[0]);
	transform.setRotation(br);

	setWorldTransform(transform);
}



UT_Vector3 BRigidBody::getTranslateFull() const
{
	UT_Vector3 translate(0,0,0);
	if(isStaticOrKinematicObject())
	{
		translate = get_hdk_V3(m_t_current);
	}
	else
	{
		btTransform t;
		t.setIdentity();
		getMotionState()->getWorldTransform(t);

		if(m_compound)
			t *= m_compound->getChildTransform(this);

		translate = get_hdk_V3(t.getOrigin());
	}

	return translate;
}


btTransform BRigidBody::getChildTransform(const BRigidBody* child) const
{
	btTransform tChild;
	tChild.setIdentity();

	const btCollisionShape* find_child = child->getCollisionShape();

	//try find our "find_child" in compound and get transformation
	const btCompoundShape* compShape = static_cast<const btCompoundShape*>( getCollisionShape() );
	for(int i=0; i < compShape->getNumChildShapes(); i++)
	{
		if(find_child==compShape->getChildShape(i))
		{
			tChild = compShape->getChildTransform(i);
			break;
		}
	}

	return tChild;
}


UT_Vector3 BRigidBody::getRotateFull() const
{
	UT_Vector3 rotate(0,0,0);

	if(isStaticOrKinematicObject())
	{
		btMatrix3x3 m;
		m.setRotation(m_r_current);
		m.getEulerZYX(rotate[2], rotate[1], rotate[0]);
	}
	else
	{
		btTransform t;
		t.setIdentity();
		getMotionState()->getWorldTransform(t);

		t.getBasis().getEulerZYX(rotate[2], rotate[1], rotate[0]);
	}

	return (UT_Vector3)rotate;
}



UT_Vector3 BRigidBody::getTranslateWithoutInitRot() const
{
	UT_Vector3 translate(0,0,0);
	if(isStaticOrKinematicObject())
	{
		translate = get_hdk_V3(m_t_current);
	}
	else
	{
		btTransform tFinal, tInit;
		tFinal.setIdentity();
		tInit.setIdentity();

		if(m_compound)
		{
			m_compound->getMotionState()->getWorldTransform(tFinal);
			tFinal *= m_compound->getChildTransform(this);
			//tInit.setBasis( m_compound->m_initRot.inverse() );
		}
		else
		{
			getMotionState()->getWorldTransform(tFinal);
			//tInit.setBasis( m_initRot.inverse() );
		}

		btTransform tTrans = tFinal;
		tTrans *= tInit;

		translate = get_hdk_V3( tTrans.getOrigin() );
	}

	return translate;
}

UT_Vector3 BRigidBody::getRotateWithoutInitRot() const
{
	UT_Vector3 rotate(0,0,0);

	if(isStaticOrKinematicObject())
	{
		btMatrix3x3 m;
		m.setRotation(m_r_current);
		m.getEulerZYX(rotate[2], rotate[1], rotate[0]);
	}
	else
	{
		btTransform tFinal, tInit;
		tFinal.setIdentity();
		tInit.setIdentity();

		if(m_compound)
		{
			m_compound->getMotionState()->getWorldTransform(tFinal);
			//tInit.setBasis( m_compound->m_initRot.inverse() );
		}
		else
		{
			getMotionState()->getWorldTransform(tFinal);
			//tInit.setBasis( m_initRot.inverse() );
		}

		(tFinal.getBasis() * tInit.getBasis()).getEulerZYX(rotate[2], rotate[1], rotate[0]);
	}

	return (UT_Vector3)rotate;
}




UT_Vector3 BRigidBody::getVelocity() const
{
	if(m_compound)
		return get_hdk_V3( m_compound->getVelocityInLocalPoint( m_compound->getChildTransform(this).getOrigin() ) );

	return get_hdk_V3(getLinearVelocity());
}

UT_Vector3 BRigidBody::getAngVelocity() const
{
	if(m_compound)
		return UT_Vector3(0,0,0);
	return get_hdk_V3(getAngularVelocity());
}

UT_Vector3 BRigidBody::getOldVelocity() const
{
	return old_v;
}

UT_Vector3 BRigidBody::getOldAngVelocity() const
{
	return old_av;
}



void BRigidBody::setMass(float mass)
{
	bool disable_collisions = false;
	if(mass < 0)
	{
		disable_collisions = true;
		mass *= -1;	//abs() it, because object needs to have mass => original negativ value say only that object is without collisions with others objects
	}

	btVector3 inertia;
	getCollisionShape()->calculateLocalInertia(mass, inertia);
	setMassProps(mass, inertia);
	updateInertiaTensor();

	//set flags
	if(mass==0)
	{
		setCollisionFlags(getCollisionFlags() | btCollisionObject::CF_KINEMATIC_OBJECT);	//static
		setActivationState(DISABLE_DEACTIVATION);
	}
	else
		setCollisionFlags(0);	//default dynamic

	if(disable_collisions)
		setCollisionFlags(btCollisionObject::CF_NO_CONTACT_RESPONSE);	//no collision with other bodies
}


void BRigidBody::setVelocity(UT_Vector3 v)
{
	setLinearVelocity( get_bullet_V3(v) );
}

void BRigidBody::setAngVelocity(UT_Vector3 av)
{
	setAngularVelocity(  get_bullet_V3(av) );
}

void BRigidBody::setOldVelocity(UT_Vector3 v)
{
	old_v = v;
}

void BRigidBody::setOldAngVelocity(UT_Vector3 av)
{
	old_av = av;
}




void BRigidBody::setCollisionMargin(float colmargin)
{
	btCollisionShape* shape = getCollisionShape();

	shape->setMargin(colmargin);

	//set margin for all childs
	if(m_type==SOP_Build::TYPE_COMPOUND)
	{
		btCompoundShape* compShape = static_cast<btCompoundShape*>(shape);
		for(int i=0; i < compShape->getNumChildShapes(); i++)
			compShape->getChildShape(i)->setMargin(colmargin);
	}
}

void BRigidBody::setSleeping(bool sleep)
{
	if(!isStaticOrKinematicObject())	//only for dynamic
		setActivationState( sleep ? ACTIVE_TAG : DISABLE_DEACTIVATION );
}


void BRigidBody::setForce(UT_Vector3 world_pos, UT_Vector3 force, int type)
{
	btVector3 rel_pos;
	if(type==SOP_Build::FULL || type==SOP_Build::TORQUE)
	{
		btTransform world;
		world.setIdentity();
		world.setOrigin( get_bullet_V3(world_pos) );
		rel_pos = getWorldTransform().inverseTimes(world).getOrigin();	//get relative transformation between object_COG and force_position
	}

	btVector3 f = get_bullet_V3(force);

	switch(type)
	{
		case SOP_Build::FULL:
			m_my_totalForce += f;
			m_my_totalTorque += rel_pos.cross(f*getLinearFactor());
			break;
		case SOP_Build::CENTRAL:
			m_my_totalForce += f;
			break;
		case SOP_Build::TORQUE:
			m_my_totalTorque += rel_pos.cross(f*getLinearFactor());
			break;
		case SOP_Build::MAGNET:
			m_my_totalForce += f;
			break;
	}
	/*
	switch(type)
	{
		case SOP_Build::FULL:		applyForce(f, rel_pos);								break;
		case SOP_Build::CENTRAL:	applyCentralForce(f);								break;
		case SOP_Build::TORQUE:		applyTorque(rel_pos.cross(f*getLinearFactor()));	break;
		case SOP_Build::MAGNET:		applyCentralForce(f);								break;
	}
	*/
}

void BRigidBody::clearMyForces()
{
	m_my_totalForce = btVector3(0, 0, 0);
	m_my_totalTorque = btVector3(0, 0, 0);

}
void BRigidBody::uploadMyForces()
{
	applyCentralForce(m_my_totalForce);
	applyTorque(m_my_totalTorque);
}



void BRigidBody::setUpdate(bool update)
{
	m_update = update;
}

bool BRigidBody::isUpdated()
{
	return m_update;
}


void BRigidBody::setLine(int v)
{
	m_hLine = v;
}

int BRigidBody::getLine() const
{
	return m_hLine;
}
int BRigidBody::getIndex() const
{
	return m_hId;
}

int BRigidBody::getOldCIndex() const
{
	if(!m_compound)
		return -1;

	return m_compound->getIndex();
}


void BRigidBody::setPosOld(btVector3 v)
{
	m_t_old = v;
}
void BRigidBody::setPosCurrent(btVector3 v)
{
	m_t_current = v;
}
void BRigidBody::setRotOld(btQuaternion &q)
{
	m_r_old = q;
}
void BRigidBody::setRotCurrent(btQuaternion &q)
{
	m_r_current = q;
}


btVector3 BRigidBody::getAproxPos(float rel) const
{
	return m_t_old + (m_t_current - m_t_old)*rel;
}
btQuaternion BRigidBody::getAproxRot(float rel) const
{
	return m_r_old.slerp(m_r_current, rel);
}

bool BRigidBody::isAproxChanged() const
{
	return m_t_old!=m_t_current || m_r_old!=m_r_current;
}




btVector3 vectorMax(btVector3 v, btVector3 aabbMax)
{
	btVector3 ret = aabbMax;
	if(v[0] > aabbMax[0])	ret[0] = v[0];
	if(v[1] > aabbMax[1])	ret[1] = v[1];
	if(v[2] > aabbMax[2])	ret[2] = v[2];
	return ret;
}
btVector3 vectorMin(btVector3 v, btVector3 aabbMin)
{
	btVector3 ret = aabbMin;
	if(v[0] < aabbMin[0])	ret[0] = v[0];
	if(v[1] < aabbMin[1])	ret[1] = v[1];
	if(v[2] < aabbMin[2])	ret[2] = v[2];
	return ret;
}

void BRigidBody::updateTriangles(UT_Vector3 cog, const MyVec<GEO_Primitive*> &prims, int num_tri, int* tris, btScalar* vers, btVector3 &aabbMin, btVector3 &aabbMax)
{
	aabbMin = btVector3(BT_LARGE_FLOAT, BT_LARGE_FLOAT, BT_LARGE_FLOAT);
	aabbMax = btVector3(-BT_LARGE_FLOAT, -BT_LARGE_FLOAT, -BT_LARGE_FLOAT);

	for(int i=0; i < num_tri; i++)
	{
		GEO_Primitive* prim = prims[i];
		if(prim->getVertexCount() < 3)
		{
			printf("some primitive has less than 3 verticies\n");
			continue;
		}

		UT_Vector3 v[3];
		for(int ii=0; ii < 3; ii++)
		{
			//get & set vertexes
			v[ii] = getPrimPos3(prim, ii); - cog;
			vers[i*9+ii*3+0] = v[ii][0];
			vers[i*9+ii*3+1] = v[ii][1];
			vers[i*9+ii*3+2] = v[ii][2];

			//compute bbox
			aabbMax = vectorMax(get_bullet_V3(v[ii]), aabbMax);
			aabbMin = vectorMin(get_bullet_V3(v[ii]), aabbMin);
		}

		//set triangle ID(option)
		tris[i*3+0] = i*3+2;
		tris[i*3+1] = i*3+0;
		tris[i*3+2] = i*3+1;
	}
}



bool BRigidBody::computeAproxConvexRelVec(MyVec<btVector3> &rel_vec, btConvexHullShape* newShape)
{
	const size_t N = newShape->getNumPoints();
	btConvexHullShape* origShape = (btConvexHullShape*)getCollisionShape();

	if(origShape->getNumPoints()==N)
	{
		rel_vec.resize(N);
		for(int i=0; i < N; i++)
		{
			rel_vec[i] = newShape->getUnscaledPoints()[i] - origShape->getUnscaledPoints()[i];	//computes rel
			newShape->getUnscaledPoints()[i] = origShape->getUnscaledPoints()[i];				//reset actual
		}
	}
	else
		return false;

	return true;
}


bool BRigidBody::computeAproxDeformRelVec(MyVec<btVector3> &rel_vec, btBvhTriangleMeshShape* newShape)
{
	btStridingMeshInterface* newMesh = newShape->getMeshInterface();
	btScalar *new_vers = 0; int new_numverts; PHY_ScalarType new_type; int new_stride;int *new_tris = 0;int new_indexstride;int new_numfaces;PHY_ScalarType new_indicestype;
	newMesh->getLockedReadOnlyVertexIndexBase((const unsigned char**)&new_vers, new_numverts, new_type, new_stride, (const unsigned char**)&new_tris, new_indexstride, new_numfaces, new_indicestype);


	btBvhTriangleMeshShape* origShape = (btBvhTriangleMeshShape*)getCollisionShape();
	btStridingMeshInterface* origMesh = origShape->getMeshInterface();
	btScalar *orig_vers = 0; int orig_numverts; PHY_ScalarType orig_type; int orig_stride;int *orig_tris = 0;int orig_indexstride;int orig_numfaces;PHY_ScalarType orig_indicestype;
	origMesh->getLockedReadOnlyVertexIndexBase((const unsigned char**)&orig_vers, orig_numverts, orig_type, orig_stride, (const unsigned char**)&orig_tris, orig_indexstride, orig_numfaces, orig_indicestype);

	if(orig_numverts==new_numverts)
	{
		rel_vec.resize(new_numverts);
		for(int i=0; i < new_numverts; i++)
		{
			for(int ii=0; ii < 3; ii++)
			{
				rel_vec[i][ii] = new_vers[i*3+ii] - orig_vers[i*3+ii];	//computes rel
				new_vers[i*3+ii] = orig_vers[i*3+ii];					//reset actual ...
			}
			rel_vec[i][3] = 1.0f;	//just for sure
		}
	}
	else
		return false;

	return true;
}


MyVec<btVector3>& BRigidBody::getRelShape()
{
	return m_rel_shape;
}

bool BRigidBody::aproxConvexShape(const int num_substeps)
{
	if(m_type!=SOP_Build::TYPE_CONVEX)
		return false;

	btConvexHullShape* shape = (btConvexHullShape*)getCollisionShape();
	
	if(m_rel_shape.size()!=shape->getNumPoints())
		return false;
	
	float t = 1.0f / num_substeps;
	for(int i=0; i < m_rel_shape.size(); i++)
		shape->getUnscaledPoints()[i] += m_rel_shape[i] * t;

	return true;
}


bool BRigidBody::aproxDeformShape(const int num_substeps)
{
	if(m_type!=SOP_Build::TYPE_DEFORM)
		return false;

	btBvhTriangleMeshShape* shape = (btBvhTriangleMeshShape*)getCollisionShape();
	btStridingMeshInterface* mesh = shape->getMeshInterface();
	btScalar *vers = 0; int numverts; PHY_ScalarType type; int stride;int *tris = 0;int indexstride;int numfaces;PHY_ScalarType indicestype;
	mesh->getLockedVertexIndexBase(reinterpret_cast<unsigned char**>(&vers), numverts, type, stride, reinterpret_cast<unsigned char**>(&tris), indexstride, numfaces, indicestype);


	if(m_rel_shape.size()!=numverts)
		return false;
	
	btVector3 aabbMin = btVector3(BT_LARGE_FLOAT, BT_LARGE_FLOAT, BT_LARGE_FLOAT);
	btVector3 aabbMax = btVector3(-BT_LARGE_FLOAT, -BT_LARGE_FLOAT, -BT_LARGE_FLOAT);

	float t = 1.0f / num_substeps;
	for(int i=0; i < m_rel_shape.size(); i++)
	{
		btVector3 v(vers[i*3+0] + m_rel_shape[i][0] * t, 
					vers[i*3+1] + m_rel_shape[i][1] * t, 
					vers[i*3+2] + m_rel_shape[i][2] * t);

		for(int ii=0; ii < 3; ii++)
			vers[i*3+ii] = v[ii];

		aabbMax = vectorMax(v, aabbMax);
		aabbMin = vectorMin(v, aabbMin);
	}

	//update inside Bullet Library
	mesh->unLockVertexBase(0);
	shape->refitTree(aabbMin, aabbMax);	//sha->partialRefitTree(aabbMin,aabbMax);
	return true;
}
