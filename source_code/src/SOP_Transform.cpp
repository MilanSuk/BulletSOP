/**
BulletSOP v2.0
Copyright (c)2010-2013 Suk Milan. All Rights Reserved.
This software is provided 'as-is', without any express or implied warranty.
In no event will the authors be held liable for any damages arising from the use of this software.
This file may not be redistributed in whole or significant part.
This notice may not be removed or altered from any source distribution.
*/
#include "stdafx.h"
#include "SOP_Transform.h"

#include "SObject.h"
#include "SShape.h"
#include "SEmit.h"



OP_ERROR SOP_Transform::cookMySop(OP_Context &context)
{
	m_time = context.getTime();
 	m_boss = SOPBoss( UTgetInterrupt() );
    if(lockInputs(context) >= UT_ERROR_ABORT)
		return error();

	try
	{
		m_objs.clear();

		const int act_frame = (int)OPgetDirector()->getChannelManager()->getSample(m_time);
		m_input_proper = (GU_Detail*)inputGeo(0, context);
		m_input_geo = (GU_Detail*)inputGeo(1, context);

		duplicateSource(1, context);	//copy 2nd to ouput(gdp)

		if(m_input_proper->getPointMap().indexSize()==0)
		{
			if(DELETE_GEO())
				gdp->clearAndDestroy();

			THROW_SOP(NULL, "No data on 1st input");
		}

		if(gdp->getPointMap().indexSize()==0)
		{
			THROW_SOP(NULL, "No data on 2nd input");
		}

		computeTransform();	//fill m_objs vector with properties data

		SShape shape;
		shape.initFind(&m_boss, gdp, false);

		switch(WORK_OUTPUT())	//extract geometry according bt_index
		{
		case OUT_PREFER_HIGH:
			deletePreferHigh(&shape);
			break;
		case OUT_HIGH:
			shape.deleteHigh();
			break;
		case OUT_LOW:
			shape.deleteLow();
			break;
		}

		if(DELETE_SUBCOMPOUND())
			shape.deleteSubCompund();

		if(DELETE_GEO())
			deleteUnused(&shape);

		transformGeo(&shape);

		if(DELETE_INDEX_ATTRS())
			deleteIndexAttributes();

		if(UPDATE_EMIT_FRAME())
			updateEmitFrame(act_frame);

		if(EXTRACT_EMIT_POLY())
			extractEmitPoly(act_frame);
	}
	catch(SOPException& e)
	{
		if(e.m_err)		if(strlen(e.m_err))		addError(SOP_MESSAGE, e.m_err);
		if(e.m_warning)	if(strlen(e.m_warning))	addWarning(SOP_MESSAGE, e.m_warning);
	}

	m_objs.clear();

	m_boss.exit();	//just to be sure
    unlockInputs();
	return error();
}

void SOP_Transform::deleteIndexAttributes()
{
	gdp->destroyAttribute(GA_ATTRIB_POINT, "bt_index");
	gdp->destroyAttribute(GA_ATTRIB_PRIMITIVE, "bt_index");
	gdp->destroyAttribute(GA_ATTRIB_POINT, "bt_cindex");
	gdp->destroyAttribute(GA_ATTRIB_PRIMITIVE, "bt_cindex");
}

void SOP_Transform::updateEmitFrame(int act_frame)
{
	SEmit emit;
	emit.initFind(&m_boss, gdp);
	emit.setFrameFromGroups(act_frame);

	//reset transform(copy original P)
	GA_Offset ptoff;
	GA_FOR_ALL_PTOFF(gdp, ptoff)
		gdp->setPos3(ptoff, m_input_geo->getPos3(ptoff));
}

void SOP_Transform::extractEmitPoly(int act_frame)
{
	BOSS_START;
	SEmit emit;
	emit.initFind(&m_boss, gdp);

	const int age = EXTRACT_EMIT_POLY_MAX_AGE();

	GEO_PrimitivePtrArray delArray;
	GEO_Primitive* prim;
	GA_FOR_ALL_PRIMITIVES(gdp, prim)
	{
		int frame = emit.getFrame(prim->getMapOffset());

		if(frame < -100000 || act_frame < frame || act_frame > frame+age)
			delArray.append(prim);	//out of range indexes

		BOSS_INTERRUPT_EMPTY;
	}
	gdp->deletePrimitives(delArray, true);	//real deleting
	BOSS_END;
}



void SOP_Transform::computeTransform()
{
	BOSS_START;

	//pre-alloc transform for every object
	const int num_objects = (int)m_input_proper->getPointMap().indexSize();
	m_objs.reserve(num_objects+EXTRA_ALLOC);

	const bool add_velocity = ADD_POINT_VELOCITY()!=0;	

	SObject obj;
	obj.initFind_TransformMinimum(&m_boss, m_input_proper, add_velocity);

	int i = 0;
	GA_Offset ptoff;
	GA_FOR_ALL_PTOFF(m_input_proper, ptoff)
	{
		//get property attributes
		int index = obj.getIndex(ptoff);
		int cindex = obj.getCIndex(ptoff);

		m_objs.resizeAdd(index+1);	//if "bt_index" doesnt start with zero or is not order(eg.: 0,1,2,100,101,102) we need to resize
		m_objs[index].use = true;

		//save data
		m_objs[index].use = true;
		m_objs[index].high = false;
		m_objs[index].mat = obj.getTranformationMatrix(ptoff);	//compute transform
		m_objs[index].cog = obj.getCOG(ptoff);
		m_objs[index].subCompound = cindex > -1;
		if(add_velocity)
		{
			m_objs[index].v = obj.getV(ptoff);
			m_objs[index].av = obj.getAV(ptoff);
		}

		BOSS_INTERRUPT(i, num_objects);
		i++;
	}

	BOSS_END;
}



UT_Vector3 SOP_Transform::getPointVelocity(UT_Vector3 rel, OBJ &obj)
{
	UT_Matrix3 matRot;	obj.mat.extractRotate(matRot);
	UT_Vector3 relAV = obj.av;	relAV.cross(rel);
	UT_Vector3 absAV = relAV * matRot;
	return obj.v + absAV;
}




void SOP_Transform::transformGeo(SShape* shape)
{
	BOSS_START;

	const bool add_velocity = ADD_POINT_VELOCITY()!=0;	
	
	GA_RWHandleV3 attr_v;
	if(add_velocity)
		attr_v = GA_RWHandleV3( gdp->addVelocityAttribute(GA_ATTRIB_POINT) );


	int num_warnings = 0;
	const int num_points = (int)gdp->getPointMap().indexSize();

	int i = 0;
	GA_Offset ptoff;
	GA_FOR_ALL_PTOFF(gdp, ptoff)
	{
		int index = shape->getPointIndex(ptoff);
		index = absoluteValue(index);
		
		if(index >= m_objs.size() || !m_objs[index].use)
		{
			if(num_warnings < 10)
			{
				char t[255];	sprintf(t, "Point[%d] without obj properties[%d]", i, index);
				addWarning(SOP_MESSAGE, t);
			}
			num_warnings++;
			continue;
		}

		UT_Vector3 pos = gdp->getPos3(ptoff);
		if(add_velocity)
		{
			UT_Vector3 rel = pos - m_objs[index].cog;
			attr_v.set(ptoff, getPointVelocity(rel, m_objs[index]) );	//compute and set velocity of point on obj
		}

		pos -= m_objs[index].cog;	//move obj to center
		pos *= m_objs[index].mat;	//apply rotation and translation
		gdp->setPos3(ptoff, pos);
	
		BOSS_INTERRUPT(i, num_points);
		i++;
	}

	BOSS_END;
}




void SOP_Transform::deleteUnused(SShape* shape)
{
	BOSS_START;

	GEO_PrimitivePtrArray delArray;
	GEO_Primitive* prim;
	GA_FOR_ALL_PRIMITIVES(gdp, prim)
	{
		int index = shape->getPrimitiveIndex(prim->getMapOffset());
		index = absoluteValue(index);

		//delete primitive with index, which doesnt have properties
		if(index < 0 || index >= m_objs.size())		delArray.append(prim);	//out of range indexes
		else
		if(!m_objs[index].use)	delArray.append(prim);

		BOSS_INTERRUPT_EMPTY;
	}
	gdp->deletePrimitives(delArray, true);	//real deleting

	BOSS_END;
}






void SOP_Transform::deletePreferHigh(SShape* shape)
{
	BOSS_START;

	GEO_Primitive* prim;
	//mark properties which have high poly geo
	GA_FOR_ALL_PRIMITIVES(gdp, prim)
	{
		int index = shape->getPrimitiveIndex(prim->getMapOffset());
		if(index < 0)
		{
			index = absoluteValue(index);
			if(index > 0 && index < m_objs.size())	
				m_objs[index].high = true;
		}

		BOSS_INTERRUPT_EMPTY;
	}

	//deleting primitives
	GEO_PrimitivePtrArray delArray;
	GA_FOR_ALL_PRIMITIVES(gdp, prim)
	{
		int index = shape->getPrimitiveIndex(prim->getMapOffset());
		if(index > 0 && index < m_objs.size())	//only for LOW(no abs())
		{
			if(m_objs[index].high)
				delArray.append(prim);
		}

		BOSS_INTERRUPT_EMPTY;
	}
	gdp->deletePrimitives(delArray, true);	//real deleting

	//delete points
	MyVec<GA_Offset> delPoints;
	delPoints.setExtraAlloc(EXTRA_ALLOC);

	GA_Offset ptoff;
	GA_FOR_ALL_PTOFF(gdp, ptoff)
	{
		int index = shape->getPointIndex(ptoff);
		if(index > 0 && index < m_objs.size())	//only for LOW(no abs())
		{
			if(m_objs[index].high)
				delPoints.push_back(ptoff);
		}

		BOSS_INTERRUPT_EMPTY;
	}
	for(int i=0; i < delPoints.size(); i++)
		gdp->destroyPointOffset(delPoints[i]);

	BOSS_END;
}



const char *SOP_Transform::inputLabel(unsigned inum) const
{
    switch (inum)
    {
		case 0: return "Physics Properties";
		case 1:	return "Shapes(Geometry)";
    }
    return "Unknown source"; 
}

bool SOP_Transform::updateParmsFlags()
{
	bool changed = SOP_Node::updateParmsFlags();

	changed |= enableParm("extract_emit_poly_max_age", EXTRACT_EMIT_POLY());

	return changed;
}

OP_Node *SOP_Transform::myConstructor(OP_Network *net, const char *name, OP_Operator *op)
{
    return new SOP_Transform(net, name, op);
}
SOP_Transform::SOP_Transform(OP_Network *net, const char *name, OP_Operator *op)
	: SOP_Node(net, name, op)
{
	getFullPath(m_operatorName);
	m_objs.setExtraAlloc(EXTRA_ALLOC);
}
SOP_Transform::~SOP_Transform()
{
}



static PRM_Name names[] = {

	PRM_Name("work_output", "Output"),
	PRM_Name("sep0", "sep0"),
	PRM_Name("delete_geo", "Delete Geometry without Properties"),
	PRM_Name("delete_subcompound", "Delete SubCompound Objects"),
	PRM_Name("delete_index_attrs", "Delete Index Attributes"),
	PRM_Name("sep1", "sep1"),
	PRM_Name("add_point_velocity", "Add Point Velocity"),
	PRM_Name("sep2", "sep2"),
	PRM_Name("update_emit_frame", "Update Emit Frame(No Transform)"),
	PRM_Name("extract_emit_poly", "Extract Emit Poly"),
	PRM_Name("extract_emit_poly_max_age", "Max Frame Age"),
};

static PRM_Name outputNames[] = {
	PRM_Name("0",	"Prefer Original"),
	PRM_Name("1",	"Original"),
	PRM_Name("2",	"Collision"),
	PRM_Name("3",	"Both"),
	PRM_Name(0)
};
static PRM_ChoiceList   ouputMenu((PRM_ChoiceListType)(PRM_CHOICELIST_SINGLE), &(outputNames[0]) );


static PRM_Name g_name_info("tool_name", TOOL_NAME);

static PRM_Default	max_age(10);

PRM_Template SOP_Transform::myTemplateList[] = {
	PRM_Template(PRM_HEADING ,1, &g_name_info, 0),
	PRM_Template(PRM_INT,	1, &names[0], 0, &ouputMenu),
	PRM_Template(PRM_SEPARATOR, 1, &names[1]),
    PRM_Template(PRM_TOGGLE,1, &names[2], 0),
	PRM_Template(PRM_TOGGLE,1, &names[3], 0),
	PRM_Template(PRM_TOGGLE,1, &names[4], PRMoneDefaults),
	PRM_Template(PRM_SEPARATOR, 1, &names[5]),
	PRM_Template(PRM_TOGGLE,1, &names[6], 0),
	PRM_Template(PRM_SEPARATOR, 1, &names[7]),
    PRM_Template(PRM_TOGGLE,1, &names[8], 0),
	PRM_Template(PRM_TOGGLE,1, &names[9], 0),
	PRM_Template(PRM_INT_J, 1, &names[10], &max_age),
    PRM_Template()
};
