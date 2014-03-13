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


/**
SImpact inherit from SHelper.
SImpact stores, modify and classify(deleteDuplicity()) data about collision(s) between objects.
*/
class SImpact :	public SHelper
{
private:
	GA_RWHandleI m_attr_bullet_indexA;
	GA_RWHandleI m_attr_bullet_indexB;
	GA_RWHandleF m_attr_bullet_impulse;

	GA_RWHandleF m_attr_bullet_vA;
	GA_RWHandleF m_attr_bullet_vB;
	GA_RWHandleF m_attr_bullet_avA;
	GA_RWHandleF m_attr_bullet_avB;

	GA_RWHandleI m_attr_bullet_lineA;
	GA_RWHandleI m_attr_bullet_lineB;

public:
	SImpact(void);
	virtual ~SImpact(void);

public:
	void init(SOPBoss* boss, GU_Detail* gdp);		///creates attribute structure
	void initFind(SOPBoss* boss, GU_Detail* gdp);	///tries to find attributes in gdp otherwise adds error and @return false

	GA_Offset add();		///adds impact info
	void deleteDuplicity();		///deletes duplicity rows => row with same indexA and indexB(can be switch)


	int getIndexA(const GA_Offset &off) const;
	int getIndexB(const GA_Offset &off) const;

	void setIndexA(const GA_Offset &off, int i);
	void setIndexB(const GA_Offset &off, int i);
	void setImpuls(const GA_Offset &off, float v);
	void setPos(const GA_Offset &off, UT_Vector3 v);
	void setLinearVelocityA(const GA_Offset &off, UT_Vector3 v);
	void setLinearVelocityB(const GA_Offset &off, UT_Vector3 v);
	void setAngularVelocityA(const GA_Offset &off, UT_Vector3 v);
	void setAngularVelocityB(const GA_Offset &off, UT_Vector3 v);
	void setLineA(const GA_Offset &off, int i);
	void setLineB(const GA_Offset &off, int i);

};
