/**
BulletSOP v2.0
Copyright (c)2010-2013 Suk Milan. All Rights Reserved.
This software is provided 'as-is', without any express or implied warranty.
In no event will the authors be held liable for any damages arising from the use of this software.
This file may not be redistributed in whole or significant part.
This notice may not be removed or altered from any source distribution.
*/
#include "stdafx.h"
#include "SConstraint.h"
#include "SShape.h"
#include "Grid.h"

int SConstraint::g_max_constr_index = 0;	//This is not effective!!! Need to be improved!!!


SConstraint::SConstraint(void)
{
}
SConstraint::~SConstraint(void)
{
}


void SConstraint::init(SOPBoss* boss, GU_Detail* gdp, SShape* input)
{
	setBasic(boss, gdp);
	m_input = input;

	m_attr_bt_index = SHelper::addIntPrimitiveAttr(m_gdp, "bt_index", 1, -1);
	m_attr_bt_index_a = SHelper::addIntPrimitiveAttr(m_gdp, "bt_index_a", 1);
	m_attr_bt_index_b = SHelper::addIntPrimitiveAttr(m_gdp, "bt_index_b", 1);

	m_attr_bt_stiffness = SHelper::addFloatPrimitiveAttr(m_gdp, "bt_stiffness", 1);
	m_attr_bt_damping = SHelper::addFloatPrimitiveAttr(m_gdp, "bt_damping", 1);
	m_attr_bt_maxforce = SHelper::addFloatPrimitiveAttr(m_gdp, "bt_maxforce", 1);
	m_attr_bt_actforce = SHelper::addFloatPrimitiveAttr(m_gdp, "bt_actforce", 1);

	m_attr_bt_linlowerlimit = SHelper::addFloatVectorPrimitiveAttr(m_gdp, "bt_linlowerlimit");
	m_attr_bt_linupperlimit = SHelper::addFloatVectorPrimitiveAttr(m_gdp, "bt_linupperlimit");
	m_attr_bt_anglowerlimit = SHelper::addFloatVectorPrimitiveAttr(m_gdp, "bt_anglowerlimit");
	m_attr_bt_angupperlimit = SHelper::addFloatVectorPrimitiveAttr(m_gdp, "bt_angupperlimit");

	m_attr_bt_weariness = SHelper::addIntPrimitiveAttr(m_gdp, "bt_weariness", 1);
	m_attr_bt_iter = SHelper::addIntPrimitiveAttr(m_gdp, "bt_iter", 1, -1);
	m_attr_bt_update = SHelper::addIntPrimitiveAttr(m_gdp, "bt_update", 1);
	m_attr_bt_recreate = SHelper::addIntPrimitiveAttr(m_gdp, "bt_recreate", 1);
	m_attr_bt_equilibrium = SHelper::addIntPrimitiveAttr(m_gdp, "bt_equilibrium", 1);
	m_attr_bt_collision = SHelper::addIntPrimitiveAttr(m_gdp, "bt_collision", 1);


	m_gdp->addVariableName("bt_index", "BI");
	m_gdp->addVariableName("bt_index_a", "BIA");
	m_gdp->addVariableName("bt_index_b", "BIB");

	m_gdp->addVariableName("bt_stiffness", "BSTIFF");
	m_gdp->addVariableName("bt_damping", "BDAMP");
	m_gdp->addVariableName("bt_maxforce", "BMAXF");
	m_gdp->addVariableName("bt_actforce", "BACTF");

	m_gdp->addVariableName("bt_linlowerlimit", "BLL");
	m_gdp->addVariableName("bt_linupperlimit", "BLU");
	m_gdp->addVariableName("bt_anglowerlimit", "BAL");
	m_gdp->addVariableName("bt_angupperlimit", "BAU");

	m_gdp->addVariableName("bt_weariness", "BWEAR");
	m_gdp->addVariableName("bt_iter", "BITER");
	m_gdp->addVariableName("bt_update", "BUPDATE");
	m_gdp->addVariableName("bt_recreate", "BRECREATE");
	m_gdp->addVariableName("bt_equilibrium", "BEQU");
	m_gdp->addVariableName("bt_collision", "BCOL");
}



void SConstraint::initFind(SOPBoss* boss, GU_Detail* gdp, SShape* input)
{
	setBasic(boss, gdp);
	m_input = input;

	m_attr_bt_index = SHelper::findPrimitiveAttrI(m_gdp, "bt_index");
	m_attr_bt_index_a = SHelper::findPrimitiveAttrI(m_gdp, "bt_index_a");
	m_attr_bt_index_b = SHelper::findPrimitiveAttrI(m_gdp, "bt_index_b");

	m_attr_bt_stiffness = SHelper::findPrimitiveAttrF(m_gdp, "bt_stiffness");
	m_attr_bt_damping = SHelper::findPrimitiveAttrF(m_gdp, "bt_damping");
	m_attr_bt_maxforce = SHelper::findPrimitiveAttrF(m_gdp, "bt_maxforce");
	m_attr_bt_actforce = SHelper::findPrimitiveAttrF(m_gdp, "bt_actforce");

	m_attr_bt_linlowerlimit = SHelper::findPrimitiveAttrV3(m_gdp, "bt_linlowerlimit");
	m_attr_bt_linupperlimit = SHelper::findPrimitiveAttrV3(m_gdp, "bt_linupperlimit");
	m_attr_bt_anglowerlimit = SHelper::findPrimitiveAttrV3(m_gdp, "bt_anglowerlimit");
	m_attr_bt_angupperlimit = SHelper::findPrimitiveAttrV3(m_gdp, "bt_angupperlimit");

	m_attr_bt_weariness = SHelper::findPrimitiveAttrI(m_gdp, "bt_weariness");
	m_attr_bt_iter = SHelper::findPrimitiveAttrI(m_gdp, "bt_iter");
	m_attr_bt_update = SHelper::findPrimitiveAttrI(m_gdp, "bt_update");
	m_attr_bt_recreate = SHelper::findPrimitiveAttrI(m_gdp, "bt_recreate");
	m_attr_bt_equilibrium = SHelper::findPrimitiveAttrI(m_gdp, "bt_equilibrium");
	m_attr_bt_collision = SHelper::findPrimitiveAttrI(m_gdp, "bt_collision");

	

	if(m_attr_bt_index.isInvalid())		THROW_SOP("SConstraint: No \"bt_index\" primitive attribute", 0);
	if(m_attr_bt_index_a.isInvalid())	THROW_SOP("SConstraint: No \"bt_index_a\" primitive attribute", 0);
	if(m_attr_bt_index_b.isInvalid())	THROW_SOP("SConstraint: No \"bt_index_b\" primitive attribute", 0);

	if(m_attr_bt_stiffness.isInvalid())	THROW_SOP("SConstraint: No \"bt_stiffness\" primitive attribute", 0);
	if(m_attr_bt_damping.isInvalid())	THROW_SOP("SConstraint: No \"bt_damping\" primitive attribute", 0);
	if(m_attr_bt_maxforce.isInvalid())	THROW_SOP("SConstraint: No \"bt_maxforce\" primitive attribute", 0);
	if(m_attr_bt_actforce.isInvalid())	THROW_SOP("SConstraint: No \"bt_actforce\" primitive attribute", 0);

	if(m_attr_bt_linlowerlimit.isInvalid())	THROW_SOP("SConstraint: No \"bt_linlowerlimit\" primitive attribute", 0);
	if(m_attr_bt_linupperlimit.isInvalid())	THROW_SOP("SConstraint: No \"bt_linupperlimit\" primitive attribute", 0);
	if(m_attr_bt_anglowerlimit.isInvalid())	THROW_SOP("SConstraint: No \"bt_anglowerlimit\" primitive attribute", 0);
	if(m_attr_bt_angupperlimit.isInvalid())	THROW_SOP("SConstraint: No \"bt_angupperlimit\" primitive attribute", 0);
	
	if(m_attr_bt_weariness.isInvalid())		THROW_SOP("SConstraint: No \"bt_weariness\" primitive attribute", 0);
	if(m_attr_bt_iter.isInvalid())			THROW_SOP("SConstraint: No \"bt_iter\" primitive attribute", 0);
	if(m_attr_bt_update.isInvalid())		THROW_SOP("SConstraint: No \"bt_update\" primitive attribute", 0);
	if(m_attr_bt_recreate.isInvalid())		THROW_SOP("SConstraint: No \"bt_recreate\" primitive attribute", 0);
	if(m_attr_bt_equilibrium.isInvalid())	THROW_SOP("SConstraint: No \"bt_equilibrium\" primitive attribute", 0);
	if(m_attr_bt_collision.isInvalid())		THROW_SOP("SConstraint: No \"bt_collision\" primitive attribute", 0);

}


GEO_Primitive* SConstraint::add()
{
	GEO_Primitive* prim = GU_PrimPoly::build(m_gdp, 2, GU_POLY_OPEN);	//add primitive(line)
	setIndex(prim->getMapOffset(), g_max_constr_index++);
	return prim;
}


void SConstraint::addToGroup(GA_PrimitiveGroup* group)
{
	GEO_Primitive* prim;
	GA_FOR_ALL_PRIMITIVES(m_gdp, prim)
		group->addOffset(prim->getMapOffset());
}



//structure
struct Pair
{
	int indexB;
	GEO_Primitive* prim;
};
typedef MyVec<Pair> PAIRS;

void SConstraint::deleteDuplicity(const GA_PrimitiveGroup* myGroupPr)
{
	BOSSP_START;

	MyVec<PAIRS> pairs;
	pairs.setExtraAlloc(EXTRA_ALLOC);

	//generate pairs
	GEO_Primitive* prim;
	GA_FOR_ALL_OPT_GROUP_PRIMITIVES(m_gdp, myGroupPr, prim)
	{
		int indexA = getIndexA(prim->getMapOffset());
		int indexB = getIndexB(prim->getMapOffset());

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
		Pair p = {indexB, prim};
		pairs[indexA].push_back(p);

		BOSSP_INTERRUPT_EMPTY;
	}

	//add duplicity constraints(primitives) to array
	UT_Array<GEO_Primitive*> arr;	//GEO_PrimitivePtrArray arr;
	const int N = (int)pairs.size();
	for(int i=0; i < N; i++)
	{
		PAIRS* pp = &pairs[i];
		int n = (int)(*pp).size();
		for(int j1=0; j1 < n-1; j1++)
		{
			for(int j2=j1+1; j2 < n; j2++)
			{
				if((*pp)[j1].indexB==(*pp)[j2].indexB)
				{
					arr.append((*pp)[j2].prim);
					break;
				}
			}
		}

		BOSSP_INTERRUPT(i, N);
	}

	//delete duplicity constraints array
	m_gdp->deletePrimitives(arr, true);

	BOSSP_END;
}



void SConstraint::createMinimumDistanceConstraints(float toler_distance, float stiffness, float damping, float max_force, bool weariness, int iter, bool collision, float memory_mult)
{
	BOSSP_START;

	//init grid
	Grid g;
	if(!g.init(m_input->getGeo(), toler_distance, memory_mult))
		THROW_SOP("SConstraint: Not enought memory for allocation", 0);
	
	//performance problem warning!
	float nm = g.getMemoryIfNeed();
	if(nm > 0)
	{
		//char text[255];
		//sprintf(text, "SConstraint: performance problem - Set memory > %fMB!\n", ((float)nm)/1024/1024);
		//addWarning(text);
	}

	//generate constraints according minimum distance
	const int npoints = m_input->getNumPoints();
	for( int i = 0; i < npoints; i++ )
	{
		g.createMinimumDistanceConstraints(m_input, i, this, stiffness, damping, max_force, weariness, iter, collision);

		BOSSP_INTERRUPT(i, npoints);
	}


	BOSSP_END;
}





UT_Vector3 SConstraint::getPosA(GEO_Primitive* prim) const
{
	if(prim->getVertexCount() > 1)
		return m_gdp->getPos3(prim->getPointOffset(0));
	return UT_Vector3();
}
UT_Vector3 SConstraint::getPosB(GEO_Primitive* prim) const
{
	if(prim->getVertexCount() > 1)
		return m_gdp->getPos3(prim->getPointOffset(1));
	return UT_Vector3();
}
int SConstraint::getIndex(const GA_Offset &off) const
{
	return m_attr_bt_index.get(off);
}

int SConstraint::getIndexA(const GA_Offset &off) const
{
	return m_attr_bt_index_a.get(off);
}
int SConstraint::getIndexB(const GA_Offset &off) const
{
	return m_attr_bt_index_b.get(off);
}

int SConstraint::getIter(const GA_Offset &off) const
{
	return m_attr_bt_iter.get(off);
}
bool SConstraint::getWeariness(const GA_Offset &off) const
{
	return m_attr_bt_weariness.get(off) != 0;
}
float SConstraint::getStiffness(const GA_Offset &off) const
{
	return m_attr_bt_stiffness.get(off);
}
float SConstraint::getDamping(const GA_Offset &off) const
{
	return m_attr_bt_damping.get(off);
}
float SConstraint::getMaxforce(const GA_Offset &off) const
{
	return m_attr_bt_maxforce.get(off);
}
UT_Vector3 SConstraint::getLinlowerlimit(const GA_Offset &off) const
{
	return m_attr_bt_linlowerlimit.get(off);
}
UT_Vector3 SConstraint::getLinupperlimit(const GA_Offset &off) const
{
	return m_attr_bt_linupperlimit.get(off);
}
UT_Vector3 SConstraint::getAnglowerlimit(const GA_Offset &off) const
{
	return m_attr_bt_anglowerlimit.get(off);
}
UT_Vector3 SConstraint::getAngupperlimit(const GA_Offset &off) const
{
	return m_attr_bt_angupperlimit.get(off);
}
bool SConstraint::getUpdate(const GA_Offset &off) const
{
	return m_attr_bt_update.get(off) != 0;
}
bool SConstraint::getRecreate(const GA_Offset &off) const
{
	return m_attr_bt_recreate.get(off) != 0;
}
bool SConstraint::getEquilibrium(const GA_Offset &off) const
{
	return m_attr_bt_equilibrium.get(off) != 0;
}
bool SConstraint::getCollision(const GA_Offset &off) const
{
	return m_attr_bt_collision.get(off) != 0;
}



void SConstraint::setIndex(const GA_Offset &off, int i)
{
	m_attr_bt_index.set(off, i);
}

void SConstraint::setIndexA(const GA_Offset &off, int i)
{
	m_attr_bt_index_a.set(off, i);
}
void SConstraint::setIndexB(const GA_Offset &off, int i)
{
	m_attr_bt_index_b.set(off, i);
}
void SConstraint::setPosA(GEO_Primitive* prim, UT_Vector3 v)
{
	if(prim->getVertexCount() > 1)
		m_gdp->setPos3(prim->getPointOffset(0), v);
}

void SConstraint::setPosB(GEO_Primitive* prim, UT_Vector3 v)
{
	if(prim->getVertexCount() > 1)
		m_gdp->setPos3(prim->getPointOffset(1), v);
}

void SConstraint::setStiffness(const GA_Offset &off, float v)
{
	m_attr_bt_stiffness.set(off, v);
}
void SConstraint::setDamping(const GA_Offset &off, float v)
{
	m_attr_bt_damping.set(off, v);
}
void SConstraint::setMaxForce(const GA_Offset &off, float v)
{
	m_attr_bt_maxforce.set(off, v);
}
void SConstraint::setWeariness(const GA_Offset &off, bool v)
{
	m_attr_bt_weariness.set(off, v);
}
void SConstraint::setIter(const GA_Offset &off, int i)
{
	m_attr_bt_iter.set(off, i);
}
void SConstraint::setLL(const GA_Offset &off, UT_Vector3 v)
{
	m_attr_bt_linlowerlimit.set(off, v);
}
void SConstraint::setLU(const GA_Offset &off, UT_Vector3 v)
{
	m_attr_bt_linupperlimit.set(off, v);
}
void SConstraint::setAL(const GA_Offset &off, UT_Vector3 v)
{
	m_attr_bt_anglowerlimit.set(off, v);
}
void SConstraint::setAU(const GA_Offset &off, UT_Vector3 v)
{
	m_attr_bt_angupperlimit.set(off, v);
}
void SConstraint::setUpdate(const GA_Offset &off, bool v)
{
	m_attr_bt_update.set(off, v);
}
void SConstraint::setRecreate(const GA_Offset &off, bool v)
{
	m_attr_bt_recreate.set(off, v);
}

void SConstraint::setEquilibrium(const GA_Offset &off, bool v)
{
	m_attr_bt_equilibrium.set(off, v);
}

void SConstraint::setLinearLock(GEO_Primitive* prim)
{
	UT_Vector3 v = getPosB(prim) - getPosA(prim);
	setLL(prim->getMapOffset(), v);
	setLU(prim->getMapOffset(), v);
}
void SConstraint::setActualForce(const GA_Offset &off, float v)
{
	m_attr_bt_actforce.set(off, v);
}


void SConstraint::setCollision(const GA_Offset &off, bool v)
{
	m_attr_bt_collision.set(off, v);
}



