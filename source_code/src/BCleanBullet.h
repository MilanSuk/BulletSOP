/**
BulletSOP v2.0
Copyright (c)2010-2013 Suk Milan. All Rights Reserved.
This software is provided 'as-is', without any express or implied warranty.
In no event will the authors be held liable for any damages arising from the use of this software.
This file may not be redistributed in whole or significant part.
This notice may not be removed or altered from any source distribution.
*/
#pragma once

#include "EThread.h"

/**
Bullet World which need to be delete from memory
Class supports freeing data in separ thread(inhereit from EThread class)
*/
class ECleanBullet : public EThread
{
public:
	btDiscreteDynamicsWorld* m_dynamicsWorld;
	btDefaultCollisionConfiguration* m_collisionConfiguration;
	btCollisionDispatcher* m_dispatcher;
	btBroadphaseInterface* m_broadphase;
	btConstraintSolver* m_solver;
	btThreadSupportInterface* m_threadSupportSolver;
	btAlignedObjectArray<btCollisionShape*> m_shapes;

public:
	virtual void run()
	{
		if(m_dynamicsWorld)
		{
			//deletes constraints
			for(int i=m_dynamicsWorld->getNumConstraints()-1; i>=0 ;i--)
			{
				btTypedConstraint* constraint = m_dynamicsWorld->getConstraint(i);
				m_dynamicsWorld->removeConstraint(constraint);
				delete constraint;
			}

			//deletes objects
			for(int i=m_dynamicsWorld->getNumCollisionObjects()-1; i>=0 ;i--)
			{
				btCollisionObject* obj = m_dynamicsWorld->getCollisionObjectArray()[i];
				m_dynamicsWorld->removeCollisionObject(obj);
				delete obj;	//deletes motion state
			}

			for(int i=0; i < m_shapes.size(); i++)
				delete m_shapes[i];

			delete m_dynamicsWorld;
		}

		if(m_solver)				delete m_solver;
		if(m_threadSupportSolver)	delete m_threadSupportSolver;
		if(m_broadphase)			delete m_broadphase;
		if(m_dispatcher)			delete m_dispatcher;
		if(m_collisionConfiguration)delete m_collisionConfiguration;
	}

};


/**
Array of Bullet Worlds which need to be remove from memory
*/
class ECleanBullets
{
private:
	MyVec<ECleanBullet*> s_cleans;

public:
	virtual ~ECleanBullets()
	{
		for(int i=0; i < s_cleans.size(); i++)
			delete s_cleans[i];
		s_cleans.clear();
	}

	///add Bullet world to array and remove it in separ thread
	void addAndRun(ECleanBullet* c)
	{
		s_cleans.push_back(c);
		c->start();
	}
};
