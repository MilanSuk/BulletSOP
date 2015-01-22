/**
BulletSOP v2.0
Copyright (c)2010-2013 Suk Milan. All Rights Reserved.
This software is provided 'as-is', without any express or implied warranty.
In no event will the authors be held liable for any damages arising from the use of this software.
This file may not be redistributed in whole or significant part.
This notice may not be removed or altered from any source distribution.
*/
#include "stdafx.h"
#include "SOP_ExtraCells.h"

#include "SHelper.h"


OP_ERROR SOP_ExtraCells::cookMySop(OP_Context &context)
{
	m_time = (float)context.getTime();
	m_boss = SOPBoss( UTgetInterrupt() );
    if (lockInputs(context) >= UT_ERROR_ABORT)
		return error();

	try
	{
		GU_Detail* input_1 = (GU_Detail *)inputGeo(0, context);
		GU_Detail* input_2 = (GU_Detail *)inputGeo(1, context);

		int num_prim_input_1 = (int)input_1->getPrimitiveMap().indexSize();

		//set output
		duplicateSource(0, context);
		gdp->merge(*input_2);

		//get attribute
		UT_String attr_name;
		ATTRIBUTE_NAME(attr_name);
		GA_RWHandleI attr = SHelper::findPrimitiveAttrI(gdp, attr_name);
		if(!attr.isValid())
		{
			char t[255];	sprintf(t, "2nd input hasn't '%s' attribute", attr_name.buffer());
			THROW_SOP(t, NULL);
		}

		GEO_Primitive* prim;

		//find max
		int max_index = 0;
		GA_FOR_ALL_PRIMITIVES(gdp, prim)
		{
			int index = attr(prim->getMapOffset());
			if(index > max_index)
				max_index = index;
		}

		//define array
		MyVec<OBJ> objs;
		objs.setExtraAlloc(EXTRA_ALLOC);
		objs.resize(max_index+1);	//realloc
		
		//fill structure
		int i = 0;
		GA_FOR_ALL_PRIMITIVES(gdp, prim)
		{
			int index = attr(prim->getMapOffset());

			if(i < num_prim_input_1)	objs[index].i1.push_back(prim);
			else						objs[index].i2.push_back(prim);

			i++;
		}



		//delete and group primitives
		int what_del = DELETE_EMPTY();

		UT_String group_name;
		GROUP_NAME(group_name);

		UT_Array<GEO_Primitive*> arr;	//GEO_PrimitivePtrArray arr;
		for(int i=0; i < objs.size(); i++)
		{
			if(	objs[i].i1.size()==0 ||
				objs[i].i2.size()==0)
			{
				bool del1 = true;
				bool del2 = true;

				if(what_del==INTPUT_1)	del2 = false;
				else
				if(what_del==INTPUT_2)	del1 = false;


				if(del1)
				{
					for(int j=0; j < objs[i].i1.size(); j++)
						arr.append(objs[i].i1[j]);
					objs[i].i1.clear();
				}

				if(del2)
				{
					for(int j=0; j < objs[i].i2.size(); j++)
						arr.append(objs[i].i2[j]);
					objs[i].i2.clear();
				}
			}

			if(	objs[i].i1.size() ||
				objs[i].i2.size())
			{
				char t[255];	sprintf(t, "%s%d", group_name.buffer(), i);
				GA_PrimitiveGroup* group = gdp->newPrimitiveGroup(t);

				for(int j=0; j < objs[i].i1.size(); j++)	group->add(objs[i].i1[j]);
				for(int j=0; j < objs[i].i2.size(); j++)	group->add(objs[i].i2[j]);
			}

		}

		gdp->deletePrimitives(arr, true);
	}
	catch(SOPException& e)
	{
		if(e.m_err)		if(strlen(e.m_err))		addError(SOP_MESSAGE, e.m_err);
		if(e.m_warning)	if(strlen(e.m_warning))	addWarning(SOP_MESSAGE, e.m_warning);
	}

	m_boss.exit();	//just to be sure
    unlockInputs();
    return error();
}



const char *SOP_ExtraCells::inputLabel(unsigned inum) const
{
    switch (inum)
    {
		case 0: return "Source";
    }
    return "Unknown source";
}


bool SOP_ExtraCells::updateParmsFlags()
{
	bool changed = SOP_Node::updateParmsFlags();

	return changed;
}



OP_Node *SOP_ExtraCells::myConstructor(OP_Network *net, const char *name, OP_Operator *op)
{
    return new SOP_ExtraCells(net, name, op);
}


SOP_ExtraCells::SOP_ExtraCells(OP_Network *net, const char *name, OP_Operator *op)
	: SOP_Node(net, name, op)
{
    getFullPath(m_operatorName);
}

SOP_ExtraCells::~SOP_ExtraCells()
{
}





static PRM_Name objs_names[] = {
	PRM_Name("attribute_name", "Attribute Name"),
	PRM_Name("delete_empty", "Delete Empty"),
	PRM_Name("group_name", "Group name"),
};


static PRM_Name deleteEmptyNames[] = {
	PRM_Name("0",	"Both"),
	PRM_Name("1",	"Input 1"),
	PRM_Name("2",	"Input 2"),
	PRM_Name(0)
};
static PRM_ChoiceList   deleteEmptyMenu((PRM_ChoiceListType)(PRM_CHOICELIST_SINGLE), &(deleteEmptyNames[0]) );

static PRM_Name g_name_info("tool_name", TOOL_NAME);
static PRM_Default	attribute_name(0, "cellpt");
static PRM_Default	group_name(0, "piece");


PRM_Template
SOP_ExtraCells::myTemplateList[] = {
	PRM_Template(PRM_HEADING ,1, &g_name_info, 0),

	PRM_Template(PRM_STRING,1, &objs_names[0], &attribute_name),
	PRM_Template(PRM_INT,	1, &objs_names[1], 0, &deleteEmptyMenu),
	PRM_Template(PRM_STRING,1, &objs_names[2], &group_name),

    PRM_Template()
};



