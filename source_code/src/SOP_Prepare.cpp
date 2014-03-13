/**
BulletSOP v2.0
Copyright (c)2010-2013 Suk Milan. All Rights Reserved.
This software is provided 'as-is', without any express or implied warranty.
In no event will the authors be held liable for any damages arising from the use of this software.
This file may not be redistributed in whole or significant part.
This notice may not be removed or altered from any source distribution.
*/
#include "stdafx.h"
#include "SOP_Prepare.h"
#include "SShape.h"


OP_ERROR SOP_Prepare::cookMySop(OP_Context &context)
{
	m_time = (float)context.getTime();
	m_boss = SOPBoss( UTgetInterrupt() );
    if (lockInputs(context) >= UT_ERROR_ABORT)
		return error();

	try
	{
		m_input1 = (GU_Detail *)inputGeo(0, context);
		m_input2 = (GU_Detail *)inputGeo(1, context);
		duplicateSource(0, context);	//copy 1st input to output. Note: very slow part of this node!!!

		bool add_cindex = (WORK_COMP_DECOMP()==TAB_COMP && COMPOUND_NUM()>0) || WORK_COMP_DECOMP()==TAB_DECOMP;	//Do We need bt_cindex attribute?

		SShape myShape;
		myShape.init(&m_boss, gdp, add_cindex);	//create structure for output

		int start_i = 1;
		if(getInput(1))	//is 2nd input connected
		{
			if(m_input2->getPointMap().indexSize() > 0)
			{
				SShape myShapeInput2;
				myShapeInput2.initFind(&m_boss, m_input2);
					start_i = myShapeInput2.getMaxIndex() + 1;
			}
		}
		else
			start_i = INDEX_START();


		//fill bt_index ...
		if(WORKGROUPORCOPY()==TAB_GROUP)	//according groups
		{
			UT_String group_name;
			GROUP_NAME(group_name);
			myShape.fillAtrributeGroup( start_i, group_name, !DELETEALLGROUPS() && DELETEGROUPS() );

			if( DELETEALLGROUPS() )
			{
				gdp->primitiveGroups().clear();
				gdp->pointGroups().clear();
				gdp->edgeGroups().clear();
			}
		}
		else
		if(WORKGROUPORCOPY()==TAB_COPIES)	//according number of copies
		{
			myShape.fillAtrributeCopyPoints(start_i, COPYNUMPOINTS());	//points
			myShape.fillAtrributeCopyPrimitives(start_i, COPYNUMPRIMS());	//primitives
		}


		//fill bt_cindex
		if(WORK_COMP_DECOMP()==TAB_COMP && COMPOUND_NUM()>0)	//according compound number
		{
			if(COMPOUND_FIRST())	myShape.setCompoundsFirst(COMPOUND_NUM(), start_i);
			else					myShape.setCompounds(COMPOUND_NUM(), start_i);
		}


		//do we need original("high_poly") geometry
		GU_Detail save_origGeo;
		if(ORIGINAL_GEO())
		{
			SShape myShapeHighPoly;
			save_origGeo.copy(*gdp);
			myShapeHighPoly.initFind(&m_boss, &save_origGeo);
			myShapeHighPoly.convertToHighPoly();
		}


		if(WORK_COMP_DECOMP()==TAB_COMP)	//move("implode") geometry inside according PADDING()
		{
			myShape.shiftGeo(m_input1, PADDING(), NORMAL_TYPE());
		}
		else
		if(WORK_COMP_DECOMP()==TAB_DECOMP)
		{
			myShape.createConvexGroup(DECOMP_CONVEX_PRECISE()!=0, DECOMP_CONVEX_TOLER());	//divite objects to convex or concave groups

			if(DECOMP_WORK_TETRA_VOXEL()==TAB_TETRA)
			{
				myShape.shiftGeo(m_input1, PADDING(), NORMAL_TYPE());	//move("implode") geometry inside according PADDING()
				myShape.decompositionTetra(DECOMP_TETRA_NUM_RAY_STEPS(), DECOMP_COMPOUND_GEO()!=0);	//create tetra from every polygon
			}
			else
			if(DECOMP_WORK_TETRA_VOXEL()==TAB_VOXEL)
			{
				myShape.decompositionVoxel(DECOMP_VOXEL_CELL_SIZE(), PADDING(), DECOMP_COMPOUND_GEO()!=0);	//put 3d grid of points to object
			}

		}

		if(ORIGINAL_GEO())
			gdp->copy(save_origGeo, GEO_COPY_ADD);	//adds original geometry to ouput

		myShape.setPadding(PADDING()+PADDING_EXTRA());	//set padding value to detail attribute
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



const char *SOP_Prepare::inputLabel(unsigned inum) const
{
    switch (inum)
    {
		case 0: return "Source";
		case 1: return "Previous Prepare data";
    }
    return "Unknown source";
}


bool SOP_Prepare::updateParmsFlags()
{
	bool changed = SOP_Node::updateParmsFlags();

	changed |= enableParm("group_delete", !DELETEALLGROUPS());
	changed |= enableParm("index_start", getInput(1)==0);
	changed |= enableParm("decomp_convex_toler", DECOMP_CONVEX_PRECISE());

	return changed;
}


OP_Node *SOP_Prepare::myConstructor(OP_Network *net, const char *name, OP_Operator *op)
{
    return new SOP_Prepare(net, name, op);
}

SOP_Prepare::SOP_Prepare(OP_Network *net, const char *name, OP_Operator *op)
	: SOP_Node(net, name, op)
{
    getFullPath(m_operatorName);
}

SOP_Prepare::~SOP_Prepare()
{
}






static PRM_Name names[] = {
	PRM_Name("index_start", "index_start"),
	PRM_Name("original_geo", "Original Geometry"),
	PRM_Name("padding", "Padding"),
	PRM_Name("normal_type", "Normal"),
	PRM_Name("padding_extra", "Extra Padding"),

	PRM_Name("work_group_or_copy", "work_group_or_copy"),
		//Groups
		PRM_Name("group_name", "Group Prefix"),
		PRM_Name("group_delete_all", "Delete All Groups"),
		PRM_Name("group_delete", "Delete Only Prefix Groups"),
		//Copies
		PRM_Name("copy_npoints", "copy_npoints"),
		PRM_Name("copy_nprims", "copy_nprims"),


	PRM_Name("work_comp_decomp", "work_comp_decomp"),
		//Compound
		PRM_Name("compound_num", "Compound objects"),
		PRM_Name("compound_first", "Compound First"),

		//Decomposition
		PRM_Name("decomp_convex_precise", "Higher Convex Precision Test"),
		PRM_Name("decomp_convex_toler", "Convex Toler"),
		PRM_Name("decomp_work_tetra_voxel", "work_comp_decomp"),
			//tetra
			PRM_Name("decomp_tetra_num_ray_steps", "Precision"),
			//voxel
			PRM_Name("decomp_voxel_cell_size", "Cell Size"),
			
		PRM_Name("decomp_compound_geo", "Compound Geo"),

};



static PRM_Default  switcherList[] = {
    PRM_Default(3, "Groups"),
	PRM_Default(2, "Copies"),
};


static PRM_Default  switcherList2[] = {
    PRM_Default(2, "Compound"),
	PRM_Default(4, "Decomposition"),
};


static PRM_Default  switcherList3[] = {
    PRM_Default(1, "Tetra"),
	PRM_Default(1, "Voxel"),
};



static PRM_Name normalTypesNames[] = {
	PRM_Name("0",	"EdgeAngles"),
	PRM_Name("1",	"Planes"),
	PRM_Name(0)
};
static PRM_ChoiceList   normalTypesMenu((PRM_ChoiceListType)(PRM_CHOICELIST_SINGLE), &(normalTypesNames[0]) );



static PRM_Default	divSize(0.1f);
static PRM_Default	padding(0.01f);
static PRM_Default	extra_padding(0);
static PRM_Default	two(2);
static PRM_Default	raySteps(50);
static PRM_Default	cellSize(0.1f);
static PRM_Default	group_name(0, "piece");
static PRM_Name g_name_info("tool_name", TOOL_NAME);
static PRM_Default	decomp_convex_toler(0.01f);


PRM_Template SOP_Prepare::myTemplateList[] = {
	PRM_Template(PRM_HEADING ,1, &g_name_info, 0),
	PRM_Template(PRM_INT,	1, &names[0], PRMoneDefaults, 0, &g_range_one_more),
	PRM_Template(PRM_TOGGLE,1, &names[1], 0),
	PRM_Template(PRM_FLT|PRM_TYPE_JOIN_NEXT,	1, &names[2], &padding),
	PRM_Template(PRM_INT,	1, &names[3], 0, &normalTypesMenu),
	PRM_Template(PRM_FLT,	1, &names[4], &extra_padding),

	PRM_Template(PRM_SWITCHER_EXCLUSIVE, 2, &names[5], switcherList),
		//groups
		PRM_Template(PRM_STRING ,1, &names[6], &group_name),
		PRM_Template(PRM_TOGGLE ,1, &names[7], 0),
		PRM_Template(PRM_TOGGLE, 1, &names[8], PRMoneDefaults),
		//copies
		PRM_Template(PRM_INT,	1, &names[9], PRMoneDefaults, 0, &g_range_one_more),
		PRM_Template(PRM_INT,	1, &names[10], PRMoneDefaults, 0, &g_range_one_more),


	PRM_Template(PRM_SWITCHER_EXCLUSIVE, 2, &names[11], switcherList2),
		//Compound
		PRM_Template(PRM_INT,	1, &names[12], 0),
		PRM_Template(PRM_TOGGLE,1, &names[13], 0),
		//Decomposition
		PRM_Template(PRM_TOGGLE,1, &names[14], 0),
		PRM_Template(PRM_FLT,	1, &names[15], &decomp_convex_toler, 0, &g_range_zero_more),
		PRM_Template(PRM_SWITCHER_EXCLUSIVE, 2, &names[16], switcherList3),
			//tetra
			PRM_Template(PRM_INT,	1, &names[17], &raySteps),
			//voxel
			PRM_Template(PRM_FLT,	1, &names[18], &cellSize),

		PRM_Template(PRM_TOGGLE,1, &names[19], 0),

    PRM_Template()
};


