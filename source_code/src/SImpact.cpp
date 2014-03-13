/**
BulletSOP v2.0
Copyright (c)2010-2013 Suk Milan. All Rights Reserved.
This software is provided 'as-is', without any express or implied warranty.
In no event will the authors be held liable for any damages arising from the use of this software.
This file may not be redistributed in whole or significant part.
This notice may not be removed or altered from any source distribution.
*/
#include "stdafx.h"
#include "SImpact.h"


SImpact::SImpact(void)
{
}

SImpact::~SImpact(void)
{
}


void SImpact::init(SOPBoss* boss, GU_Detail* gdp)
{
	setBasic(boss, gdp);

	m_attr_bullet_indexA = SHelper::addIntPointAttr(m_gdp, "bullet_indexA", 1, -1);
	m_attr_bullet_indexB = SHelper::addIntPointAttr(m_gdp, "bullet_indexB", 1, -1);
	m_attr_bullet_impulse = SHelper::addFloatPointAttr(m_gdp, "bullet_impulse", 1);

	m_attr_bullet_vA = SHelper::addFloatPointAttr(m_gdp, "bullet_vA", 3);
	m_attr_bullet_vB = SHelper::addFloatPointAttr(m_gdp, "bullet_vB", 3);
	m_attr_bullet_avA = SHelper::addFloatPointAttr(m_gdp, "bullet_avA", 3);
	m_attr_bullet_avB = SHelper::addFloatPointAttr(m_gdp, "bullet_avB", 3);

	m_attr_bullet_lineA = SHelper::addIntPointAttr(m_gdp, "bullet_lineA", 1, -1);
	m_attr_bullet_lineB = SHelper::addIntPointAttr(m_gdp, "bullet_lineB", 1, -1);

	m_gdp->addVariableName("P", "BIP");
	m_gdp->addVariableName("bullet_indexA", "BIA");
	m_gdp->addVariableName("bullet_indexB", "BIB");
	m_gdp->addVariableName("bullet_impulse", "BII");

}
	
void SImpact::initFind(SOPBoss* boss, GU_Detail* gdp)
{
	setBasic(boss, gdp);

	m_attr_bullet_indexA = SHelper::findPointAttrI(m_gdp, "bullet_indexA");
	m_attr_bullet_indexB = SHelper::findPointAttrI(m_gdp, "bullet_indexB");

	m_attr_bullet_vA = SHelper::findPointAttrF(m_gdp, "bullet_vA");
	m_attr_bullet_vB = SHelper::findPointAttrF(m_gdp, "bullet_vB");
	m_attr_bullet_avA = SHelper::findPointAttrF(m_gdp, "bullet_avA");
	m_attr_bullet_avB = SHelper::findPointAttrF(m_gdp, "bullet_avB");

	m_attr_bullet_lineA = SHelper::findPointAttrI(m_gdp, "bullet_lineA");
	m_attr_bullet_lineB = SHelper::findPointAttrI(m_gdp, "bullet_lineB");
	

	if(m_attr_bullet_indexA.isInvalid())THROW_SOP("SImpact: No \"bullet_indexA\" point attribute", 0);
	if(m_attr_bullet_indexB.isInvalid())THROW_SOP("SImpact: No \"bullet_indexB\" point attribute", 0);

	if(m_attr_bullet_vA.isInvalid())THROW_SOP("SImpact: No \"bullet_vA\" point attribute", 0);
	if(m_attr_bullet_vB.isInvalid())THROW_SOP("SImpact: No \"bullet_vB\" point attribute", 0);
	if(m_attr_bullet_avA.isInvalid())THROW_SOP("SImpact: No \"bullet_avA\" point attribute", 0);
	if(m_attr_bullet_avB.isInvalid())THROW_SOP("SImpact: No \"bullet_avB\" point attribute", 0);

	if(m_attr_bullet_lineA.isInvalid())THROW_SOP("SImpact: No \"bullet_lineA\" point attribute", 0);
	if(m_attr_bullet_lineB.isInvalid())THROW_SOP("SImpact: No \"bullet_lineB\" point attribute", 0);
}



GA_Offset SImpact::add()
{
	return m_gdp->appendPointOffset();
}



//structure
struct Pair2
{
	int indexB;
	GA_Offset ptoff;
};
typedef MyVec<Pair2> PAIRS2;

void SImpact::deleteDuplicity()
{
	BOSSP_START;

	MyVec<PAIRS2> pairs;
	pairs.setExtraAlloc(EXTRA_ALLOC);

	//generate pairs
	GA_Offset ptoff;
	GA_FOR_ALL_PTOFF(m_gdp, ptoff)
	{
		int indexA = getIndexA(ptoff);
		int indexB = getIndexB(ptoff);

		if(indexA==indexB)	//error
			continue;

		if(indexA > indexB)
		{
			int c = indexA;
			indexA = indexB;
			indexB = c;
		}

		//resize
		pairs.resizeAdd(indexA+1);

		//save it
		Pair2 p = {indexB, ptoff};
		pairs[indexA].push_back(p);

		BOSSP_INTERRUPT_EMPTY;
	}

	//delete duplicity rows
	for(int i=0; i < pairs.size(); i++)
	{
		PAIRS2* pp = &pairs[i];
		int n = (int)(*pp).size();

		for(int j1=0; j1 < n-1; j1++)
			for(int j2=j1+1; j2 < n; j2++)
				if((*pp)[j1].indexB==(*pp)[j2].indexB)
				{
					m_gdp->destroyPointOffset((*pp)[j2].ptoff);
					break;
				}

		BOSSP_INTERRUPT_EMPTY;
	}

	BOSSP_END;
}




int SImpact::getIndexA(const GA_Offset &off) const
{
	return m_attr_bullet_indexA(off);
}
int SImpact::getIndexB(const GA_Offset &off) const
{
	return m_attr_bullet_indexB(off);
}





void SImpact::setIndexA(const GA_Offset &off, int i)
{
	m_attr_bullet_indexA.set(off, i);
}
void SImpact::setIndexB(const GA_Offset &off, int i)
{
	m_attr_bullet_indexB.set(off, i);
}

void SImpact::setImpuls(const GA_Offset &off, float v)
{
	m_attr_bullet_impulse.set(off, v);
}

void SImpact::setPos(const GA_Offset &off, UT_Vector3 v)
{
	m_gdp->setPos3(off, v);
}
void SImpact::setLinearVelocityA(const GA_Offset &off, UT_Vector3 v)
{
	m_attr_bullet_vA.set(off, 0, v[0]);
	m_attr_bullet_vA.set(off, 1, v[1]);
	m_attr_bullet_vA.set(off, 2, v[2]);
}
void SImpact::setLinearVelocityB(const GA_Offset &off, UT_Vector3 v)
{
	m_attr_bullet_vB.set(off, 0, v[0]);
	m_attr_bullet_vB.set(off, 1, v[1]);
	m_attr_bullet_vB.set(off, 2, v[2]);
}
void SImpact::setAngularVelocityA(const GA_Offset &off, UT_Vector3 v)
{
	m_attr_bullet_avA.set(off, 0, v[0]);
	m_attr_bullet_avA.set(off, 1, v[1]);
	m_attr_bullet_avA.set(off, 2, v[2]);
}
void SImpact::setAngularVelocityB(const GA_Offset &off, UT_Vector3 v)
{
	m_attr_bullet_avB.set(off, 0, v[0]);
	m_attr_bullet_avB.set(off, 1, v[1]);
	m_attr_bullet_avB.set(off, 2, v[2]);
}
void SImpact::setLineA(const GA_Offset &off, int i)
{
	m_attr_bullet_lineA.set(off, i);
}
void SImpact::setLineB(const GA_Offset &off, int i)
{
	m_attr_bullet_lineB.set(off, i);
}




