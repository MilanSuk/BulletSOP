/**
BulletSOP v2.0
Copyright (c)2010-2013 Suk Milan. All Rights Reserved.
This software is provided 'as-is', without any express or implied warranty.
In no event will the authors be held liable for any damages arising from the use of this software.
This file may not be redistributed in whole or significant part.
This notice may not be removed or altered from any source distribution.
*/
#include "stdafx.h"
#include "SForce.h"


SForce::SForce(void)
{
}

SForce::~SForce(void)
{
}


void SForce::init(SOPBoss* boss, GU_Detail* gdp)
{
	setBasic(boss, gdp);

	m_attr_bt_type = SHelper::addIntPointAttr(m_gdp, "bt_type", 1, 1);
	m_attr_bt_dir = SHelper::addFloatVectorPointAttr(m_gdp, "bt_dir");
	m_attr_bt_scale = SHelper::addFloatPointAttr(m_gdp, "bt_scale", 1, 1);

	m_attr_bt_index = SHelper::addIntPointAttr(m_gdp, "bt_index", 1, -1);
	m_attr_bt_rad = SHelper::addFloatPointAttr(m_gdp, "bt_rad", 1, 1);
	m_attr_bt_fade = SHelper::addFloatPointAttr(m_gdp, "bt_fade", 1, 1);

	m_gdp->addVariableName("P", "BT");
	m_gdp->addVariableName("bt_type", "BTYPE");
	m_gdp->addVariableName("bt_dir", "BDIR");
	m_gdp->addVariableName("bt_scale", "BSCALE");

	m_gdp->addVariableName("bt_index", "BI");
	m_gdp->addVariableName("bt_rad", "BRAD");
	m_gdp->addVariableName("bt_fade", "BFADE");
}

void SForce::initFind(SOPBoss* boss, GU_Detail* gdp)
{
	setBasic(boss, gdp);

	m_attr_bt_type = SHelper::findPointAttrI(m_gdp, "bt_type");
	m_attr_bt_dir = SHelper::findPointAttrV3(m_gdp, "bt_dir");
	m_attr_bt_scale = SHelper::findPointAttrF(m_gdp, "bt_scale");

	m_attr_bt_index = SHelper::findPointAttrI(m_gdp, "bt_index");
	m_attr_bt_rad = SHelper::findPointAttrF(m_gdp, "bt_rad");
	m_attr_bt_fade = SHelper::findPointAttrF(m_gdp, "bt_fade");

	if(m_attr_bt_type.isInvalid())	THROW_SOP("SForce: No \"bt_type\" point attribute", 0);
	if(m_attr_bt_dir.isInvalid())	THROW_SOP("SForce: No \"bt_dir\" point attribute", 0);
	if(m_attr_bt_scale.isInvalid())	THROW_SOP("SForce: No \"bt_scale\" point attribute", 0);
	if(m_attr_bt_index.isInvalid())	THROW_SOP("SForce: No \"bt_index\" point attribute", 0);
	if(m_attr_bt_rad.isInvalid())	THROW_SOP("SForce: No \"bt_rad\" point attribute", 0);
	if(m_attr_bt_fade.isInvalid())	THROW_SOP("SForce: No \"bt_fade\" point attribute", 0);
}


GA_Offset SForce::add()
{
	return m_gdp->insertPointOffset();	//add point
}

void SForce::addToGroup(GA_PointGroup* group)
{
	GA_Offset ptoff;
	GA_FOR_ALL_PTOFF(m_gdp, ptoff)
		group->addOffset(ptoff);
}



UT_Vector3 SForce::getPos(const GA_Offset &off) const
{
	return m_gdp->getPos3(off);
}
int SForce::getType(const GA_Offset &off) const
{
	return m_attr_bt_type(off);
}
UT_Vector3 SForce::getDir(const GA_Offset &off) const
{
	return m_attr_bt_dir(off);
}
float SForce::getScale(const GA_Offset &off) const
{
	return m_attr_bt_scale(off);
}

int SForce::getIndex(const GA_Offset &off) const
{
	return m_attr_bt_index(off);
}
float SForce::getRad(const GA_Offset &off) const
{
	return m_attr_bt_rad(off);
}
float SForce::getFade(const GA_Offset &off) const
{
	return m_attr_bt_fade(off);
}




void SForce::setPos(const GA_Offset &off, UT_Vector3 v)
{
	m_gdp->setPos3(off, v);
}
void SForce::setType(const GA_Offset &off, int i)
{
	m_attr_bt_type.set(off, i);
}
void SForce::setDir(const GA_Offset &off, UT_Vector3 v)
{
	m_attr_bt_dir.set(off, v);
}
void SForce::setScale(const GA_Offset &off, float i)
{
	m_attr_bt_scale.set(off, i);
}

void SForce::setIndex(const GA_Offset &off, int i)
{
	m_attr_bt_index.set(off, i);
}
void SForce::setRad(const GA_Offset &off, float i)
{
	m_attr_bt_rad.set(off, i);
}
void SForce::setFade(const GA_Offset &off, float i)
{
	m_attr_bt_fade.set(off, i);
}


