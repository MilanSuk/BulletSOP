/**
BulletSOP v2.0
Copyright (c)2010-2013 Suk Milan. All Rights Reserved.
This software is provided 'as-is', without any express or implied warranty.
In no event will the authors be held liable for any damages arising from the use of this software.
This file may not be redistributed in whole or significant part.
This notice may not be removed or altered from any source distribution.
*/
#include "stdafx.h"
#include "SOP_Modify.h"

#include "SObject.h"
#include "SConstraint.h"
#include "SForce.h"
#include "SImpact.h"
#include "SEmit.h"

OP_ERROR SOP_Modify::cookMySop(OP_Context &context)
{
	m_time = (float)context.getTime();
	m_currframe = (int)OPgetDirector()->getChannelManager()->getSample(m_time);
	m_boss = SOPBoss( UTgetInterrupt() );

    if (lockInputs(context) >= UT_ERROR_ABORT)
		return error();

	try
	{
		duplicateSource(0, context);	//copy input to ouput

		if(gdp->getPointMap().indexSize()==0)
			THROW_SOP(0, 0);

		//get group array
		GA_PointGroup* myGroupPt = 0;
		GA_PrimitiveGroup* myGroupPr = 0;
		getGroup(myGroupPt, myGroupPr);	//loads and check groups


		//modify data
		int work_obj_cstr_forc = WORK_OBJS_CSTR_FORC();
		if(work_obj_cstr_forc==TAB_RBDS)		changeRBDs(myGroupPt);
		else
		if(work_obj_cstr_forc==TAB_CONSTRAINTS)	changeConstraints(myGroupPr);
		else
		if(work_obj_cstr_forc==TAB_FORCES)		changeForces(myGroupPt);
		else
		if(work_obj_cstr_forc==TAB_IMPACTS)
		{
			if(IMP_DEL_DUPLICITY())
			{
				SImpact imp;
				imp.initFind(&m_boss, gdp);
				imp.deleteDuplicity();
			}
		}
		else
		if(work_obj_cstr_forc==TAB_EMITS)		changeEmits(myGroupPr);
	}
	catch(SOPException& e)
	{
		if(e.m_err)		if(strlen(e.m_err))		addError(SOP_MESSAGE, e.m_err);
		if(e.m_warning)	if(strlen(e.m_warning))	addWarning(SOP_MESSAGE, e.m_warning);
	}

	m_boss.exit();	//just to be sure
    unlockInputs();
    resetLocalVarRefs();
    return error();
}




	
void SOP_Modify::getGroup(GA_PointGroup* &myGroupPt, GA_PrimitiveGroup* &myGroupPr)
{
	int work_obj_cstr_forc = WORK_OBJS_CSTR_FORC();

	//load names
	UT_String gpr_name1, gpr_name2;
	GROUP_NAME1(gpr_name1);
	GROUP_NAME2(gpr_name2);

	//check If point group exist
	if(gpr_name1.length() && (work_obj_cstr_forc==TAB_RBDS || work_obj_cstr_forc==TAB_FORCES))
	{
		myGroupPt = (GA_PointGroup*)parsePointGroups(gpr_name1, gdp);
		if(!myGroupPt)
			THROW_SOP("Unknow Point Group", 0);
	}
	//check If primitive group exist
	if(gpr_name2.length() && (work_obj_cstr_forc==TAB_CONSTRAINTS || work_obj_cstr_forc==TAB_EMITS))
	{
		myGroupPr = (GA_PrimitiveGroup*)parsePrimitiveGroups(gpr_name2, gdp);
		if(!myGroupPr)
			THROW_SOP("Unknow Primitive Group", 0);
	}
}


void SOP_Modify::changeRBDs(GA_PointGroup* myGroupPt)
{
	BOSS_START;

	SObject obj;
	obj.init(&m_boss, gdp);

	setLocalVars(true);

	int N;
	if(myGroupPt)	N = (int)myGroupPt->entries();
	else			N = (int)gdp->getPointMap().indexSize();

	int i = 0;
	GA_Offset ptoff;
	GA_FOR_ALL_GROUP_PTOFF(gdp, myGroupPt, ptoff)
	{
		myCurPtOff[0] = ptoff;	//map local variable(s)

		//change data
		obj.setPos(ptoff, OBJS_T());
		obj.setCOG(ptoff, OBJS_COG());
		obj.setCOGRotation(ptoff, OBJS_COG_R());
		obj.setBBox(ptoff, OBJS_BOXSIZE());
		obj.setIndex(ptoff, OBJS_INDEX());
		obj.setCIndex(ptoff, OBJS_CINDEX());
		obj.setType(ptoff, OBJS_TYPE());
		obj.setMass(ptoff, OBJS_MASS());
		obj.setRestitution(ptoff, OBJS_RESTITUTION());
		obj.setFriction(ptoff, OBJS_FRICTION());
		obj.setPadding(ptoff, OBJS_PADDING());
		obj.setLinearDamping(ptoff, OBJS_LIN_DAMP());
		obj.setAlgularDamping(ptoff, OBJS_ANG_DAMP());
		obj.setRotation(ptoff, OBJS_R());
		obj.setLinearVelocity(ptoff, OBJS_V());
		obj.setAlgularVelocity(ptoff, OBJS_AV());
		obj.setSleep(ptoff, OBJS_SLEEP());
		obj.setUpdate(ptoff, OBJS_UPDATE());
		obj.setRecreate(ptoff, OBJS_RECREATE());

		BOSS_INTERRUPT(i, N);
		i++;
	}

	obj.updateMaxObjIndex();

	BOSS_END;
}


void SOP_Modify::changeConstraints(GA_PrimitiveGroup* myGroupPr)
{
	BOSS_START;

	SConstraint constr;
	constr.init(&m_boss, gdp);	//create attrs(If It doesnt exist)

	setLocalVars(false);

	bool linlock = CSTR_LINLOCK()!=0;

	int N;
	if(myGroupPr)	N = (int)myGroupPr->entries();
	else			N = (int)gdp->getPrimitiveMap().indexSize();

	int i = 0;
	GEO_Primitive* prim;
	GA_FOR_ALL_OPT_GROUP_PRIMITIVES(gdp, myGroupPr, prim)
	{
		GA_Offset primoff = prim->getMapOffset();
		myCurPrimOff[0] = primoff;	//map local variable(s)

		//change data
		constr.setIndexA(primoff, CSTR_BODY_START());
		constr.setIndexB(primoff, CSTR_BODY_END());
		constr.setStiffness(primoff, CSTR_STIFFNESS());
		constr.setDamping(primoff, CSTR_DAMPING());
		constr.setMaxForce(primoff, CSTR_MAX_FORCE());
		constr.setWeariness(primoff, CSTR_WEARINESS()!=0);
		constr.setIter(primoff, CSTR_ITER());
		constr.setLL(primoff, CSTR_LL());
		constr.setLU(primoff, CSTR_LU());
		constr.setAL(primoff, CSTR_AL());
		constr.setAU(primoff, CSTR_AU());
		constr.setUpdate(primoff, CSTR_UPDATE()!=0);
		constr.setRecreate(primoff, CSTR_RECREATE()!=0);
		constr.setEquilibrium(primoff, CSTR_RESET_EQUILIBRIUM()!=0);

		if(linlock)
			constr.setLinearLock(prim);

		BOSS_INTERRUPT(i, N);
		i++;
	}

	if(CSTR_DEL_DUPLICITY())
		constr.deleteDuplicity(myGroupPr);

	BOSS_END;
}


void SOP_Modify::changeForces(GA_PointGroup* myGroupPt)
{
	BOSS_START;

	SForce forc;
	forc.init(&m_boss, gdp);	//create attrs(If It doesnt exist)

	setLocalVars(true);

	int N;
	if(myGroupPt)	N = (int)myGroupPt->entries();
	else			N = (int)gdp->getPointMap().indexSize();

	int i = 0;
	GA_Offset ptoff;
	GA_FOR_ALL_GROUP_PTOFF(gdp, myGroupPt, ptoff)
	{
		myCurPtOff[0] = ptoff;	//map local variable(s)

		//change data
		forc.setPos(ptoff, FORC_POS());
		forc.setType(ptoff, FORC_TYPE());
		forc.setDir(ptoff, FORC_DIR());
		forc.setScale(ptoff, FORC_SCALE());

		forc.setIndex(ptoff, FORC_INDEX());
		forc.setRad(ptoff, FORC_RAD());
		forc.setFade(ptoff, FORC_FADE());

		BOSS_INTERRUPT(i, N);
		i++;
	}

	BOSS_END;
}





void SOP_Modify::changeEmits(GA_PrimitiveGroup* myGroupPr)
{
	BOSS_START;

	SEmit emit;
	emit.init(&m_boss, gdp);	//create attrs(If It doesnt exist)

	setLocalVars(false);

	//bool linlock = CSTR_LINLOCK()!=0;

	int N;
	if(myGroupPr)	N = (int)myGroupPr->entries();
	else			N = (int)gdp->getPrimitiveMap().indexSize();

	int i = 0;
	GEO_Primitive* prim;
	GA_FOR_ALL_OPT_GROUP_PRIMITIVES(gdp, myGroupPr, prim)
	{
		myCurPrimOff[0] = prim->getMapOffset();	//map local variable(s)

		//change data
		emit.setConnectDistance(myCurPrimOff[0], EMIT_CONNECT_DISTANCE());
		emit.setBreakDistance(myCurPrimOff[0], EMIT_BREAK_DISTANCE());
		emit.setFrame(myCurPrimOff[0], EMIT_FRAME());

		BOSS_INTERRUPT(i, N);
		i++;
	}


	BOSS_END;
}







void SOP_Modify::setLocalVars(bool points)
{
	//set variable privileges(Solve problem If you have same name of local_variable for points, primitives, ...)
	if(points)	setVariableOrder(3, 2, 0, 1);
	else		setVariableOrder(3, 0, 2, 1);
	setCurGdh(0, myGdpHandle);
	setupLocalVars();
}


const char *SOP_Modify::inputLabel(unsigned inum) const
{
    switch (inum)
    {
		case 0: return "Source";
    }
    return "Unknown source";
}



bool SOP_Modify::updateParmsFlags()
{
	bool changed = SOP_Node::updateParmsFlags();

	int w = WORK_OBJS_CSTR_FORC();
	changed |= enableParm("group1", w==TAB_RBDS || w==TAB_FORCES);
	changed |= enableParm("group2", w==TAB_CONSTRAINTS || w==TAB_EMITS);
	return changed;
}



OP_Node *SOP_Modify::myConstructor(OP_Network *net, const char *name, OP_Operator *op)
{
    return new SOP_Modify(net, name, op);
}
SOP_Modify::SOP_Modify(OP_Network *net, const char *name, OP_Operator *op)
	: SOP_Node(net, name, op)
{
    getFullPath(m_operatorName);
}
SOP_Modify::~SOP_Modify()
{
}






static PRM_Name objs_names[] = {
	PRM_Name("objs_index",	"Index"),
	PRM_Name("objs_cindex",	"Compound Index"),
	PRM_Name("objs_type",	"Hull Type"),
	PRM_Name("objs_t",	"Translate"),
	PRM_Name("objs_r",	"Rotate"),
	PRM_Name("objs_cog",	"COG"),
	PRM_Name("objs_cog_r",	"COG Rotation"),

	PRM_Name("objs_mass",	"Mass"),
	PRM_Name("objs_restitution",	"Restitution"),
	PRM_Name("objs_friction",	"Friction"),
	PRM_Name("objs_padding",	"Padding"),
	PRM_Name("objs_v",	"Linear Velocity"),
	PRM_Name("objs_av",	"Angular Velocity"),
	PRM_Name("objs_lin_damp",	"Linear Damping"),
	PRM_Name("objs_ang_damp",	"Angular Damping"),

	PRM_Name("objs_boxsize",	"Box Size"),

	PRM_Name("objs_sleep",	"Sleeping"),
	PRM_Name("objs_update",	"Update Properties"),
	PRM_Name("objs_recreate",	"Recreate"),

};
static PRM_Default objs_def[] = {
	PRM_Default(0, "$BI"),
	PRM_Default(0, "$BCI"),
	PRM_Default(0, "$BTYPE"),
	PRM_Default(0),
	PRM_Default(0),
	PRM_Default(0),
	PRM_Default(0),

	PRM_Default(0, "$BMASS"),
	PRM_Default(0, "$BREST"),
	PRM_Default(0, "$BFRI"),
	PRM_Default(0, "$BPAD"),
	PRM_Default(0),
	PRM_Default(0),
	PRM_Default(0, "$BLDMP"),
	PRM_Default(0, "$BADMP"),

	PRM_Default(0),
	PRM_Default(0, "$BSLEEP"),
	PRM_Default(0, "$BUPDATE"),
	PRM_Default(0, "$BRECREATE"),
};
static PRM_Default objs_trans[] = {PRM_Default(0, "$BTX"), PRM_Default(0, "$BTY"), PRM_Default(0, "$BTZ")};
static PRM_Default objs_rot[] = {PRM_Default(0, "$BRX"), PRM_Default(0, "$BRY"), PRM_Default(0, "$BRZ")};
static PRM_Default objs_cog[] = {PRM_Default(0, "$BCOGX"), PRM_Default(0, "$BCOGY"), PRM_Default(0, "$BCOGZ")};
static PRM_Default objs_cog_r[] = {PRM_Default(0, "$BCOGRX"), PRM_Default(0, "$BCOGRY"), PRM_Default(0, "$BCOGRZ")};
static PRM_Default objs_v[] = {PRM_Default(0, "$BVX"), PRM_Default(0, "$BVY"), PRM_Default(0, "$BVZ")};
static PRM_Default objs_av[] = {PRM_Default(0, "$BAVX"), PRM_Default(0, "$BAVY"), PRM_Default(0, "$BAVZ")};
static PRM_Default objs_bbox[] = {PRM_Default(0, "$BBX"), PRM_Default(0, "$BBY"), PRM_Default(0, "$BBZ")};





static PRM_Name cstr_names[] = {
	PRM_Name("cstr_body_start",	"Index Start Object"),
	PRM_Name("cstr_body_end",	"Index End Object"),
	PRM_Name("cstr_stiffness", "Stiffness"),
	PRM_Name("cstr_damping",	"Damping"),
	PRM_Name("cstr_maxforce",	"Maximum Force"),
	PRM_Name("cstr_linlowerlimit",	"Linear Lower Limit"),
	PRM_Name("cstr_linupperlimit",	"Linear Upper Limit"),
	PRM_Name("cstr_anglowerlimit",	"Angular Lower Limit"),
	PRM_Name("cstr_angupperlimit",	"Angular Upper Limit"),
	PRM_Name("cstr_iter",	"Iteration"),
	PRM_Name("cstr_weariness",	"Weariness"),
	PRM_Name("cstr_reset_equilibrium",	"Reset Equilibrium"),
	PRM_Name("cstr_update",	"Update"),
	PRM_Name("cstr_recreate",	"Recreate"),
	PRM_Name("cstr_sep1", "sep1"),
	PRM_Name("cstr_delduplicity", "Delete Duplicity"),
	PRM_Name("cstr_linlock", "Linear Lock"),

};
static PRM_Default cstr_def[] = {
	PRM_Default(0, "$BIA"),
	PRM_Default(0, "$BIB"),
	PRM_Default(0, "$BSTIFF"),
	PRM_Default(0, "$BDAMP"),
	PRM_Default(0, "$BMAXF"),

	PRM_Default(0),
	PRM_Default(0),
	PRM_Default(0),
	PRM_Default(0),

	PRM_Default(0, "$BITER"),
	PRM_Default(0, "$BWEAR"),
	PRM_Default(0, "$BEQU"),
	PRM_Default(0, "$BUPDATE"),
	PRM_Default(0, "$BRECREATE"),
};
static PRM_Default cstr_ll[] = {PRM_Default(0, "$BLLX"), PRM_Default(0, "$BLLY"), PRM_Default(0, "$BLLZ")};
static PRM_Default cstr_lu[] = {PRM_Default(0, "$BLUX"), PRM_Default(0, "$BLUY"), PRM_Default(0, "$BLUZ")};
static PRM_Default cstr_al[] = {PRM_Default(0, "$BALX"), PRM_Default(0, "$BALY"), PRM_Default(0, "$BALZ")};
static PRM_Default cstr_au[] = {PRM_Default(0, "$BAUX"), PRM_Default(0, "$BAUY"), PRM_Default(0, "$BAUZ")};






static PRM_Name forc_names[] = {
	PRM_Name("forc_type",	"Type"),
	PRM_Name("forc_pos",	"Position"),
	PRM_Name("forc_dir",	"Direction"),
	PRM_Name("forc_scale",	"Scale"),

	PRM_Name("forc_index", "Object index"),
	PRM_Name("forc_rad", "Radius"),
	PRM_Name("forc_fade", "Fade"),
};
static PRM_Default forc_def[] = {
	PRM_Default(0, "$BTYPE"),
	PRM_Default(0),
	PRM_Default(0),
	PRM_Default(0, "$BSCALE"),

	PRM_Default(0, "$BI"),
	PRM_Default(0, "$BRAD"),
	PRM_Default(0, "$BFADE"),
};

static PRM_Default forc_pos[] = {PRM_Default(0, "$BTX"), PRM_Default(0, "$BTY"), PRM_Default(0, "$BTZ")};
static PRM_Default forc_dir[] = {PRM_Default(0, "$BDIRX"), PRM_Default(0, "$BDIRY"), PRM_Default(0, "$BDIRZ")};



static PRM_Name imp_names[] = {
	PRM_Name("imp_delduplicity", "Delete Duplicity"),
};






static PRM_Name emit_names[] = {
	PRM_Name("emit_connect_distance", "Connect Distance"),
	PRM_Name("emit_break_distance", "Break Distance"),
	PRM_Name("emit_frame", "Emit Frame"),
};

static PRM_Default emit_def[] = {
	PRM_Default(0, "$BCD"),
	PRM_Default(0, "$BBD"),
	PRM_Default(0, "$BF"),
};






static PRM_Name name_group1("group1", "RBD, Force Group");
static PRM_Name name_group2("group2", "Constraint Group");

static PRM_Default  switcherList[] = {
    PRM_Default(19, "RBDs"),
	PRM_Default(17, "Constraints"),
	PRM_Default(7, "Forces"),
	PRM_Default(1, "Impacts"),
	PRM_Default(3, "Emits"),
};
static PRM_Name name_work_obj_constr_force("work_obj_constr_force", "work_obj_constr_force");


static PRM_Name g_name_info("tool_name", TOOL_NAME);

PRM_Template
SOP_Modify::myTemplateList[] = {

	PRM_Template(PRM_HEADING ,1, &g_name_info, 0),

	PRM_Template(PRM_STRING,	1, &name_group1, 0, &SOP_Node::pointGroupMenu),
	PRM_Template(PRM_STRING,	1, &name_group2, 0, &SOP_Node::primGroupMenu),

	PRM_Template(PRM_SWITCHER_EXCLUSIVE, 5, &name_work_obj_constr_force, switcherList),

	//RBDs
	PRM_Template(PRM_INT,	1, &objs_names[0], &objs_def[0]),
	PRM_Template(PRM_INT,	1, &objs_names[1], &objs_def[1]),
	PRM_Template(PRM_INT,	1, &objs_names[2], &objs_def[2]),
	PRM_Template(PRM_XYZ_J,	3, &objs_names[3], objs_trans),
	PRM_Template(PRM_XYZ_J,	3, &objs_names[4], objs_rot),
	PRM_Template(PRM_XYZ_J,	3, &objs_names[5], objs_cog),
	PRM_Template(PRM_XYZ_J,	3, &objs_names[6], objs_cog_r),

	PRM_Template(PRM_FLT,	1, &objs_names[7], &objs_def[7]),
	PRM_Template(PRM_FLT,	1, &objs_names[8], &objs_def[8]),
	PRM_Template(PRM_FLT,	1, &objs_names[9], &objs_def[9]),
	PRM_Template(PRM_FLT,	1, &objs_names[10], &objs_def[10]),
	PRM_Template(PRM_XYZ_J,	3, &objs_names[11], objs_v),
	PRM_Template(PRM_XYZ_J,	3, &objs_names[12], objs_av),
	PRM_Template(PRM_FLT,	1, &objs_names[13], &objs_def[13]),
	PRM_Template(PRM_FLT,	1, &objs_names[14], &objs_def[14]),

	PRM_Template(PRM_XYZ_J,	3, &objs_names[15], objs_bbox),

	PRM_Template(PRM_INT,	1, &objs_names[16], &objs_def[16]),
	PRM_Template(PRM_INT,	1, &objs_names[17], &objs_def[17]),
	PRM_Template(PRM_INT,	1, &objs_names[18], &objs_def[18]),


	//Constraints
	PRM_Template(PRM_INT,	1, &cstr_names[0], &cstr_def[0]),
	PRM_Template(PRM_INT,	1, &cstr_names[1], &cstr_def[1]),
	PRM_Template(PRM_FLT,	1, &cstr_names[2], &cstr_def[2]),
	PRM_Template(PRM_FLT,	1, &cstr_names[3], &cstr_def[3]),
	PRM_Template(PRM_FLT,	1, &cstr_names[4], &cstr_def[4]),

	PRM_Template(PRM_XYZ_J,	3, &cstr_names[5], cstr_ll),
	PRM_Template(PRM_XYZ_J,	3, &cstr_names[6], cstr_lu),
	PRM_Template(PRM_XYZ_J,	3, &cstr_names[7], cstr_al),
	PRM_Template(PRM_XYZ_J,	3, &cstr_names[8], cstr_au),

	PRM_Template(PRM_INT,	1, &cstr_names[9], &cstr_def[9]),
	PRM_Template(PRM_INT,	1, &cstr_names[10], &cstr_def[10]),
	PRM_Template(PRM_INT,	1, &cstr_names[11], &cstr_def[11]),
	PRM_Template(PRM_INT,	1, &cstr_names[12], &cstr_def[12]),
	PRM_Template(PRM_INT,	1, &cstr_names[13], &cstr_def[13]),
	
	PRM_Template(PRM_SEPARATOR,	1, &cstr_names[14]),
	PRM_Template(PRM_TOGGLE,	1, &cstr_names[15]),
	PRM_Template(PRM_TOGGLE,	1, &cstr_names[16]),

	//Forces
	PRM_Template(PRM_INT,	1, &forc_names[0], &forc_def[0]),
	PRM_Template(PRM_XYZ_J,	3, &forc_names[1], forc_pos),
	PRM_Template(PRM_XYZ_J,	3, &forc_names[2], forc_dir),
	PRM_Template(PRM_FLT,	1, &forc_names[3], &forc_def[3]),

	PRM_Template(PRM_INT,	1, &forc_names[4], &forc_def[4]),
	PRM_Template(PRM_FLT,	1, &forc_names[5], &forc_def[5]),
	PRM_Template(PRM_FLT,	1, &forc_names[6], &forc_def[6]),

	//Impacts
	PRM_Template(PRM_TOGGLE,	1, &imp_names[0], 0),


	//Emit
	PRM_Template(PRM_FLT,	1, &emit_names[0], &emit_def[0]),
	PRM_Template(PRM_FLT,	1, &emit_names[1], &emit_def[1]),
	PRM_Template(PRM_INT,	1, &emit_names[2], &emit_def[2]),

    PRM_Template()
};

