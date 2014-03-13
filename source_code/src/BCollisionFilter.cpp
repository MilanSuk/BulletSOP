/**
BulletSOP v2.0
Copyright (c)2010-2013 Suk Milan. All Rights Reserved.
This software is provided 'as-is', without any express or implied warranty.
In no event will the authors be held liable for any damages arising from the use of this software.
This file may not be redistributed in whole or significant part.
This notice may not be removed or altered from any source distribution.
*/
#include "stdafx.h"
#include "BCollisionFilter.h"


BCollisionFilter::BCollisionFilter(btDiscreteDynamicsWorld* dynamicsWorld)
{
	m_groups.setExtraAlloc(EXTRA_ALLOC);

	m_collisionsOn = true;
	m_dynamicsWorld = dynamicsWorld;
	m_dynamicsWorld->getPairCache()->setOverlapFilterCallback(this);	//register this class to Bullet Library
}

BCollisionFilter::~BCollisionFilter(void)
{
	m_dynamicsWorld->getPairCache()->setOverlapFilterCallback(0);
}



void BCollisionFilter::run(bool on)
{
	m_collisionsOn = on;
}


int BCollisionFilter::addGroup()
{
	MyVec<btBroadphaseProxy*> v;
	m_groups.push_back(v);

	//printf("addGroup: %d\n", m_groups.size());
	return (int)m_groups.size()-1;
}


void BCollisionFilter::addProxy(int group_i, btBroadphaseProxy* proxies)
{
	m_groups[group_i].push_back(proxies);
}

void BCollisionFilter::clear()
{
	m_groups.clear();
}



bool BCollisionFilter::needBroadphaseCollision(btBroadphaseProxy* proxy0, btBroadphaseProxy* proxy1) const
{
	if(!m_collisionsOn)		//don't compute collisions
		return false;

	if(!m_groups.size())	//empty => compute all collisions
		return true;

	for(int i = 0; i < m_groups.size(); i++)
	{
		bool find_a = false;
		bool find_b = false;

		//try to find one group which includes both bodies
		for(int j = 0; j < m_groups[i].size(); j++)
		{
			if( m_groups[i][j]==proxy0 )
				find_a = true;

			if( m_groups[i][j]==proxy1 )
				find_b = true;

			if(find_a && find_b)
				return true;	//return true when pairs need collision
		}
	}

	return false;
}


