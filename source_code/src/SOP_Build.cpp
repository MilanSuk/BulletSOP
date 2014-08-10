/**
BulletSOP v2.0
Copyright (c)2010-2013 Suk Milan. All Rights Reserved.
This software is provided 'as-is', without any express or implied warranty.
In no event will the authors be held liable for any damages arising from the use of this software.
This file may not be redistributed in whole or significant part.
This notice may not be removed or altered from any source distribution.
*/
#include "stdafx.h"
#include "SOP_Build.h"

#include "SObject.h"
#include "SConstraint.h"
#include "SForce.h"
#include "SShape.h"
#include "SEmit.h"



OP_ERROR SOP_Build::cookMySop(OP_Context &context)
{
	m_time = (float)context.getTime();
	m_boss = SOPBoss( UTgetInterrupt() );
    if(lockInputs(context) >= UT_ERROR_ABORT)
		return error();

	try
	{
		if(WORK_OBJS_CSTR_FORC()==EMIT)		duplicateSource(0, context);	//copy input to ouput
		else								gdp->clearAndDestroy();
		GU_Detail* input_1 = (GU_Detail *)inputGeo(0, context);

		//check error(s)
		if(	WORK_OBJS_CSTR_FORC()==RBDS ||
			WORK_OBJS_CSTR_FORC()==DIRECTORY ||
			WORK_OBJS_CSTR_FORC()==EMIT ||
			( WORK_OBJS_CSTR_FORC()==CONSTRAINTS && CSTR_STAT_GEN()==CONSTRAINTS_GEN ) )	//Object OR ConstraintsGen
		{
			if(!input_1)
			{
				THROW_SOP("1st Input is not connected", NULL);	//error
			}
			else
			{
				if(input_1->getPointMap().indexSize()==0)
				{
					SObject obj;	obj.init(&m_boss, gdp);	//create attributes
					THROW_SOP(0, 0);
				}
			}
		}

		//create group
		GA_PrimitiveGroup* group_pr = 0;
		GA_PointGroup* group_pt = 0;
		if(CREATE_GROUP())
		{
			UT_String name;
			GROUP_NAME(name);
			if(name.length())
			{
				if(	WORK_OBJS_CSTR_FORC()==CONSTRAINTS ||
					WORK_OBJS_CSTR_FORC()==EMIT)		group_pr = gdp->newPrimitiveGroup(name, false);	//Constraints, Emit
				else									group_pt = gdp->newPointGroup(name, false);		//RBDs, Forces
			}
			else
				addWarning(SOP_MESSAGE, "No group_name set");
		}

		//fill output
		switch(WORK_OBJS_CSTR_FORC())
		{
			case RBDS:			fillRBDs(input_1, group_pt);		break;
			case CONSTRAINTS:	fillConstraints(input_1, group_pr);	break;
			case FORCES:		fillForces(input_1, group_pt);		break;
			case DIRECTORY:		fillDirectory(input_1);				break;
			case EMIT:			fillEmit(input_1, group_pr);		break;
		}


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



void SOP_Build::fillRBDs(GU_Detail* input_1, GA_PointGroup* group_pt)
{
	SShape shape;
	shape.initFind(&m_boss, input_1, false);	//abstract data from Houdini

	SObject obj;
	obj.init(&m_boss, gdp, &shape);	//ouput

	//compute COG
	if(OBJS_HULL()!=TYPE_DEFORM)
	{
		switch(OBJS_COG())
		{
			case COG_BBOX:		obj.computeBoxCOG();		break;
			case COG_AVERAGE:	obj.computeAverageCOG();	break;
			case COG_VOLUME:	obj.computeVolumeCOG();		break;
		}
	}

	if(OBJS_HULL() != TYPE_DEFORM && OBJS_HULL() != TYPE_CONVEX)
		obj.computeCogRotation();

	obj.computeBBox();

	obj.setAllPadding(shape.getPadding());
	obj.setAllNotCompoundType(OBJS_HULL());


	if(OBJS_RECREATE())	obj.setAllRecreate(1);
	else
	if(OBJS_UPDATE())	obj.setAllUpdate(1);

	if(OBJS_STATIC())	obj.setAllMass(0);
	else
	if(OBJS_MASS_FROM_VOLUME())	obj.computeMassFromVolume(OBJS_DENSITY());

	if(OBJS_COPY_PRIM_ATTRS())	obj.copyPrimAttrs();

	obj.updateAllCompoundMass();

	obj.copyCOGtoPos();
	obj.copyCOGRotToRot();

	if(group_pt)
		obj.addToGroup(group_pt);	//put data to group
}




void SOP_Build::fillConstraints(GU_Detail* input_1, GA_PrimitiveGroup* group_pr)
{
	SShape* shapeptr = 0;
	SShape shape;
	if(input_1)
	{
		shape.initFind(&m_boss, input_1, false);	//abstract data from Houdini
		shapeptr = &shape;
	}

	SConstraint cstr;
	cstr.init(&m_boss, gdp, shapeptr);	//ouput


	if(CSTR_STAT_GEN()==CONSTRAINTS_STATIC)	//create static
	{
		for(int i=1; i < CSTR_STAT_NUM_TABS()+1; i++)
		{
			UT_Vector3 s = CSTR_STAT_POS_START(&i);
			UT_Vector3 e = CSTR_STAT_POS_END(&i);
			

			GEO_Primitive* prim = cstr.add();
			GA_Offset primoff = prim->getMapOffset();

			cstr.setIndexA(primoff, CSTR_STAT_BODY_START(&i));
			cstr.setIndexB(primoff, CSTR_STAT_BODY_END(&i));
			cstr.setPosA(prim, s);
			cstr.setPosB(prim, e);

			cstr.setStiffness(primoff, CSTR_STAT_STIFFNESS(&i));
			cstr.setDamping(primoff, CSTR_STAT_DAMPING(&i));
			cstr.setMaxForce(primoff, CSTR_STAT_MAX_FORCE(&i));
			cstr.setWeariness(primoff, CSTR_STAT_WEARINESS(&i)!=0);
			cstr.setIter(primoff, CSTR_STAT_ITER(&i));
			cstr.setCollision(primoff, CSTR_STAT_COLLISION(&i)!=0);
			
			if(CSTR_STAT_LOCK(&i))
			{
				UT_Vector3 lock = e-s;
				cstr.setLL(primoff, lock);
				cstr.setLU(primoff, lock);
			}
			else
			{
				cstr.setLL(primoff, CSTR_STAT_LL(&i));
				cstr.setLU(primoff, CSTR_STAT_LU(&i));
			}

			cstr.setAL(primoff, CSTR_STAT_AL(&i));
			cstr.setAU(primoff, CSTR_STAT_AU(&i));
		}
	}
	else
	if(CSTR_STAT_GEN()==CONSTRAINTS_GEN)	//generate constraints from geometry
	{
		cstr.createMinimumDistanceConstraints(CSTR_GEN_TOLER_DISTANCE(), CSTR_GEN_STIFFNESS(), CSTR_GEN_DAMPING(), CSTR_GEN_MAXFORCE(), CSTR_GEN_WEARINESS()!=0, CSTR_GEN_ITER(), CSTR_GEN_COLLISION()!=0, CSTR_GEN_MEMORY());
	}

	if(group_pr)
		cstr.addToGroup(group_pr);	//put data to group
}



void SOP_Build::fillForces(GU_Detail* input_1, GA_PointGroup* group_pt)
{
	SForce forc;
	forc.init(&m_boss, gdp);

	for(int i=1; i < FORC_NUM_TABS()+1; i++)
	{
		int obj_index = FORC_INDEX(&i);
		if(FORC_OBJ_RAD(&i)==1)	//radius
			obj_index = -1;

		GA_Offset ptoff = forc.add();	//add point

		//set point attributes
		forc.setPos(ptoff, FORC_POS(&i));
		forc.setType(ptoff, FORC_TYPE(&i));
		forc.setDir(ptoff, FORC_DIR(&i));
		forc.setScale(ptoff, FORC_SCALE(&i));

		forc.setIndex(ptoff, obj_index);
		forc.setRad(ptoff, FORC_RAD(&i));
		forc.setFade(ptoff, FORC_FADE(&i));
	}

	if(group_pt)
		forc.addToGroup(group_pt);	//put data to group
}


/**
This function creates ouput which includes directory(register), where every row includes line number of first match attribute value on input(input_1)
*/
void SOP_Build::fillDirectory(GU_Detail* input_1)
{
	UT_String name;
	DIR_NAME(name);

	GA_RWHandleI attr;
	if(DIR_CLASS()==0)	attr = SHelper::findPointAttrI(input_1, name);	//input_1->findPointAttribute(name);
	else				attr = SHelper::findPrimitiveAttrI(input_1, name);
	if(attr.isInvalid())
	{
		char t[255];	sprintf(t, "Cannot find attribute: %s", name.buffer());
		THROW_SOP(t, 0);
	}

	//create attribute for ouput
	GA_RWHandleI attr_bt_line = SHelper::addIntPrimitiveAttr(gdp, "bt_line", 1, -1);

	//get max. value from attr
	int max_index = -1;
	if(DIR_CLASS()==CLASS_POINT)
	{
		GA_Offset ptoff;
		GA_FOR_ALL_PTOFF(input_1, ptoff)
		{
			int index = absoluteValue(attr.get(ptoff));
			if(index > max_index)
				max_index = index;
		}
	}
	else
	if(DIR_CLASS()==CLASS_PRIMITVE)
	{
		GEO_Primitive* prim;
		GA_FOR_ALL_PRIMITIVES(input_1, prim)
		{
			int index = absoluteValue(attr.get(prim->getMapOffset()));
			if(index > max_index)
				max_index = index;
		}
	}

	//create "max. value" primitives
	max_index += 1;
	for(int i=0; i < max_index; i++)
		GU_PrimPoly::build(gdp, 0, GU_POLY_OPEN);

	//fill it
	if(DIR_CLASS()==CLASS_POINT)
	{
		int i=0;
		GA_Offset ptoff;
		GA_FOR_ALL_PTOFF(input_1, ptoff)
		{
			int index = absoluteValue(attr.get(ptoff));	//always positive!

			GA_Offset off = gdp->pointOffset(index);
			if(attr_bt_line.get(off)==-1)
				attr_bt_line.set(off, i);

			i++;
		}
	}
	else
	if(DIR_CLASS()==CLASS_PRIMITVE)
	{
		int i=0;
		GEO_Primitive* prim;
		GA_FOR_ALL_PRIMITIVES(input_1, prim)
		{
			int index = absoluteValue(attr.get(prim->getMapOffset()));	//always positive!

			GA_Offset off = gdp->pointOffset(index);
			if(attr_bt_line.get(off)==-1)
				attr_bt_line.set(off, i);

			i++;

		}
	}

}


void SOP_Build::fillEmit(GU_Detail* input_1, GA_PrimitiveGroup* group_pr)
{
	SEmit emit;

	if(EMIT_SET_DISTANCES())	emit.init(&m_boss, gdp);
	else						emit.initFind(&m_boss, gdp);

	if(EMIT_SET_DISTANCES())
	{
		emit.setAllBreakDistance( EMIT_BREAK_DISTANCE() );
		emit.setAllConnectDistance( EMIT_CONNECT_DISTANCE() );
	}

	emit.buildEmitGroups(500);

	if(group_pr)
		emit.addToGroup(group_pr);	//put data to group
}







const char *SOP_Build::inputLabel(unsigned inum) const
{
    switch (inum)
    {
		case 0: return "Source";
    }
    return "Unknown source";
}


bool SOP_Build::updateParmsFlags()
{
	bool changed = SOP_Node::updateParmsFlags();

	changed |= enableParm("group_name", CREATE_GROUP());

	changed |= enableParm("objs_mass_from_volume", !OBJS_STATIC());
	changed |= enableParm("objs_density", OBJS_MASS_FROM_VOLUME() && !OBJS_STATIC());

	changed |= enableParm("objs_update", !OBJS_RECREATE());


	for(int i=1; i < CSTR_STAT_NUM_TABS()+1; i++)
	{
		int state = !evalIntInst("cstr_stat_lock", &i, 0, m_time, 1);
		changed |= enableParmInst("cstr_stat_linlowerlimit", &i, state, 1);
		changed |= enableParmInst("cstr_stat_linupperlimit", &i, state, 1);
	}

	for(int i=1; i < FORC_NUM_TABS()+1; i++)
		changed |= enableParmInst("forc_dir", &i, FORC_TYPE(&i)!=MAGNET, 1);	//MAGNET

	changed |= enableParm("emit_break_distance", EMIT_SET_DISTANCES());
	changed |= enableParm("emit_connect_distance", EMIT_SET_DISTANCES());

	return changed;
}



OP_Node *SOP_Build::myConstructor(OP_Network *net, const char *name, OP_Operator *op)
{
    return new SOP_Build(net, name, op);
}


SOP_Build::SOP_Build(OP_Network *net, const char *name, OP_Operator *op)
	: SOP_Node(net, name, op)
{
    getFullPath(m_operatorName);
}

SOP_Build::~SOP_Build()
{
}





static PRM_Name objs_names[] = {
	PRM_Name("objs_hull", "Hull"),
	PRM_Name("objs_cog", "Cog"),

	PRM_Name("objs_sep0", "sep0"),

	PRM_Name("objs_static", "Static"),
	PRM_Name("objs_recreate", "Recreate every frame"),
	PRM_Name("objs_update", "Update every frame"),


	PRM_Name("objs_sep1", "sep1"),

	PRM_Name("objs_mass_from_volume", "Calculate mass from volume and density"),
	PRM_Name("objs_density",	"Density(kg/m3) - concrete:2100, wood:600 "),

	PRM_Name("objs_sep2", "sep2"),

	PRM_Name("objs_copy_prim_attrs", "Copy primitive attribute(s)"),
};

static PRM_Name objs_hullNames[] = {
	PRM_Name("0",	"Convex"),
	PRM_Name("1",	"Sphere"),
	PRM_Name("2",	"Box"),
	PRM_Name("3",	"Capsule"),
	PRM_Name("4",	"Cone"),
	PRM_Name("5",	"Cylinder"),
	PRM_Name("6",	"Deform"),
	PRM_Name("7",	"Compound"),
	PRM_Name(0)
};

static PRM_Name objs_cogNames[] = {
	PRM_Name("0",	"BBox"),
	PRM_Name("1",	"Average"),
	PRM_Name("2",	"Volume"),
	PRM_Name(0)
};

static PRM_ChoiceList   objs_hullMenu((PRM_ChoiceListType)(PRM_CHOICELIST_SINGLE), &(objs_hullNames[0]) );
static PRM_ChoiceList   objs_cogMenu((PRM_ChoiceListType)(PRM_CHOICELIST_SINGLE), &(objs_cogNames[0]) );
static PRM_Default	objs_density(2100);
static PRM_Default	objs_convex_test_distance(0.01f);










static PRM_Name cstr_names[] = {
	PRM_Name("cstr_constraints_tab", "constraints_tab"),

	//stat
	PRM_Name("cstr_stat_constraints_tab", "Constraints"),
	PRM_Name("cstr_stat_body_start",	"Index Start Object"),
	PRM_Name("cstr_stat_body_end",	"Index End Object"),
	PRM_Name("cstr_sep1", "sep1"),
	PRM_Name("cstr_stat_pos_start",	"Start World Pos"),
	PRM_Name("cstr_stat_pos_end",	"End World Pos"),
	PRM_Name("cstr_sep2", "sep1"),
	PRM_Name("cstr_stat_stiffness", "Stiffness"),
	PRM_Name("cstr_stat_damping",	"Damping"),
	PRM_Name("cstr_stat_maxforce",	"Maximum Force"),
	PRM_Name("cstr_sep3", "sep1"),
	PRM_Name("cstr_stat_lock",	"Linear Lock"),
	PRM_Name("cstr_stat_linlowerlimit",	"Linear Lower Limit"),
	PRM_Name("cstr_stat_linupperlimit",	"Linear Upper Limit"),
	PRM_Name("cstr_stat_anglowerlimit",	"Angular Lower Limit"),
	PRM_Name("cstr_stat_angupperlimit",	"Angular Upper Limit"),
	PRM_Name("cstr_sep4", "sep1"),
	PRM_Name("cstr_stat_iter",	"Iteration"),
	PRM_Name("cstr_stat_weariness",	"Weariness"),
	PRM_Name("cstr_stat_collision",	"Collision"),	//

	//gen
	PRM_Name("cstr_gen_toler_distance",	"Maximum Distance"),
	PRM_Name("cstr_sep5", "sep1"),
	PRM_Name("cstr_gen_stiffness",	"Stiffness"),
	PRM_Name("cstr_gen_damping",	"Damping"),
	PRM_Name("cstr_gen_maxforce",	"Maximum Force"),
	PRM_Name("cstr_gen_iter",	"Iteration"),
	PRM_Name("cstr_gen_weariness",	"Weariness"),
	PRM_Name("cstr_gen_collision",	"Collision"),	//
	PRM_Name("cstr_sep7", "sep1"),
	PRM_Name("cstr_gen_memory",	"Max Memory(MB)"),

};

static PRM_Default	cstr_body_start(0);
static PRM_Default	cstr_body_end(1);

static PRM_Default cstr_body_start_origin[] = {PRM_Default(0), PRM_Default(0), PRM_Default(0)};
static PRM_Default cstr_body_end_origin[] = {PRM_Default(0), PRM_Default(0), PRM_Default(-1)};

static PRM_Default	cstr_stiffness(1000);
static PRM_Default	cstr_damping(0.001);
static PRM_Default	cstr_maxforce(0);

static PRM_Default cstr_linlowerlimit[] = {PRM_Default(-1000), PRM_Default(-1000), PRM_Default(-1000)};
static PRM_Default cstr_linupperlimit[] = {PRM_Default( 1000), PRM_Default( 1000), PRM_Default( 1000)};

static PRM_Default cstr_anglowerlimit[] = {PRM_Default(0), PRM_Default(0), PRM_Default(0)};
static PRM_Default cstr_angupperlimit[] = {PRM_Default(0), PRM_Default(0), PRM_Default(0)};


static PRM_Default	cstr_toler_distance(0.05);
static PRM_Default	cstr_memory(500);
static PRM_Default	cstr_iter(-1);


static PRM_Default  cstr_switcherList[] = {
	PRM_Default(1, "Static"),
    PRM_Default(10, "Generating"),
};


PRM_Template myConstraints[] = {
	PRM_Template(PRM_INT,	1, &cstr_names[2], &cstr_body_start),
	PRM_Template(PRM_INT,	1, &cstr_names[3], &cstr_body_end),

	PRM_Template(PRM_SEPARATOR, 1, &cstr_names[4]),

	PRM_Template(PRM_XYZ_J,	3, &cstr_names[5], cstr_body_start_origin),
    PRM_Template(PRM_XYZ_J,	3, &cstr_names[6], cstr_body_end_origin),

	PRM_Template(PRM_SEPARATOR, 1, &cstr_names[7]),

	PRM_Template(PRM_FLT,	1, &cstr_names[8], &cstr_stiffness, 0, &g_range_zero_more),
	PRM_Template(PRM_FLT,	1, &cstr_names[9], &cstr_damping, 0, &g_range_zero_more),
	PRM_Template(PRM_FLT,	1, &cstr_names[10], &cstr_maxforce, 0, &g_range_zero_more),

	PRM_Template(PRM_SEPARATOR, 1, &cstr_names[11]),

	PRM_Template(PRM_TOGGLE,	1, &cstr_names[12], PRMoneDefaults),
	PRM_Template(PRM_XYZ_J,	3, &cstr_names[13], cstr_linlowerlimit),
    PRM_Template(PRM_XYZ_J,	3, &cstr_names[14], cstr_linupperlimit),
	PRM_Template(PRM_XYZ_J,	3, &cstr_names[15], cstr_anglowerlimit),
    PRM_Template(PRM_XYZ_J,	3, &cstr_names[16], cstr_angupperlimit),

	PRM_Template(PRM_SEPARATOR, 1, &cstr_names[17]),

	PRM_Template(PRM_INT,	1, &cstr_names[18], &cstr_iter, 0, &g_range_minus_one_more),
	PRM_Template(PRM_TOGGLE,	1, &cstr_names[19], 0),
	PRM_Template(PRM_TOGGLE,	1, &cstr_names[20], PRMoneDefaults),

    PRM_Template()
};







static PRM_Name forc_names[] = {
	PRM_Name("forc_forces_tab", "forces_tab"),

	PRM_Name("forc_type",	"Type"),
	PRM_Name("forc_pos",	"Position"),
	PRM_Name("forc_dir",	"Direction"),
	PRM_Name("forc_scale",	"Scale"),

	PRM_Name("forc_obj_rad", "forc_obj_rad"),
		//object
		PRM_Name("forc_index", "Object index"),
		//radius
		PRM_Name("forc_rad", "Radius"),
		PRM_Name("forc_fade", "Fade"),
};


static PRM_Name objs_forceNames[] = {
	PRM_Name("0",	"Full"),
	PRM_Name("1",	"Central"),
	PRM_Name("2",	"Torque"),
	PRM_Name("3",	"Magnet"),
	PRM_Name(0)
};
static PRM_ChoiceList   objs_forceMenu((PRM_ChoiceListType)(PRM_CHOICELIST_SINGLE), &(objs_forceNames[0]) );

static PRM_Default  forc_switcherList[] = {
	PRM_Default(1, "Object"),
	PRM_Default(2, "Radius"),
};

static PRM_Default forc_forceVec[] = {PRM_Default(0), PRM_Default(1), PRM_Default(0)};

PRM_Template myForces[] = {
	PRM_Template(PRM_INT,	1, &forc_names[1], 0, &objs_forceMenu),
	PRM_Template(PRM_XYZ_J,	3, &forc_names[2], 0),
	PRM_Template(PRM_XYZ_J,	3, &forc_names[3], forc_forceVec),
	PRM_Template(PRM_FLT,	1, &forc_names[4], PRMoneDefaults),

	PRM_Template(PRM_SWITCHER_EXCLUSIVE, 2, &forc_names[5], forc_switcherList),
		//object
		PRM_Template(PRM_INT,	1, &forc_names[6], 0),
		//radius
		PRM_Template(PRM_FLT,	1, &forc_names[7], PRMoneDefaults, 0, 0, 0, 0, 1, "Force is applied only on object in this radius"),
		PRM_Template(PRM_FLT,	1, &forc_names[8], PRMoneDefaults, 0, 0, 0, 0, 1, "Size of force = force_size*(radius^fade) => 0=constant, 1=linear"),

    PRM_Template()
};







static PRM_Name dir_names[] = {
	PRM_Name("dir_name", "Attribute Name"),
	PRM_Name("dir_class", "Class"),
};

static PRM_Name dir_classNames[] = {
	PRM_Name("0",	"Point"),
	PRM_Name("1",	"Primitive"),
	PRM_Name(0)
};

static PRM_ChoiceList   dir_classMenu((PRM_ChoiceListType)(PRM_CHOICELIST_SINGLE), &(dir_classNames[0]) );
static PRM_Default	dir_name(0, "bt_index");






static PRM_Name emit_names[] = {
	PRM_Name("emit_set_distances", "Set Distances"),
	PRM_Name("emit_connect_distance", "Connect Distance"),
	PRM_Name("emit_break_distance", "Break Distance"),
};

static PRM_Default emit_break_distance(0.01f);
static PRM_Default emit_connect_distance(0.005f);





static PRM_Default  switcherList[] = {
    PRM_Default(11, "RBDs"),
	PRM_Default(1, "Constraints"),
	PRM_Default(1, "Forces"),
	PRM_Default(2, "Directory"),
	PRM_Default(3, "Emit"),
};


static PRM_Name name_create_group("create_group", "Create Group");
static PRM_Name name_group_name("group_name", "Group Name");
static PRM_Name name_sep("sep0", "sep0");
static PRM_Name name_work_obj_constr_force("work_obj_constr_force", "work_obj_constr_force");


static PRM_Default group_name(0, "bullet_group1");
static PRM_Name g_name_info("tool_name", TOOL_NAME);

PRM_Template
SOP_Build::myTemplateList[] = {
	PRM_Template(PRM_HEADING ,1, &g_name_info, 0),

	PRM_Template(PRM_TOGGLE,1, &name_create_group, 0),
	PRM_Template(PRM_STRING,	1, &name_group_name, &group_name),

	PRM_Template(PRM_SEPARATOR, 1, &name_sep),

	PRM_Template(PRM_SWITCHER_EXCLUSIVE, 5, &name_work_obj_constr_force, switcherList),
		//Objects
		PRM_Template(PRM_INT,	1, &objs_names[0], 0, &objs_hullMenu),
		PRM_Template(PRM_INT,	1, &objs_names[1], 0, &objs_cogMenu),

		PRM_Template(PRM_SEPARATOR, 1, &objs_names[2]),

		PRM_Template(PRM_TOGGLE, 1, &objs_names[3], 0),
		PRM_Template(PRM_TOGGLE, 1, &objs_names[4], 0),
		PRM_Template(PRM_TOGGLE, 1, &objs_names[5], 0),

		PRM_Template(PRM_SEPARATOR, 1, &objs_names[6]),

		PRM_Template(PRM_TOGGLE,1, &objs_names[7], 0),
		PRM_Template(PRM_FLT,	1, &objs_names[8], &objs_density, 0, &g_range_zero_more),

		PRM_Template(PRM_SEPARATOR, 1, &objs_names[9]),

		PRM_Template(PRM_TOGGLE, 1, &objs_names[10], 0),

		//Constraints
		PRM_Template(PRM_SWITCHER_EXCLUSIVE, 2, &cstr_names[0], cstr_switcherList),
			PRM_Template(PRM_MULTITYPE_LIST, myConstraints, 19, &cstr_names[1], PRMoneDefaults),

			PRM_Template(PRM_FLT,	1, &cstr_names[21], &cstr_toler_distance, 0, &g_range_zero_min_more),
			PRM_Template(PRM_SEPARATOR, 1, &cstr_names[22]),
			PRM_Template(PRM_FLT,	1, &cstr_names[23], &cstr_stiffness, 0, &g_range_zero_more),
			PRM_Template(PRM_FLT,	1, &cstr_names[24], &cstr_damping, 0, &g_range_zero_more),
			PRM_Template(PRM_FLT,	1, &cstr_names[25], &cstr_maxforce, 0, &g_range_zero_more),
			PRM_Template(PRM_INT, 1, &cstr_names[26], &cstr_iter, 0, &g_range_minus_one_more),
			PRM_Template(PRM_TOGGLE,1, &cstr_names[27], 0),
			PRM_Template(PRM_TOGGLE,1, &cstr_names[28], PRMoneDefaults),

			PRM_Template(PRM_SEPARATOR, 1, &cstr_names[29]),
			PRM_Template(PRM_FLT,	1, &cstr_names[30], &cstr_memory, 0, &g_range_one_more),


		//Forces
		PRM_Template(PRM_MULTITYPE_LIST, myForces, 5, &forc_names[0], PRMoneDefaults),


		//Directory
		PRM_Template(PRM_STRING,1, &dir_names[0], &dir_name),
		PRM_Template(PRM_INT,	1, &dir_names[1], 0, &dir_classMenu),



		//Emit
		PRM_Template(PRM_TOGGLE, 1, &emit_names[0], PRMoneDefaults),
		PRM_Template(PRM_FLT,	1, &emit_names[1], &emit_connect_distance),
		PRM_Template(PRM_FLT,	1, &emit_names[2], &emit_break_distance),



    PRM_Template()
};



