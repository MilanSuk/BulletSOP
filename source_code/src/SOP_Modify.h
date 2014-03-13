/**
BulletSOP v2.0
Copyright (c)2010-2013 Suk Milan. All Rights Reserved.
This software is provided 'as-is', without any express or implied warranty.
In no event will the authors be held liable for any damages arising from the use of this software.
This file may not be redistributed in whole or significant part.
This notice may not be removed or altered from any source distribution.
*/
#pragma once

namespace HDK_BulletSOP_Modify {
/**
SOP_Modify changes properties data.
It copies input to output and set(copy) parameters(GUI) to attributes.
Local variable are allowed as well as apply changes only on data in group.
*/
class SOP_Modify : public SOP_Node
{
private:
	float m_time;
	int m_currframe;
	UT_String m_operatorName;
	SOPBoss m_boss;


public:
	SOP_Modify(OP_Network *net, const char *name, OP_Operator *op);
    virtual ~SOP_Modify();

    static PRM_Template myTemplateList[];
    static OP_Node *myConstructor(OP_Network*, const char *, OP_Operator *);

protected:
    virtual bool updateParmsFlags();
    virtual const char* inputLabel(unsigned idx) const;
    virtual OP_ERROR cookMySop(OP_Context &context);

private:
	void setLocalVars(bool points);	///Sets local variable(s) privileges
	void getGroup(GA_PointGroup* &myGroupPt, GA_PrimitiveGroup* &myGroupPr);	///loads groups from parameters and check If groups exist
	void changeRBDs(GA_PointGroup* myGroupPt);
	void changeConstraints(GA_PrimitiveGroup* myGroupPr);
	void changeForces(GA_PointGroup* myGroupPt);
	void changeEmits(GA_PrimitiveGroup* myGroupPr);

private:
    UT_String& GROUP_NAME1(UT_String &str) const	{ evalString(str, "group1", 0, 0, m_time);	return str; }
	UT_String& GROUP_NAME2(UT_String &str) const	{ evalString(str, "group2", 0, 0, m_time);	return str; }

	int WORK_OBJS_CSTR_FORC() const	{ INT_PARM("work_obj_constr_force", 0, m_time) }


	int OBJS_INDEX() const { INT_PARM("objs_index", 0, m_time) }
	int OBJS_CINDEX() const { INT_PARM("objs_cindex", 0, m_time) }
	int OBJS_TYPE() const { INT_PARM("objs_type", 0, m_time) }
	UT_Vector3 OBJS_T() const { V3_PARM("objs_t", m_time) }
	UT_Vector3 OBJS_R() const { V3_PARM("objs_r", m_time) }
	UT_Vector3 OBJS_COG() const { V3_PARM("objs_cog", m_time) }
	UT_Vector3 OBJS_COG_R() const { V3_PARM("objs_cog_r", m_time) }
	float OBJS_MASS() const { FLT_PARM("objs_mass", 0, m_time) }
	float OBJS_RESTITUTION() const { FLT_PARM("objs_restitution", 0, m_time) }
	float OBJS_FRICTION() const { FLT_PARM("objs_friction", 0, m_time) }
	float OBJS_PADDING() const { FLT_PARM("objs_padding", 0, m_time) }
	UT_Vector3 OBJS_V() const { V3_PARM("objs_v", m_time) }
	UT_Vector3 OBJS_AV() const { V3_PARM("objs_av", m_time) }
	float OBJS_LIN_DAMP() const { FLT_PARM("objs_lin_damp", 0, m_time) }
	float OBJS_ANG_DAMP() const { FLT_PARM("objs_ang_damp", 0, m_time) }

	UT_Vector3 OBJS_BOXSIZE() const { V3_PARM("objs_boxsize", m_time) }
//	int OBJS_CONVEX() const { INT_PARM("objs_convex", 0, m_time) }
	int OBJS_SLEEP() const { INT_PARM("objs_sleep", 0, m_time) }
	int OBJS_UPDATE() const { INT_PARM("objs_update", 0, m_time) }
	int OBJS_RECREATE() const { INT_PARM("objs_recreate", 0, m_time) }
	

		


	int CSTR_BODY_START() const { INT_PARM("cstr_body_start", 0, m_time) }
	int CSTR_BODY_END() const { INT_PARM("cstr_body_end", 0, m_time) }
	float CSTR_STIFFNESS() const { FLT_PARM("cstr_stiffness", 0, m_time) }
	float CSTR_DAMPING() const { FLT_PARM("cstr_damping", 0, m_time) }
	float CSTR_MAX_FORCE() const { FLT_PARM("cstr_maxforce", 0, m_time) }

	UT_Vector3 CSTR_LL() const { V3_PARM("cstr_linlowerlimit", m_time) }
	UT_Vector3 CSTR_LU() const { V3_PARM("cstr_linupperlimit", m_time) }
	UT_Vector3 CSTR_AL() const { V3_PARM("cstr_anglowerlimit", m_time) }
	UT_Vector3 CSTR_AU() const { V3_PARM("cstr_angupperlimit", m_time) }

	int CSTR_WEARINESS() const { INT_PARM("cstr_weariness", 0, m_time) }
	int CSTR_ITER() const { INT_PARM("cstr_iter", 0, m_time) }

	int CSTR_DEL_DUPLICITY() const { INT_PARM("cstr_delduplicity", 0, m_time) }

	int CSTR_UPDATE() const { INT_PARM("cstr_update", 0, m_time) }
	int CSTR_RECREATE() const { INT_PARM("cstr_recreate", 0, m_time) }
	int CSTR_RESET_EQUILIBRIUM() const { INT_PARM("cstr_reset_equilibrium", 0, m_time) }

	int CSTR_LINLOCK() const { INT_PARM("cstr_linlock", 0, m_time) }



	int FORC_TYPE() const { INT_PARM("forc_type", 0, m_time) }
	UT_Vector3 FORC_POS() const { V3_PARM("forc_pos", m_time) }
	UT_Vector3 FORC_DIR() const { V3_PARM("forc_dir", m_time) }
	float FORC_SCALE() const { FLT_PARM("forc_scale", 0, m_time) }

	int FORC_INDEX() const { INT_PARM("forc_index", 0, m_time) }
	float FORC_RAD() const { FLT_PARM("forc_rad", 0, m_time) }
	float FORC_FADE() const { FLT_PARM("forc_fade", 0, m_time) }


	int IMP_DEL_DUPLICITY() const { INT_PARM("imp_delduplicity", 0, m_time) }



	float EMIT_CONNECT_DISTANCE() const { FLT_PARM("emit_connect_distance", 0, m_time) }
	float EMIT_BREAK_DISTANCE() const { FLT_PARM("emit_break_distance", 0, m_time) }
	int EMIT_FRAME() const { INT_PARM("emit_frame", 0, m_time) }



	enum{
		TAB_RBDS,
		TAB_CONSTRAINTS,
		TAB_FORCES,
		TAB_IMPACTS,
		TAB_EMITS,
	};
};
};
using namespace HDK_BulletSOP_Modify;
