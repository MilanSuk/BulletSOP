/**
BulletSOP v2.0
Copyright (c)2010-2013 Suk Milan. All Rights Reserved.
This software is provided 'as-is', without any express or implied warranty.
In no event will the authors be held liable for any damages arising from the use of this software.
This file may not be redistributed in whole or significant part.
This notice may not be removed or altered from any source distribution.
*/
#include "stdafx.h"
#include "SEmit.h"
#include "Grid.h"

int SEmit::emit_id = 0;


SEmit::SEmit(void)
{
}

SEmit::~SEmit(void)
{
}


void SEmit::init(SOPBoss* boss, GU_Detail* gdp)
{
	setBasic(boss, gdp);

	m_attr_bt_emit_connect_dist = SHelper::addFloatPrimitiveAttr(m_gdp, "bt_emit_connect_dist", 1, 0.005f);
	m_attr_bt_emit_break_dist = SHelper::addFloatPrimitiveAttr(m_gdp, "bt_emit_break_dist", 1, 0.01f);
	m_attr_bt_emit_frame = SHelper::addIntPrimitiveAttr(m_gdp, "bt_emit_frame", 1, -1000000);

	m_gdp->addVariableName("bt_emit_connect_dist", "BCD");
	m_gdp->addVariableName("bt_emit_break_dist", "BBD");
	m_gdp->addVariableName("bt_emit_frame", "BF");
}

void SEmit::initFind(SOPBoss* boss, GU_Detail* gdp)
{
	setBasic(boss, gdp);

	m_attr_bt_emit_connect_dist = SHelper::findPrimitiveAttrF(m_gdp, "bt_emit_connect_dist");
	m_attr_bt_emit_break_dist = SHelper::findPrimitiveAttrF(m_gdp, "bt_emit_break_dist");
	m_attr_bt_emit_frame = SHelper::findPrimitiveAttrI(m_gdp, "bt_emit_frame");

	if(m_attr_bt_emit_connect_dist.isInvalid())	THROW_SOP("SEmit: No \"bt_emit_connect_dist\" primitive attribute", 0);
	if(m_attr_bt_emit_break_dist.isInvalid())	THROW_SOP("SEmit: No \"bt_emit_break_dist\" primitive attribute", 0);
	if(m_attr_bt_emit_frame.isInvalid())		THROW_SOP("SEmit: No \"bt_emit_frame\" primitive attribute", 0);
}



void SEmit::addToGroup(GA_PrimitiveGroup* group)
{
	GEO_Primitive* pr;
	GA_FOR_ALL_PRIMITIVES(m_gdp, pr)
		group->add(pr);
}



UT_Vector3 averageCenter(GEO_Primitive* prim)
{
	const int N = (int)prim->getVertexCount();
	UT_Vector3 sum(0,0,0);

	if(!N)
		return sum;

	for(int v=0; v < N; v++ )
		//sum += prim->getPos3(v);
		sum += getPrimPos3(prim, v);
	
	return sum / (float)N;
}

void SEmit::buildEmitGroups(float memory)
{
	BOSSP_START;

	float min_distance = FLT_MAX;
	GEO_Primitive* pr;
	GA_FOR_ALL_PRIMITIVES(m_gdp, pr)
	{
		float dist = getConnectDistance(pr->getMapOffset());
		if(dist > 0 && dist < min_distance)
			min_distance = dist;
	}


	GU_Detail gdp_points;
	GA_FOR_ALL_PRIMITIVES(m_gdp, pr)
	{
		GA_Offset ptoff = gdp_points.insertPointOffset();
		gdp_points.setPos3(ptoff, averageCenter(pr));
	}



	//init grid
	Grid g;
	if(!g.init(&gdp_points, min_distance, memory))
		THROW_SOP("SEmit: Not enought memory for allocation", 0);


	//performance problem warning!
	float nm = g.getMemoryIfNeed();
	if(nm > 0)
	{
		//char text[255];
		//sprintf(text, "SEmit: performance problem - Set memory > %fMB!\n", ((float)nm)/1024/1024);
		//addWarning(text);
	}

	//generate constraints according minimum distance
	const int npoints = (int)gdp_points.getNumPoints();
	for( int i = 0; i < npoints; i++ )
	{
		g.createMinimumDistancePairGroups(&gdp_points, i, this, emit_id);

		BOSSP_INTERRUPT(i, npoints);
	}

	BOSSP_END;
}


#define EMIT_GROUP_NAME "bt_emit"


int isEmitGroup(GA_PrimitiveGroup* gr)
{
	if(!gr->isEmpty())
	{
		UT_String name;
		name.harden(gr->getName());
		if(name.substitute(EMIT_GROUP_NAME, "") > 0 )	//work only with our groups
			if(name.isInteger() )
				return name.toInt();	//convert group number(X in "groupnameX") to integer
	}
	return -1;
}



void SEmit::setFrameFromGroups(int frame)
{
	BOSSP_START;

	GA_PrimitiveGroup* curr;
	GA_FOR_ALL_PRIMGROUPS(m_gdp, curr)	//iterate over all groups
	{
		if( isEmitGroup(curr) != -1 )
		{
			const int NUM_PRIM_GROUPS = (int)curr->entries();
			if(NUM_PRIM_GROUPS==1)	//only one
			{
				GA_GBPrimitiveIterator it(*(m_gdp), curr);
				GEO_Primitive* prim=GA_Detail::GB_MACRO_CAST(m_gdp, it.getPrimitive());

				if(getFrame(prim->getMapOffset()))
					setFrame(prim->getMapOffset(), frame);
			}
			else
			if(NUM_PRIM_GROUPS > 1)
			{
				GA_GBPrimitiveIterator it(*(m_gdp), curr);
				GEO_Primitive* prim1 = GA_Detail::GB_MACRO_CAST(m_gdp, it.getPrimitive());	++it;
				GEO_Primitive* prim2 = GA_Detail::GB_MACRO_CAST(m_gdp, it.getPrimitive());

				float lenCenter = (averageCenter(prim1) - averageCenter(prim2)).length();

				if(getFrame(prim1->getMapOffset()) <= -100000 && lenCenter > getBreakDistance(prim1->getMapOffset()))	setFrame(prim1->getMapOffset(), frame);
				if(getFrame(prim2->getMapOffset()) <= -100000 && lenCenter > getBreakDistance(prim2->getMapOffset()))	setFrame(prim2->getMapOffset(), frame);
			}
		}

		BOSSP_INTERRUPT_EMPTY;
	}

	BOSSP_END;
}



GU_Detail* SEmit::getGeo() const
{
	return m_gdp;
}


float SEmit::getConnectDistance(const GA_Offset &off) const
{
	return m_attr_bt_emit_connect_dist(off);
}
float SEmit::getBreakDistance(const GA_Offset &off) const
{
	return m_attr_bt_emit_break_dist(off);
}
int SEmit::getFrame(const GA_Offset &off) const
{
	return m_attr_bt_emit_frame(off);
}



void SEmit::setConnectDistance(const GA_Offset &off, float v)
{
	m_attr_bt_emit_connect_dist.set(off, v);
}

void SEmit::setBreakDistance(const GA_Offset &off, float v)
{
	m_attr_bt_emit_break_dist.set(off, v);
}
void SEmit::setFrame(const GA_Offset &off, int i)
{
	m_attr_bt_emit_frame.set(off, i);
}




void SEmit::setAllBreakDistance(float dist)
{
	//GEO_Primitive* pr;
	for (GA_Iterator it(m_gdp->getPrimitiveRange()); !it.atEnd(); ++it)	//GA_FOR_ALL_PRIMITIVES(m_gdp, pr) 
		setBreakDistance(*it, dist);
}


void SEmit::setAllConnectDistance(float dist)
{
	//GEO_Primitive* pr;
	for (GA_Iterator it(m_gdp->getPrimitiveRange()); !it.atEnd(); ++it)	//GA_FOR_ALL_PRIMITIVES(m_gdp, pr)
		setConnectDistance(*it, dist);
}
