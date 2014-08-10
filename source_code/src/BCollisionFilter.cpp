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
	m_ignore_groups.setExtraAlloc(EXTRA_ALLOC);

	m_collisionsOn = true;
	m_dynamicsWorld = dynamicsWorld;
	m_dynamicsWorld->getPairCache()->setOverlapFilterCallback(this);	//register this class to Bullet Library
}

BCollisionFilter::~BCollisionFilter(void)
{
	for(int i = 0; i < m_groups.size(); ++i)
		delete m_groups[i];

	for(int i = 0; i < m_ignore_groups.size(); ++i)
		delete m_ignore_groups[i];

	m_dynamicsWorld->getPairCache()->setOverlapFilterCallback(0);
}



void BCollisionFilter::run(bool on)
{
	m_collisionsOn = on;
}

void BCollisionFilter::clearGroups()
{
	m_groups.resize(0);	
}
void BCollisionFilter::clearIgnoreGroups()
{
	m_ignore_groups.resize(0);	
}


int BCollisionFilter::addGroup()
{
	m_groups.push_back( new MyVec<btBroadphaseProxy*>() );

	const size_t N = m_groups.size();
	return (int)N-1;
}

void BCollisionFilter::addProxy(int group_i, btBroadphaseProxy* proxies)
{
	m_groups[group_i]->push_back(proxies);
}


int BCollisionFilter::addIgnoreGroup()
{
	m_ignore_groups.push_back( new MyVec<btBroadphaseProxy*>() );

	const size_t N = m_ignore_groups.size();
	return (int)N-1;
}

void BCollisionFilter::addIgnoreProxy(int group_i, btBroadphaseProxy* proxies)
{
	m_ignore_groups[group_i]->push_back(proxies);
}


void BCollisionFilter::optimizeIgnoreGroups()
{
	for(int i = 0; i < m_ignore_groups.size(); i++)
	{
		const size_t N = m_ignore_groups[i]->size();

		if(N < 2)
		{
			delete m_ignore_groups[i];
			m_ignore_groups.erase(i);
			i--;
			continue;
		}

		for(int ii = i+1; ii < m_ignore_groups.size(); ii++)
		{
			if(N==m_ignore_groups[ii]->size())	//same group size!
			{
				bool find_duplicate = true;
				for(int j = 0; j < N; j++)
				{
					if(!m_ignore_groups[ii]->find(m_ignore_groups[i]->get(j)))
					{
						find_duplicate = false;
						break;
					}
				}

				if(find_duplicate)
				{
					delete m_ignore_groups[ii];
					m_ignore_groups.erase(ii);
					ii--;
				}
			}
		}
	}

}


bool BCollisionFilter::needBroadphaseCollision(btBroadphaseProxy* proxy0, btBroadphaseProxy* proxy1) const
{
	if(!m_collisionsOn)		//don't compute collisions
		return false;


	if(m_ignore_groups.size())
	{
		for(int i = 0; i < m_ignore_groups.size(); i++)
		{
			if(m_ignore_groups[i]->find(proxy0, proxy1))
				return false;
		}
	}

	if(m_groups.size())
	{
		for(int i = 0; i < m_groups.size(); i++)
		{
			if(m_groups[i]->find(proxy0, proxy1))
				return true;	//return true when pairs need collision
		}
		return false;
	}


	return true;
}


