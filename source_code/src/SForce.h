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
SForce inherit from SHelper.
SForce creates attribute structure and store data about forces.
*/
class SForce : public SHelper
{
private:
	GA_RWHandleI m_attr_bt_type;
	GA_RWHandleV3 m_attr_bt_dir;
	GA_RWHandleF m_attr_bt_scale;

	GA_RWHandleI m_attr_bt_index;
	GA_RWHandleF m_attr_bt_rad;
	GA_RWHandleF m_attr_bt_fade;


public:
	SForce(void);
	virtual ~SForce(void);

public:
	void init(SOPBoss* boss, GU_Detail* gdp);		///creates attribute structure
	void initFind(SOPBoss* boss, GU_Detail* gdp);	///tries to find attributes in gdp otherwise adds error and @return false

	GA_Offset add();	///adds force
	void addToGroup(GA_PointGroup* group);	///adds all forces(points) to group


	UT_Vector3 getPos(const GA_Offset &off) const;
	int getType(const GA_Offset &off) const;
	UT_Vector3 getDir(const GA_Offset &off) const;
	float getScale(const GA_Offset &off) const;

	int getIndex(const GA_Offset &off) const;
	float getRad(const GA_Offset &off) const;
	float getFade(const GA_Offset &off) const;

	
	void setPos(const GA_Offset &off, UT_Vector3 v);
	void setType(const GA_Offset &off, int i);
	void setDir(const GA_Offset &off, UT_Vector3 v);
	void setScale(const GA_Offset &off, float i);

	void setIndex(const GA_Offset &off, int i);
	void setRad(const GA_Offset &off, float i);
	void setFade(const GA_Offset &off, float i);
};
