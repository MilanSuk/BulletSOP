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


/**
Class for checking If solver needs to compute collisions data between two bodies(Proxies)
*/
class BCollisionFilter : public btOverlapFilterCallback
{
private:
	btDiscreteDynamicsWorld* m_dynamicsWorld;
	MyVec< MyVec<btBroadphaseProxy*>* > m_groups;	//objects in group collide only between each other
	MyVec< MyVec<btBroadphaseProxy*>* > m_ignore_groups;	//objects in group don't collide

	bool m_collisionsOn;

public:
	BCollisionFilter(btDiscreteDynamicsWorld* dynamicsWorld);
	virtual ~BCollisionFilter(void);

	void clearGroups();
	void clearIgnoreGroups();
	void run(bool on);	///Turns on or off collision detections

	int addGroup();		///Adds group and index of group
	void addProxy(int group_i, btBroadphaseProxy* proxies);		///Adds body to group

	int addIgnoreGroup();		///Adds group and index of group
	void addIgnoreProxy(int group_i, btBroadphaseProxy* proxies);		///Adds body to group

	void optimizeIgnoreGroups();

	virtual bool needBroadphaseCollision(btBroadphaseProxy* proxy0, btBroadphaseProxy* proxy1) const;	///This virtual function is called from Bullet Library
};
