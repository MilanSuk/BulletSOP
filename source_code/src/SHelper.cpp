/**
BulletSOP v2.0
Copyright (c)2010-2013 Suk Milan. All Rights Reserved.
This software is provided 'as-is', without any express or implied warranty.
In no event will the authors be held liable for any damages arising from the use of this software.
This file may not be redistributed in whole or significant part.
This notice may not be removed or altered from any source distribution.
*/
#include "stdafx.h"
#include "SHelper.h"

SHelper::SHelper(void)
{
	m_gdp = 0;
	m_boss = 0;
}

SHelper::~SHelper(void)
{
}



void SHelper::setBasic(SOPBoss* boss, GU_Detail* gdp)
{
	m_gdp = gdp;
	m_boss = boss;
}


bool SHelper::isInit() const
{
	return m_gdp!=0;
}



GA_RWHandleI SHelper::addIntPointAttr(GU_Detail* gdp, const char* name, int size, int def)
{
	return GA_RWHandleI( gdp->addIntTuple(GA_ATTRIB_POINT, name, size, GA_Defaults(def)).getAttribute() );
}
GA_RWHandleF SHelper::addFloatPointAttr(GU_Detail* gdp, const char* name, int size, float def)
{
	return GA_RWHandleF( gdp->addFloatTuple(GA_ATTRIB_POINT, name, size, GA_Defaults(def)).getAttribute() );
}
GA_RWHandleV3 SHelper::addFloatVectorPointAttr(GU_Detail* gdp, const char* name, float* def)
{
	GA_RWAttributeRef attr;
	if(def)	attr = gdp->addFloatTuple(GA_ATTRIB_POINT, name, 3, GA_Defaults(def, 3));
	else	attr = gdp->addFloatTuple(GA_ATTRIB_POINT, name, 3);

	attr.setTypeInfo(GA_TYPE_VECTOR);

	return GA_RWHandleV3( attr.getAttribute() );
}


GA_RWHandleI SHelper::addIntPrimitiveAttr(GU_Detail* gdp, const char* name, int size, int def)
{
	return GA_RWHandleI( gdp->addIntTuple(GA_ATTRIB_PRIMITIVE, name, size, GA_Defaults(def)).getAttribute() );
}
GA_RWHandleF SHelper::addFloatPrimitiveAttr(GU_Detail* gdp, const char* name, int size, float def)
{
	return GA_RWHandleF( gdp->addFloatTuple(GA_ATTRIB_PRIMITIVE, name, size, GA_Defaults(def)).getAttribute() );
}
GA_RWHandleV3 SHelper::addFloatVectorPrimitiveAttr(GU_Detail* gdp, const char* name, float* def)
{
	GA_RWAttributeRef attr;
	if(def)	attr = gdp->addFloatTuple(GA_ATTRIB_PRIMITIVE, name, 3, GA_Defaults(def, 3));
	else	attr = gdp->addFloatTuple(GA_ATTRIB_PRIMITIVE, name, 3);

	attr.setTypeInfo(GA_TYPE_VECTOR);

	return GA_RWHandleV3( attr.getAttribute() );
}


GA_RWHandleI SHelper::addIntDetailAttr(GU_Detail* gdp, const char* name, int size, int def)
{
	return GA_RWHandleI( gdp->addIntTuple(GA_ATTRIB_GLOBAL, name, size, GA_Defaults(def)).getAttribute() );
}
GA_RWHandleF SHelper::addFloatDetailAttr(GU_Detail* gdp, const char* name, int size, float def)
{
	return GA_RWHandleF( gdp->addFloatTuple(GA_ATTRIB_GLOBAL, name, size, GA_Defaults(def)).getAttribute() );
}
GA_RWHandleV3 SHelper::addFloatVectorDetailAttr(GU_Detail* gdp, const char* name, float* def)
{
	GA_RWAttributeRef attr;
	if(def)	attr = gdp->addFloatTuple(GA_ATTRIB_GLOBAL, name, 3, GA_Defaults(def, 3));
	else	attr = gdp->addFloatTuple(GA_ATTRIB_GLOBAL, name, 3);

	attr.setTypeInfo(GA_TYPE_VECTOR);

	return GA_RWHandleV3( attr.getAttribute() );
}


GA_RWHandleI SHelper::findPointAttrI(GU_Detail* gdp, const char* name)
{
	return GA_RWHandleI( gdp->findPointAttribute(name).getAttribute() );
}
GA_RWHandleF SHelper::findPointAttrF(GU_Detail* gdp, const char* name)
{
	return GA_RWHandleF( gdp->findPointAttribute(name).getAttribute() );
}
GA_RWHandleV3 SHelper::findPointAttrV3(GU_Detail* gdp, const char* name)
{
	return GA_RWHandleV3( gdp->findPointAttribute(name).getAttribute() );
}


GA_RWHandleI SHelper::findPrimitiveAttrI(GU_Detail* gdp, const char* name)
{
	return GA_RWHandleI( gdp->findPrimitiveAttribute(name).getAttribute() );
}
GA_RWHandleF SHelper::findPrimitiveAttrF(GU_Detail* gdp, const char* name)
{
	return GA_RWHandleF( gdp->findPrimitiveAttribute(name).getAttribute() );
}
GA_RWHandleV3 SHelper::findPrimitiveAttrV3(GU_Detail* gdp, const char* name)
{
	return GA_RWHandleV3( gdp->findPrimitiveAttribute(name).getAttribute() );
}


GA_RWHandleI SHelper::findDetailAttrI(GU_Detail* gdp, const char* name)
{
	return GA_RWHandleI( gdp->findGlobalAttribute(name).getAttribute() );
}
GA_RWHandleF SHelper::findDetailAttrF(GU_Detail* gdp, const char* name)
{
	return GA_RWHandleF( gdp->findGlobalAttribute(name).getAttribute() );
}
GA_RWHandleV3 SHelper::findDetailAttrV3(GU_Detail* gdp, const char* name)
{
	return GA_RWHandleV3( gdp->findGlobalAttribute(name).getAttribute() );
}

