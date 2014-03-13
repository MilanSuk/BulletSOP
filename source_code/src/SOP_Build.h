/**
BulletSOP v2.0
Copyright (c)2010-2013 Suk Milan. All Rights Reserved.
This software is provided 'as-is', without any express or implied warranty.
In no event will the authors be held liable for any damages arising from the use of this software.
This file may not be redistributed in whole or significant part.
This notice may not be removed or altered from any source distribution.
*/
#pragma once

namespace HDK_BulletSOP_Build {
/**
SOP_Build creates properties data for physical object:
- RBDs(mass, cog, friction, ...)			-> POINTS
- CONSTRAINTs(stiffness, damping, limits)	-> PRIMITIVES
- FORCEs(pos, direction, repel, fadeú)		-> POINTS
- DIRECTORY(line_id for bt_index)			-> PRIMITIVES
*/
class SOP_Build : public SOP_Node
{
private:
	float m_time;
	UT_String m_operatorName;
	SOPBoss m_boss;


public:
	SOP_Build(OP_Network *net, const char *name, OP_Operator *op);
    virtual ~SOP_Build();

    static PRM_Template myTemplateList[];
    static OP_Node *myConstructor(OP_Network*, const char *, OP_Operator *);


protected:
    virtual const char* inputLabel(unsigned idx) const;
    virtual OP_ERROR cookMySop(OP_Context &context);
	virtual bool updateParmsFlags();


private:
	void fillRBDs(GU_Detail* input_1, GA_PointGroup* group_pt);
	void fillConstraints(GU_Detail* input_1, GA_PrimitiveGroup* group_pr);
	void fillForces(GU_Detail* input_1, GA_PointGroup* group_pt);
	void fillDirectory(GU_Detail* input_1);	///Create directory(indexes) for user defined attribute
	void fillEmit(GU_Detail* input_1, GA_PrimitiveGroup* group_pr);


private:
	int CREATE_GROUP() const { INT_PARM("create_group", 0, m_time) }
    UT_String& GROUP_NAME(UT_String &str) const	{ evalString(str, "group_name", 0, 0);	return str; }


	int WORK_OBJS_CSTR_FORC() const	{ INT_PARM("work_obj_constr_force", 0, m_time) }

	int OBJS_HULL() const { INT_PARM("objs_hull", 0, m_time) }
	int OBJS_COG() const { INT_PARM("objs_cog", 0, m_time) }
	int OBJS_STATIC() const { INT_PARM("objs_static", 0, m_time) }
	int OBJS_RECREATE() const { INT_PARM("objs_recreate", 0, m_time) }
	int OBJS_UPDATE() const { INT_PARM("objs_update", 0, m_time) }


	float OBJS_DENSITY() const { FLT_PARM("objs_density", 0, m_time) }
	int OBJS_MASS_FROM_VOLUME() const { INT_PARM("objs_mass_from_volume", 0, m_time) }
	int OBJS_COPY_PRIM_ATTRS() const { INT_PARM("objs_copy_prim_attrs", 0, m_time) }

	

	int CSTR_STAT_GEN() const { INT_PARM("cstr_constraints_tab", 0, m_time) }
	int CSTR_STAT_NUM_TABS() const { INT_PARM("cstr_stat_constraints_tab", 0, m_time) }

	float CSTR_GEN_STIFFNESS() const { FLT_PARM("cstr_gen_stiffness", 0, m_time) }
	float CSTR_GEN_DAMPING() const { FLT_PARM("cstr_gen_damping", 0, m_time) }
	float CSTR_GEN_MAXFORCE() const { FLT_PARM("cstr_gen_maxforce", 0, m_time) }
	float CSTR_GEN_TOLER_DISTANCE() { FLT_PARM("cstr_gen_toler_distance", 0, m_time) }
	int CSTR_GEN_WEARINESS() const { INT_PARM("cstr_gen_weariness", 0, m_time) }
	float CSTR_GEN_MEMORY() const { FLT_PARM("cstr_gen_memory", 0, m_time) }
	int CSTR_GEN_ITER() const { INT_PARM("cstr_gen_iter", 0, m_time) }
	


	int CSTR_STAT_BODY_START(int* i) const { INT_PARM_MULT("cstr_stat_body_start", 0, m_time, i) }
	int CSTR_STAT_BODY_END(int* i)  const { INT_PARM_MULT("cstr_stat_body_end", 0, m_time, i) }
	UT_Vector3 CSTR_STAT_POS_START(int* i)  const { V3_PARM_MULT("cstr_stat_pos_start", m_time, i) }
	UT_Vector3 CSTR_STAT_POS_END(int* i)  const { V3_PARM_MULT("cstr_stat_pos_end", m_time, i) }
	float CSTR_STAT_STIFFNESS(int* i)  const { FLT_PARM_MULT("cstr_stat_stiffness", 0, m_time, i) }
	float CSTR_STAT_DAMPING(int* i)  const { FLT_PARM_MULT("cstr_stat_damping", 0, m_time, i) }
	float CSTR_STAT_MAX_FORCE(int* i) const { FLT_PARM_MULT("cstr_stat_maxforce", 0, m_time, i) }
	int CSTR_STAT_LOCK(int* i) const { INT_PARM_MULT("cstr_stat_lock", 0, m_time, i) }
	UT_Vector3 CSTR_STAT_LL(int* i) const { V3_PARM_MULT("cstr_stat_linlowerlimit", m_time, i) }
	UT_Vector3 CSTR_STAT_LU(int* i) const { V3_PARM_MULT("cstr_stat_linupperlimit", m_time, i) }
	UT_Vector3 CSTR_STAT_AL(int* i) const { V3_PARM_MULT("cstr_stat_anglowerlimit", m_time, i) }
	UT_Vector3 CSTR_STAT_AU(int* i) const { V3_PARM_MULT("cstr_stat_angupperlimit", m_time, i) }
	int CSTR_STAT_WEARINESS(int* i) const { INT_PARM_MULT("cstr_stat_weariness", 0, m_time, i) }
	int CSTR_STAT_ITER(int* i) { INT_PARM_MULT("cstr_stat_iter", 0, m_time, i) }


	int FORC_NUM_TABS() const { INT_PARM("forc_forces_tab", 0, m_time) }
	int FORC_TYPE(int* i) const { INT_PARM_MULT("forc_type", 0, m_time, i) }
	UT_Vector3 FORC_POS(int* i) const { V3_PARM_MULT("forc_pos", m_time, i) }
	UT_Vector3 FORC_DIR(int* i) const { V3_PARM_MULT("forc_dir", m_time, i) }
	float FORC_SCALE(int* i) const { FLT_PARM_MULT("forc_scale", 0, m_time, i) }
	int FORC_OBJ_RAD(int* i) const { INT_PARM_MULT("forc_obj_rad", 0, m_time, i) }
		int FORC_INDEX(int* i) const { INT_PARM_MULT("forc_index", 0, m_time, i) }

		float FORC_RAD(int* i) const { FLT_PARM_MULT("forc_rad", 0, m_time, i) }
		float FORC_FADE(int* i) const { FLT_PARM_MULT("forc_fade", 0, m_time, i) }


    UT_String& DIR_NAME(UT_String &str) const	{ evalString(str, "dir_name", 0, 0);	return str; }
	int DIR_CLASS() const { INT_PARM("dir_class", 0, m_time) }


	int EMIT_SET_DISTANCES() const { INT_PARM("emit_set_distances", 0, m_time) }
	float EMIT_BREAK_DISTANCE() const { FLT_PARM("emit_break_distance", 0, m_time) }
	float EMIT_CONNECT_DISTANCE() const { FLT_PARM("emit_connect_distance", 0, m_time) }


public:
	enum{
		RBDS,
		CONSTRAINTS,
		FORCES,
		DIRECTORY,
		EMIT,
	};

	enum{
		CONSTRAINTS_STATIC,
		CONSTRAINTS_GEN,
	};

	enum{
		COG_BBOX,
		COG_AVERAGE,
		COG_VOLUME,
	};

	enum{
		TYPE_CONVEX,
		TYPE_SPHERE,
		TYPE_BOX,
		TYPE_CAPSULE,
		TYPE_CONE,
		TYPE_CYLINDER,
		TYPE_DEFORM,
		TYPE_COMPOUND,
	};

	enum{
		CLASS_POINT,
		CLASS_PRIMITVE,
	};


	enum{
		FULL,
		CENTRAL,
		TORQUE,
		MAGNET,
	};

};
};
using namespace HDK_BulletSOP_Build;

