/**
BulletSOP v2.0
Copyright (c)2010-2013 Suk Milan. All Rights Reserved.
This software is provided 'as-is', without any express or implied warranty.
In no event will the authors be held liable for any damages arising from the use of this software.
This file may not be redistributed in whole or significant part.
This notice may not be removed or altered from any source distribution.
*/
#include "stdafx.h"
#include "BWorld.h"


#ifdef _WIN32
#include "BulletMultiThreaded/Win32ThreadSupport.h"
#else
#include "BulletMultiThreaded/PosixThreadSupport.h"
#endif

#include "BulletMultiThreaded/SpuGatheringCollisionDispatcher.h"
#include "BulletMultiThreaded/SpuNarrowPhaseCollisionTask/SpuGatheringCollisionTask.h"
#include "BulletMultiThreaded/btParallelConstraintSolver.h"
#include "BulletMultiThreaded/btGpu3DGridBroadphase.h"
#include "BulletCollision/CollisionDispatch/btSimulationIslandManager.h"

//new constraint solver in Bullet v2.82
#include "BulletDynamics/MLCPSolvers/btDantzigSolver.h"
#include "BulletDynamics/MLCPSolvers/btSolveProjectedGaussSeidel.h"
#include "BulletDynamics/MLCPSolvers/btMLCPSolver.h"

//soft bodies
//#include "BulletSoftBody/btSoftRigidDynamicsWorld.h"
//#include "BulletSoftBody/btSoftBodyRigidBodyCollisionConfiguration.h"
//#include "BulletSoftBody/btSoftBodyHelpers.h"


#include "SOP_Build.h"
#include "SOP_Solver.h"
#include "SObject.h"
#include "SShape.h"

#include "BCleanBullet.h"
ECleanBullets g_cleanbullets;	//holds all worlds which are deleted in separ threads


BWorld::BWorld(
	bool deleteThread,
	bool share_shapes,
	int collisionConfiguration_defaultMaxPersistentManifoldPoolSize,
	int collisionConfiguration_defaultMaxCollisionAlgorithmPoolSize,
	int use_broadphase, 
	UT_Vector3 broadphase_WorldCenter,
	UT_Vector3 broadphase_WorldSize,
	int broadphase_maxSmallProxies,
	int broadphase_maxLargeProxies,
	int broadphase_maxPairsPerBody,
	int broadphase_maxBodiesPerCell,
	float broadphase_cellFactorAABB,
	int use_solver,
	int m_solver_numThreads)
{
	m_deleteThread = deleteThread;
	m_share_shapes = share_shapes;

	m_dynamicsWorld = 0;
	m_collisionConfiguration = 0;
	m_dispatcher = 0;
	m_broadphase = 0;
	m_solver = 0;
	m_threadSupportSolver = 0;
	m_impact = 0;


	btVector3 broadphase_WorldMin = get_bullet_V3(broadphase_WorldCenter) - get_bullet_V3(broadphase_WorldSize)*0.5f;
	btVector3 broadphase_WorldMax = get_bullet_V3(broadphase_WorldCenter) + get_bullet_V3(broadphase_WorldSize)*0.5f;


/*Collision Configuration*/
	btDefaultCollisionConstructionInfo dci;
	dci.m_defaultMaxPersistentManifoldPoolSize=collisionConfiguration_defaultMaxPersistentManifoldPoolSize;
	dci.m_defaultMaxCollisionAlgorithmPoolSize=collisionConfiguration_defaultMaxCollisionAlgorithmPoolSize;
	//dci.m_customCollisionAlgorithmMaxElementSize = sizeof(SpuContactManifoldCollisionAlgorithm);	

	m_collisionConfiguration = new btDefaultCollisionConfiguration(dci);
//	m_collisionConfiguration = new btSoftBodyRigidBodyCollisionConfiguration(dci);


/*Dispatcher*/
	if(use_solver==SOP_Solver::SOLVER_PARALLEL)
	{
	#ifdef _WIN32
		Win32ThreadSupport::Win32ThreadConstructionInfo constructionInfo("collision", processCollisionTask, createCollisionLocalStoreMemory, m_solver_numThreads);
		Win32ThreadSupport* threadSupport = new Win32ThreadSupport(constructionInfo);
	#else
	    PosixThreadSupport::ThreadConstructionInfo constructionInfo("collision", processCollisionTask, createCollisionLocalStoreMemory,	m_solver_numThreads);
		PosixThreadSupport* threadSupport = new PosixThreadSupport(constructionInfo);
	#endif

		m_dispatcher = new	SpuGatheringCollisionDispatcher(threadSupport, m_solver_numThreads, m_collisionConfiguration);
	}
	else
	{
		m_dispatcher = new btCollisionDispatcher(m_collisionConfiguration);
	}

	
/*Broadphase*/
	btHashedOverlappingPairCache* gPairCache = new( btAlignedAlloc(sizeof(btHashedOverlappingPairCache), 16) ) btHashedOverlappingPairCache(); 
	if(use_broadphase==SOP_Solver::BROAD_DBVT)
	{
		btDbvtBroadphase* dbvt = new btDbvtBroadphase(gPairCache);
		//dbvt->m_deferedcollide=false;
		//dbvt->m_prediction = 0.f;
		m_broadphase = dbvt;
	}
	else
	if(use_broadphase==SOP_Solver::BROAD_GPU3D)
	{
		btVector3 numOfCells = (broadphase_WorldMax - broadphase_WorldMin) / 2;
		int numOfCellsX = (int)numOfCells[0];
		int numOfCellsY = (int)numOfCells[1];
		int numOfCellsZ = (int)numOfCells[2];

		//broadphase_maxSmallProxies += 1024;
		m_broadphase = new btGpu3DGridBroadphase(gPairCache, broadphase_WorldMin, broadphase_WorldMax,numOfCellsX, numOfCellsY, numOfCellsZ, broadphase_maxSmallProxies,  broadphase_maxLargeProxies, broadphase_maxPairsPerBody, broadphase_maxBodiesPerCell, broadphase_cellFactorAABB);
	}
	else
	if(use_broadphase==SOP_Solver::BROAD_AXIS3_32BIT)
	{
		m_broadphase = new bt32BitAxisSweep3(broadphase_WorldMin, broadphase_WorldMax, broadphase_maxSmallProxies, gPairCache, true);
	}
		
	
/*Solver*/
	if(use_solver==SOP_Solver::SOLVER_SEQ)
	{
		m_solver = new btSequentialImpulseConstraintSolver();
	}
	else
	if(use_solver==SOP_Solver::SOLVER_PARALLEL)
	{
 
	#ifdef _WIN32
		Win32ThreadSupport::Win32ThreadConstructionInfo threadConstructionInfo("solverThreads", SolverThreadFunc, SolverlsMemoryFunc, m_solver_numThreads);
		Win32ThreadSupport* threadSupport = new Win32ThreadSupport(threadConstructionInfo);
		threadSupport->startSPU();
	#else
		PosixThreadSupport::ThreadConstructionInfo solverConstructionInfo("solverThreads", SolverThreadFunc, SolverlsMemoryFunc, m_solver_numThreads);
		PosixThreadSupport* threadSupport = new PosixThreadSupport(solverConstructionInfo);
	#endif
		m_threadSupportSolver = threadSupport;

		m_solver = new btParallelConstraintSolver(m_threadSupportSolver);
		m_dispatcher->setDispatcherFlags(btCollisionDispatcher::CD_DISABLE_CONTACTPOOL_DYNAMIC_ALLOCATION);	//this solver requires the contacts to be in a contiguous pool, so avoid dynamic allocation
	}
	else
	if(use_solver==SOP_Solver::SOLVER_MLCP)
	{
		btDantzigSolver* mlcp = new btDantzigSolver();
		//btSolveProjectedGaussSeidel* mlcp = new btSolveProjectedGaussSeidel;
		m_solver = new btMLCPSolver(mlcp);
	}




/*World*/
	m_dynamicsWorld = new btDiscreteDynamicsWorld(m_dispatcher, m_broadphase, m_solver, m_collisionConfiguration);
//	m_dynamicsWorld = new btSoftRigidDynamicsWorld(m_dispatcher, m_broadphase, m_solver, m_collisionConfiguration, 0);
	

	if(use_solver==SOP_Solver::SOLVER_PARALLEL)
	{
		m_dynamicsWorld->getSimulationIslandManager()->setSplitIslands(false);
		m_dynamicsWorld->getSolverInfo().m_numIterations = m_solver_numThreads;
		m_dynamicsWorld->getSolverInfo().m_solverMode = SOLVER_SIMD+SOLVER_USE_WARMSTARTING;//+SOLVER_RANDMIZE_ORDER;
		m_dynamicsWorld->getDispatchInfo().m_enableSPU = true;
	}
	else
	if(use_solver==SOP_Solver::SOLVER_MLCP)
	{
		m_dynamicsWorld ->getSolverInfo().m_minimumSolverBatchSize = 1;
	}


	m_impact = new BImpact(m_dynamicsWorld);
	m_collisionFilter = new BCollisionFilter(m_dynamicsWorld);

	m_dynamicsWorld->clearForces();

	//floor
	createFloor();
}



BWorld::~BWorld(void)
{
	deleteFloor();

	if(m_impact)
		delete m_impact;

	if(m_collisionFilter)
		delete m_collisionFilter;

	ECleanBullet* c = new ECleanBullet;
	c->m_dynamicsWorld = m_dynamicsWorld;
	c->m_collisionConfiguration = m_collisionConfiguration;
	c->m_dispatcher = m_dispatcher;
	c->m_broadphase = m_broadphase;
	c->m_solver = m_solver;
	c->m_threadSupportSolver = m_threadSupportSolver;
	c->m_shapes = m_shapes;

	//delete data in separ thread Or run it in current thread
	if(m_deleteThread)
		g_cleanbullets.addAndRun(c);
	else
	{
		c->run();
		delete c;
	}
}


void BWorld::createFloor()
{
	btCollisionShape* groundShape = new btStaticPlaneShape(btVector3(0,1,0), 0);

	btDefaultMotionState* groundMotionState = new btDefaultMotionState();
	btRigidBody::btRigidBodyConstructionInfo rbInfo(0, groundMotionState, groundShape, btVector3(0,0,0));

	m_floorBody = new BRigidBody(0, rbInfo, 0, 0);
	m_floorBodyInWorld = false;
}


void BWorld::deleteFloor()
{
	removeFloor();
	delete m_floorBody;
}


BImpact* BWorld::getImpact()
{
	return m_impact;
}

BCollisionFilter* BWorld::getCollisionFilter()
{
	return m_collisionFilter;
}




void BWorld::addCollisionShape(btCollisionShape* sh)
{
	if(m_shapes.size()>=m_shapes.capacity()-1)
		m_shapes.reserve(m_shapes.size()+EXTRA_ALLOC);

	m_shapes.push_back(sh);	
}

btCollisionShape* BWorld::findBoxShape(btVector3 s)
{
	if(!m_share_shapes)
		return 0;

	for(int i=0; i < m_shapes.size(); i++)
	{
		if(m_shapes[i]->getShapeType()==BOX_SHAPE_PROXYTYPE)
		{
			btBoxShape* sh = static_cast<btBoxShape*>(m_shapes[i]);

			if(sh->getHalfExtentsWithMargin()==s)
				return sh;
		}
	}
	return 0;
}

btCollisionShape* BWorld::findSphereShape(float rad)
{
	if(!m_share_shapes)
		return 0;

	for(int i=0; i < m_shapes.size(); i++)
	{
		if(m_shapes[i]->getShapeType()==SPHERE_SHAPE_PROXYTYPE)
		{
			btSphereShape* sh = static_cast<btSphereShape*>(m_shapes[i]);

			if(sh->getRadius()==rad)
				return sh;
		}
	}
	return 0;
}





BRigidBody* BWorld::addBox(int index, UT_Vector3 t, UT_Vector3 r, UT_Vector3 cog_r, UT_Vector3 half_box_size, BRigidBody* compound)
{
	btCollisionShape* shape = findBoxShape( get_bullet_V3(half_box_size) );
	if(!shape)
	{
		shape = new btBoxShape( get_bullet_V3(half_box_size) );
		addCollisionShape(shape);
	}
	BRigidBody* body = addRigidObject(SOP_Build::TYPE_BOX, shape, index, t, r, cog_r, compound);
	return body;
}

BRigidBody* BWorld::addSphere(int index, UT_Vector3 t, UT_Vector3 r, UT_Vector3 cog_r, float rad, BRigidBody* compound)
{
	btCollisionShape* shape = findSphereShape(rad);
	if(!shape)
	{
		shape = new btSphereShape(rad);
		addCollisionShape(shape);
	}
	BRigidBody* body = addRigidObject(SOP_Build::TYPE_SPHERE, shape, index, t, r, cog_r, compound);
	return body;
}

BRigidBody* BWorld::addCapsule(int index, UT_Vector3 t, UT_Vector3 r, UT_Vector3 cog_r, float rad, float h, BRigidBody* compound)
{
	btCollisionShape* shape = new btCapsuleShape(rad, h);
	addCollisionShape(shape);
	BRigidBody* body = addRigidObject(SOP_Build::TYPE_CAPSULE, shape, index, t, r, cog_r, compound);
	return body;
}

BRigidBody* BWorld::addCone(int index, UT_Vector3 t, UT_Vector3 r, UT_Vector3 cog_r, float rad, float h, BRigidBody* compound)
{
	btCollisionShape* shape = new btConeShape(rad, h);
	addCollisionShape(shape);
	BRigidBody* body = addRigidObject(SOP_Build::TYPE_CONE, shape, index, t, r, cog_r, compound);
	return body;
}

BRigidBody* BWorld::addCylinder(int index, UT_Vector3 t, UT_Vector3 r, UT_Vector3 cog_r, float rad, float h, BRigidBody* compound)
{
	btCollisionShape* shape = new btCylinderShape( btVector3(rad, h*0.5f, rad));
	addCollisionShape(shape);
	BRigidBody* body = addRigidObject(SOP_Build::TYPE_CYLINDER, shape, index, t, r, cog_r, compound);
	return body;
}


BRigidBody* BWorld::addConvex(int index, UT_Vector3 t, UT_Vector3 r, UT_Vector3 cog_r, UT_Vector3 cog, const SObject &object, BRigidBody* compound, bool save_vertexes)
{
	cog_r = 0;
	btConvexHullShape* shape = new btConvexHullShape();
	addCollisionShape(shape);

	if(!object.isIndexExist(index))
		return 0;

	//copy points
	GU_Detail* gdp = object.getShape()->getGeo();

	const MyVec<GA_Offset>& points = object.getGEO_Offsets(index);
	for(int i=0; i < points.size(); i++)
		shape->addPoint( get_bullet_V3(gdp->getPos3(points[i]) - cog) );


	//recreate objects between frames - computes relative vec between points for blending during sim. substeps
	MyVec<btVector3> rel_vec;
	BRigidBody* origBody = getBody(index);
	if(origBody)
		if(!origBody->computeAproxConvexRelVec(rel_vec, shape))
			printf("Warning: Blend points for object[%d] don't updated!\n", index);

	BRigidBody* body = addRigidObject(SOP_Build::TYPE_CONVEX, shape, index, t, r, cog_r, compound);

	body->getRelShape() = rel_vec;

	return body;
}



BRigidBody* BWorld::addDeform(int index, UT_Vector3 t, UT_Vector3 r, UT_Vector3 cog_r, UT_Vector3 cog, const SObject &object, BRigidBody* compound, bool save_vertexes)
{
	if(!object.isIndexExist(index))
		return 0;

	const MyVec<GEO_Primitive*> &prims = object.getGEO_Primitives(index);

	const int num_tri = (int)prims.size();
	const int num_ver = num_tri*3;

	//alloc
	int* tris = new int[num_tri*3];
	btScalar* vers = new btScalar[num_ver*3];
	btTriangleIndexVertexArray* m_indexVertexArrays = new btTriangleIndexVertexArray(	num_tri, tris, sizeof(int)*3,
																						num_ver, vers, sizeof(btScalar)*3);

	//copy indicates and vertexes
	btVector3 aabbMin, aabbMax;
	BRigidBody::updateTriangles(cog, prims, num_tri, tris, vers, aabbMin, aabbMax);

	btBvhTriangleMeshShape* shape = new btBvhTriangleMeshShape(m_indexVertexArrays, true);
	addCollisionShape(shape);


	//recreate objects between frames - computes relative vec between points for blending during sim. substeps
	MyVec<btVector3> rel_vec;
	BRigidBody* origBody = getBody(index);
	if(origBody)
		if(!origBody->computeAproxDeformRelVec(rel_vec, shape))
			printf("Warning: Blend points for object[%d] don't updated!\n", index);

	BRigidBody* body = addRigidObject(SOP_Build::TYPE_DEFORM, shape, index, t, r, cog_r, compound);

	body->getRelShape() = rel_vec;

	return body;
}

BRigidBody* BWorld::addCompound(int index, UT_Vector3 t, UT_Vector3 r, UT_Vector3 cog_r, BRigidBody* compound)
{
	btCompoundShape* shape = new btCompoundShape();
	addCollisionShape(shape);
	BRigidBody* body = addRigidObject(SOP_Build::TYPE_COMPOUND, shape, index, t, r, cog_r, compound);
	return body;
}




/*
btSoftBody* BWorld::addSoft(int index, UT_Vector3 t, UT_Vector3 r, UT_Vector3 cog_r, UT_Vector3 cog, const SObject &object)
{
	if(!object.isIndexExist(index))
		return 0;

	MyVec<GEO_Primitive*> prims = object.getGEO_Primitives(index);

	const int num_tri = (int)prims.size();
	const int num_ver = num_tri*3;

	//alloc
	int* tris = new int[num_tri*3];
	btScalar* vers = new btScalar[num_ver*3];

	btSoftBodyWorldInfo	m_softBodyWorldInfo;
	btSoftBody* body = btSoftBodyHelpers::CreateFromTriMesh(m_softBodyWorldInfo, static_cast<btScalar*>(&vers[0]), static_cast<int*>(&tris[0]), num_tri);

	body->m_cfg.kDF				=	0.5f;
	body->m_cfg.kMT				=	0.05f;
	body->m_cfg.piterations		=	5;
	body->randomizeConstraints();
	body->scale(btVector3(6,6,6));
	body->setTotalMass(100,true);
	body->setPose(false,true);

	m_dynamicsWorld->addSoftBody(body);
	return body;
}
*/






BRigidBody* BWorld::addRigidObject(int type, btCollisionShape* colShape, int index, UT_Vector3 t, UT_Vector3 r, UT_Vector3 cog_r, BRigidBody* compound)
{
	//compute final transformation
	btTransform trFinal;
	trFinal.setIdentity();
	trFinal.setOrigin( get_bullet_V3(t) );
	trFinal.getBasis().setEulerZYX(r[0], r[1], r[2]);

	//compute init transformation
//	btMatrix3x3 trCogR;
//	trCogR.setIdentity();
//	trCogR.setEulerZYX(cog_r[0], cog_r[1], cog_r[2]);

	//set final like "cog_r" multiply "r"
	//trFinal.setBasis( trCogR * trFinal.getBasis() );
	//trFinal.setBasis( trFinal.getBasis()*trCogR );

	//init rigid body settings
	btVector3 localInertia(0,0,0);
	colShape->calculateLocalInertia(1.0f, localInertia);
	btDefaultMotionState* myMotionState = new btDefaultMotionState(trFinal);
	btRigidBody::btRigidBodyConstructionInfo rbInfo(1.0f, myMotionState, colShape, localInertia);

	//create body
	BRigidBody* body = new BRigidBody(type, rbInfo, index, compound);
	body->setContactProcessingThreshold(BT_LARGE_FLOAT);
	
	//compute and set init values
	btQuaternion q;
	q.setEulerZYX(r[2], r[1], r[0]);
	body->setPosOld(get_bullet_V3(t));
	body->setRotOld(q);
	body->setPosCurrent(get_bullet_V3(t));
	body->setRotCurrent(q);
//	body->setInitRotation(trCogR);

	if(compound)	body->addChildShape(compound);			//set body(shape) to parent
	else			m_dynamicsWorld->addRigidBody(body);	//set body to world

	addOBJ(index, body);	//add body to our "index" array

	return body;
}


void BWorld::addOBJ(int index, BRigidBody* body)
{
	//realloc array If It needs it
	if(index >= m_objs.size())
	{
		int old_size = m_objs.size();
		m_objs.resize(index+1+EXTRA_ALLOC);
		memset(&m_objs[old_size], 0, (m_objs.size()-old_size)*sizeof(BRigidBody*));	//zero array
	}

	//delete old body
	if(m_objs[index])
		deleteOBJ(index);

	//set new body
	m_objs[index] = body;
}

void BWorld::startUpdateObjects()
{
	for(int i=0; i < m_objs.size(); i++)
	{
		BRigidBody* body = m_objs[i];
		if(body)
			body->setUpdate(false);
	}
}

void BWorld::endUpdateObjects()
{
	for(int i=0; i < m_objs.size(); i++)
	{
		BRigidBody* body = m_objs[i];
		if(body)
		{
			if(!body->isUpdated())
				deleteOBJ(i);
		}
	}
}

void BWorld::deleteOBJ(int i)
{
	BRigidBody* body = m_objs[i];
	if(body)
	{
		if(!body->hasCompound())
			m_dynamicsWorld->removeCollisionObject(body);
		delete body;
	}
	m_objs[i] = 0;
}

BRigidBody* BWorld::getBody(int i)
{
	if(i >= 0 && i < m_objs.size())
		return m_objs[i];
	return 0;
}



void BWorld::convert_world_to_parent(BRigidBody* body, BRigidBody* compound)
{
	m_dynamicsWorld->removeCollisionObject(body);
	body->addChildShape(compound);
}
void BWorld::convert_parent_to_world(BRigidBody* body)
{
	body->removeChildShape();
	m_dynamicsWorld->addRigidBody(body);
}
void BWorld::convert_parent_to_parent(BRigidBody* body, BRigidBody* compound)
{
	body->removeChildShape();
	body->addChildShape(compound);
}












BConstraint* BWorld::addConstraints(int index, BRigidBody* body_1, BRigidBody* body_2, UT_Vector3 wpos_1, UT_Vector3 wpos_2)
{
	if(!body_1)	return 0;
	if(!body_2)	return 0;

	if(body_1->hasCompound())	body_1 = body_1->getCompound();
	if(body_2->hasCompound())	body_2 = body_2->getCompound();

	//compute tranformation for A and B
	btTransform world_a, rel_a;
	btTransform world_b, rel_b;
	world_a.setIdentity();
	world_b.setIdentity();
	world_a.setOrigin( get_bullet_V3(wpos_1) );
	world_b.setOrigin( get_bullet_V3(wpos_2) );
	rel_a = body_1->getWorldTransform().inverseTimes(world_a);
	rel_b = body_2->getWorldTransform().inverseTimes(world_b);

	bool useLinearReferenceFrameA = false;	//use fixed frame A for linear limits
	BConstraint* c = new BConstraint(index, *body_1, *body_2, rel_a, rel_b, useLinearReferenceFrameA);

	m_dynamicsWorld->addConstraint(c);
	c->resetEquilibrium();
	body_1->setActivationState(DISABLE_DEACTIVATION);
	body_2->setActivationState(DISABLE_DEACTIVATION);

	addCSTR(index, c);
	return c;
}

void BWorld::addCSTR(int index, BConstraint* c)
{
	//realloc array If It needs it
	if(index >= m_constrs.size())
	{
		int old_size = m_constrs.size();

		m_constrs.resize(index+1+EXTRA_ALLOC);
		m_remove_constrs.resize(index+1+EXTRA_ALLOC);

		memset(&m_constrs[old_size], 0, (m_constrs.size()-old_size)*sizeof(BConstraint*));	//zero array
		memset(&m_remove_constrs[old_size], 0, (m_remove_constrs.size()-old_size)*sizeof(bool));	//zero array
	}

	//delete old constraint
	if(m_constrs[index])
		deleteCSTR(index);

	//save new constraint
	m_constrs[index] = c;
	m_remove_constrs[index] = false;
}

void BWorld::startUpdateConstraints()
{
	for(int i=0; i < m_constrs.size(); i++)
	{
		BConstraint* cstr = m_constrs[i];
		if(cstr)
			cstr->setUpdate(false);
	}
}

void BWorld::endUpdateConstraints()
{
	for(int i=0; i < m_constrs.size(); i++)
	{
		BConstraint* cstr = m_constrs[i];
		if(cstr)
			if(!cstr->isUpdated())
				deleteCSTR(i);
	}
}

void BWorld::deleteCSTR(int i)
{
	BConstraint* cstr = m_constrs[i];
	if(cstr)
	{
		m_dynamicsWorld->removeConstraint(cstr);
		delete cstr;
	}

	m_constrs[i] = 0;
	m_remove_constrs[i] = true;
}

void BWorld::breakConstraints(int fps, int substeps)
{
	for(int i=0; i < m_constrs.size(); i++)
	{
		BConstraint* cstr = m_constrs[i];
		if(cstr)
			if(cstr->tryBreakIt(fps, substeps))
				deleteCSTR(i);
	}

}

BConstraint* BWorld::getConstraints(int i, BRigidBody* body_1, BRigidBody* body_2)
{
	if(!body_1)	return 0;
	if(!body_2)	return 0;

	if(body_1->hasCompound())	body_1 = body_1->getCompound();
	if(body_2->hasCompound())	body_2 = body_2->getCompound();

	if(i >= 0 && i < m_constrs.size())
	{
		BConstraint* cstr = m_constrs[i];
		if(cstr)
		{
			if(cstr->isThis(body_1, body_2))
				return cstr;
		}
	}
	return 0;
}

bool BWorld::hasConstraintBroken(int i)
{
	return i >= 0 && i < m_remove_constrs.size() ? m_remove_constrs[i] : false;
}







void BWorld::applyForceRad(UT_Vector3 world_pos, int type, UT_Vector3 dir_normalized, float size, float rad, float fade)
{
	float rad2 = rad*rad;
	float rad_flip = 1.0f/rad;

	for(int i=0; i < m_objs.size(); i++)
	{
		BRigidBody* body = m_objs[i];
		if(body)
		{
			UT_Vector3 rel = body->getTranslateFull() - world_pos;

			float r2 = rel.length2();
			if(r2 < rad2)
			{
				float r = sqrt(r2);
				r *= rad_flip;	//<0,1>
				r = 1-r;	//reverse(outside==less)
				r = pow(r, fade);	//0-constant, 1-linear
				float new_size = size * r;	//change size of force

				addObjectForce(body, world_pos, type, dir_normalized, new_size);
			}
		}
	}
}

void BWorld::addObjectForce(BRigidBody* obj, UT_Vector3 world_pos, int type, UT_Vector3 dir_normalized, float size)
{
	UT_Vector3 force;
	if(type==SOP_Build::MAGNET)	//MAGNET - change force direction
	{
		UT_Vector3 rel = world_pos - obj->getTranslateFull();
		rel.normalize();
		dir_normalized = rel;
	}

	obj->setForce(world_pos, dir_normalized * size, type);
}

void BWorld::clearForces()
{
	for(int i=0; i < m_objs.size(); i++)
	{
		BRigidBody* body = m_objs[i];
		if(body)
			body->clearMyForces();
	}
	m_dynamicsWorld->clearForces();
}



void BWorld::uploadForcesToObjects()
{
	for(int i=0; i < m_objs.size(); i++)
	{
		BRigidBody* body = m_objs[i];
		if(body)
			body->uploadMyForces();
	}
}









void BWorld::stepWorld(float stepSize, int i_substep, int num_substep)
{
	//we need this, because Bullet( stepSimulation() ) clear all forces :(
	uploadForcesToObjects();

	//step world
	m_dynamicsWorld->stepSimulation(stepSize, 0);

	setKinematicPos( ((float)i_substep+1)/num_substep );

	blendConvexVertexes(num_substep);
}

void BWorld::setKinematicPos(float rel)
{
	for(int i=0; i < m_objs.size(); i++)
	{
		BRigidBody* body = m_objs[i];
		if(body)
		{
			if(body->isStaticOrKinematicObject() && body->isAproxChanged())
			{
				btTransform transform;
				transform.setIdentity();
				transform.setOrigin( body->getAproxPos(rel) );
				transform.setRotation( body->getAproxRot(rel) );

				body->getMotionState()->setWorldTransform(transform);	//set new transformation
			}
		}
	}
}



void BWorld::blendConvexVertexes(int num_substep)
{
	for(int i=0; i < m_objs.size(); i++)
	{
		BRigidBody* body = m_objs[i];
		if(body)
		{
			body->aproxConvexShape(num_substep);
			
			if(body->aproxDeformShape(num_substep))
				cleanCollisionWithDeformBody(body);
		}
	}
}





void BWorld::setGravity(UT_Vector3 gravity)
{
	m_dynamicsWorld->setGravity( get_bullet_V3(gravity) );
}

void BWorld::backUpVelocities()
{
	for(int i=0; i < m_objs.size(); i++)
	{
		BRigidBody* body = m_objs[i];
		if(body)
		{
			body->setOldVelocity( body->getVelocity() );
			body->setOldAngVelocity( body->getAngVelocity() );
		}
	}
}

void BWorld::cleanCollisionWithDeformBody(BRigidBody* body)
{
	m_dynamicsWorld->getBroadphase()->getOverlappingPairCache()->cleanProxyFromPairs(body->getBroadphaseHandle(), m_dynamicsWorld->getDispatcher());
}



void BWorld::addFloor()
{
	if(!m_floorBodyInWorld)	//we want to add our floor only once
	{
		m_dynamicsWorld->addRigidBody(m_floorBody);
		m_floorBodyInWorld = true;
	}
}
void BWorld::removeFloor()
{
	if(m_floorBodyInWorld)
	{
		m_dynamicsWorld->removeCollisionObject(m_floorBody);
		m_floorBodyInWorld = false;
	}
}





