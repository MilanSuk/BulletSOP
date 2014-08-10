/**
BulletSOP v2.0
Copyright (c)2010-2013 Suk Milan. All Rights Reserved.
This software is provided 'as-is', without any express or implied warranty.
In no event will the authors be held liable for any damages arising from the use of this software.
This file may not be redistributed in whole or significant part.
This notice may not be removed or altered from any source distribution.
*/
#pragma once
#include "SHelper.h"

class SShape;


/**
SConstraint inherit from SHelper.
SConstraint stores, modify and classify(deleteDuplicity()) data about constraints.
SConstraint also includes functions( createMinimumDistanceConstraints() ) for generating constraints from geometry.
*/
class SConstraint : public SHelper
{
private:
	SShape* m_input;	//geometry for generating constraints according points distance

	GA_RWHandleI m_attr_bt_index;
	GA_RWHandleI m_attr_bt_index_a;
	GA_RWHandleI m_attr_bt_index_b;
	GA_RWHandleF m_attr_bt_stiffness;
	GA_RWHandleF m_attr_bt_damping;
	GA_RWHandleF m_attr_bt_maxforce;
	GA_RWHandleF m_attr_bt_actforce;
	GA_RWHandleV3 m_attr_bt_linlowerlimit;
	GA_RWHandleV3 m_attr_bt_linupperlimit;
	GA_RWHandleV3 m_attr_bt_anglowerlimit;
	GA_RWHandleV3 m_attr_bt_angupperlimit;
	GA_RWHandleI m_attr_bt_weariness;
	GA_RWHandleI m_attr_bt_iter;
	GA_RWHandleI m_attr_bt_update;
	GA_RWHandleI m_attr_bt_recreate;
	GA_RWHandleI m_attr_bt_equilibrium;
	GA_RWHandleI m_attr_bt_collision;

	static int g_max_constr_index;	///This is not effective!!! Need to be improved!!!

public:
	SConstraint(void);
	virtual ~SConstraint(void);

public:
	void init(SOPBoss* boss, GU_Detail* gdp, SShape* input=NULL);		///creates attribute structure
	void initFind(SOPBoss* boss, GU_Detail* gdp, SShape* input=NULL);	///tries to find attributes in gdp otherwise adds error and @return false

	GEO_Primitive* add();	///adds constraint
	void addToGroup(GA_PrimitiveGroup* group);	///adds all forces(points) to group

	void createMinimumDistanceConstraints(float toler_distance, float stiffness, float damping, float max_force, bool weariness, int iter, bool collision, float memory_mult);	///generates constraints according minimum distance(toler_distance)
	void deleteDuplicity(const GA_PrimitiveGroup* myGroupPr);	///deletes duplicity rows => row with same indexA and indexB(can be switch)

	void setIndex(const GA_Offset &off, int i);
	void setIndexA(const GA_Offset &off, int i);
	void setIndexB(const GA_Offset &off, int i);
	void setPosA(GEO_Primitive* prim, UT_Vector3 v);
	void setPosB(GEO_Primitive* prim, UT_Vector3 v);
	void setStiffness(const GA_Offset &off, float v);
	void setDamping(const GA_Offset &off, float v);
	void setMaxForce(const GA_Offset &off, float v);
	void setWeariness(const GA_Offset &off, bool v);
	void setIter(const GA_Offset &off, int i);
	void setLL(const GA_Offset &off, UT_Vector3 v);
	void setLU(const GA_Offset &off, UT_Vector3 v);
	void setAL(const GA_Offset &off, UT_Vector3 v);
	void setAU(const GA_Offset &off, UT_Vector3 v);
	void setUpdate(const GA_Offset &off, bool v);
	void setRecreate(const GA_Offset &off, bool v);
	void setEquilibrium(const GA_Offset &off, bool v);
	void setLinearLock(GEO_Primitive* prim);
	void setActualForce(const GA_Offset &off, float v);
	void setCollision(const GA_Offset &off, bool v);

	UT_Vector3 getPosA(GEO_Primitive* prim) const;
	UT_Vector3 getPosB(GEO_Primitive* prim) const;
	int getIndex(const GA_Offset &off) const;
	int getIndexA(const GA_Offset &off) const;
	int getIndexB(const GA_Offset &off) const;
	int getIter(const GA_Offset &off) const;
	bool getWeariness(const GA_Offset &off) const;
	float getStiffness(const GA_Offset &off) const;
	float getDamping(const GA_Offset &off) const;
	float getMaxforce(const GA_Offset &off) const;
	UT_Vector3 getLinlowerlimit(const GA_Offset &off) const;
	UT_Vector3 getLinupperlimit(const GA_Offset &off) const;
	UT_Vector3 getAnglowerlimit(const GA_Offset &off) const;
	UT_Vector3 getAngupperlimit(const GA_Offset &off) const;
	bool getUpdate(const GA_Offset &off) const;
	bool getRecreate(const GA_Offset &off) const;
	bool getEquilibrium(const GA_Offset &off) const;
	bool getCollision(const GA_Offset &off) const;
};
