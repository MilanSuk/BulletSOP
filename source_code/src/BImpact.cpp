/**
BulletSOP v2.0
Copyright (c)2010-2013 Suk Milan. All Rights Reserved.
This software is provided 'as-is', without any express or implied warranty.
In no event will the authors be held liable for any damages arising from the use of this software.
This file may not be redistributed in whole or significant part.
This notice may not be removed or altered from any source distribution.
*/
#include "stdafx.h"
#include "BImpact.h"

#include "BRigidBody.h"
#include "BWorld.h"
#include "SImpact.h"


///This function is called from Bullet Library
void tickCallback(btDynamicsWorld *world, btScalar timeStep)
{
	BImpact* data = (BImpact*)world->getWorldUserInfo();
	SImpact* imp = data->getImpact();

	if(!data->isRun() || !imp)
		return;

	const int numManifolds = world->getDispatcher()->getNumManifolds();
	for(int i=0; i < numManifolds; i++)
	{
		btPersistentManifold* manifold = world->getDispatcher()->getManifoldByIndexInternal(i);
		if(!manifold->getNumContacts())
			continue;

		const BRigidBody* bodyA = static_cast<const BRigidBody*>(manifold->getBody0());
		const BRigidBody* bodyB = static_cast<const BRigidBody*>(manifold->getBody1());

		for(int k=0; k < manifold->getNumContacts(); k++)
		{
			btManifoldPoint& pt = manifold->getContactPoint(k);

			if(pt.m_appliedImpulse > 0)	//checks impulse on contact point
			{
				//get data from bullet
				UT_Vector3 wpos = get_hdk_V3( pt.getPositionWorldOnA() );
				int lnA = bodyA->getLine();
				int lnB = bodyB->getLine();
				UT_Vector3 vA = bodyA->getOldVelocity();
				UT_Vector3 vB = bodyB->getOldVelocity();	
				UT_Vector3 avA = bodyA->getOldAngVelocity()* RAD_TO_DEG;
				UT_Vector3 avB = bodyB->getOldAngVelocity()* RAD_TO_DEG;

				//create and fill data into houdini
				GA_Offset offImp = imp->add();
				imp->setIndexA(offImp, bodyA->getIndex());
				imp->setIndexB(offImp, bodyB->getIndex());
				imp->setImpuls(offImp, pt.m_appliedImpulse);
				imp->setPos(offImp, wpos);
				imp->setLinearVelocityA(offImp, vA);
				imp->setLinearVelocityB(offImp, vB);
				imp->setAngularVelocityA(offImp, avA);
				imp->setAngularVelocityB(offImp, avB);
				imp->setLineA(offImp, lnA);
				imp->setLineB(offImp, lnB);
			}
		}


	}
}



BImpact::BImpact(btDiscreteDynamicsWorld* dynamicsWorld)
{
	m_dynamicsWorld = dynamicsWorld;
	m_run = false;

	createData();

	if(dynamicsWorld)
		m_dynamicsWorld->setInternalTickCallback(tickCallback, static_cast<void *>(this));	//register function in Bullet Library
}

BImpact::~BImpact(void)
{
	if(m_dynamicsWorld)
		m_dynamicsWorld->setInternalTickCallback(0);	//unregister out function

	clearData();
}


void BImpact::clearData()
{
	if(m_gdp)
		delete m_gdp;

	if(m_impact)
		delete m_impact;

	m_boss = 0;
	m_gdp = 0;
	m_impact = 0;
}



void BImpact::createData()
{
	m_gdp = new GU_Detail();
	m_boss = SOPBoss( new UT_Interrupt() );

	m_impact = new SImpact();
	m_impact->init(&m_boss, m_gdp);
}

void BImpact::resetData()
{
	clearData();
	createData();
}

SImpact* BImpact::getImpact() const
{
	return m_impact;
}
GU_Detail* BImpact::getData() const
{
	return m_gdp;
}


void BImpact::setRun(bool v)
{
	m_run = v;
}
bool BImpact::isRun() const
{
	return m_run;
}
