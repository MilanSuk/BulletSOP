/**
BulletSOP v2.0
Copyright (c)2010-2013 Suk Milan. All Rights Reserved.
This software is provided 'as-is', without any express or implied warranty.
In no event will the authors be held liable for any damages arising from the use of this software.
This file may not be redistributed in whole or significant part.
This notice may not be removed or altered from any source distribution.
*/
#pragma once


/**
SHelper class is base class which ouput geometry(m_gdp) and boss(m_boss), which can be set with setBasic(...) function.
Also This class includes error and warning system.
*/
class SHelper
{
protected:
	GU_Detail* m_gdp;		///geo output
	SOPBoss* m_boss;

public:
	SHelper(void);
	virtual ~SHelper(void);

protected:
	void setBasic(SOPBoss* boss, GU_Detail* gdp);

public:
	bool isInit() const;	///@return true If ouput(m_gdp) exists


public:
	static GA_RWHandleI addIntPointAttr(GU_Detail* gdp, const char* name, int size, int def=0);
	static GA_RWHandleF addFloatPointAttr(GU_Detail* gdp, const char* name, int size, float def=0);
	static GA_RWHandleV3 addFloatVectorPointAttr(GU_Detail* gdp, const char* name, float* def=0);

	static GA_RWHandleI addIntPrimitiveAttr(GU_Detail* gdp, const char* name, int size, int def=0);
	static GA_RWHandleF addFloatPrimitiveAttr(GU_Detail* gdp, const char* name, int size, float def=0);
	static GA_RWHandleV3 addFloatVectorPrimitiveAttr(GU_Detail* gdp, const char* name, float* def=0);

	static GA_RWHandleI addIntDetailAttr(GU_Detail* gdp, const char* name, int size, int def=0);
	static GA_RWHandleF addFloatDetailAttr(GU_Detail* gdp, const char* name, int size, float def=0);
	static GA_RWHandleV3 addFloatVectorDetailAttr(GU_Detail* gdp, const char* name, float* def=0);

	static GA_RWHandleI findPointAttrI(GU_Detail* gdp, const char* name);
	static GA_RWHandleF findPointAttrF(GU_Detail* gdp, const char* name);
	static GA_RWHandleV3 findPointAttrV3(GU_Detail* gdp, const char* name);

	static GA_RWHandleI findPrimitiveAttrI(GU_Detail* gdp, const char* name);
	static GA_RWHandleF findPrimitiveAttrF(GU_Detail* gdp, const char* name);
	static GA_RWHandleV3 findPrimitiveAttrV3(GU_Detail* gdp, const char* name);

	static GA_RWHandleI findDetailAttrI(GU_Detail* gdp, const char* name);
	static GA_RWHandleF findDetailAttrF(GU_Detail* gdp, const char* name);
	static GA_RWHandleV3 findDetailAttrV3(GU_Detail* gdp, const char* name);


	template<typename T>
	static void destroyPointAttr(GU_Detail* gdp, T attr)
	{
		gdp->destroyAttribute(GA_ATTRIB_POINT, attr.getAttribute()->getName());
	}
};
