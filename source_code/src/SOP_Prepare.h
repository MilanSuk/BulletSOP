/**
BulletSOP v2.0
Copyright (c)2010-2013 Suk Milan. All Rights Reserved.
This software is provided 'as-is', without any express or implied warranty.
In no event will the authors be held liable for any damages arising from the use of this software.
This file may not be redistributed in whole or significant part.
This notice may not be removed or altered from any source distribution.
*/
#pragma once

namespace HDK_BulletSOP_Prepare {
/**
SOP_Prepare add atributes to geometry, which divide geometry to objects.
The 2nd input(option), It uses for getting maximum bt_index value.
bt_index is generated from groups or copy number.
bt_index attribute start with value one. And If this attribute is negative, It means that geometry is High poly and is ignores by SOP_Solver and use by SOP_Transform.
They're algorithm which break non-convex objects to convex objects or voxels(grid points).
*/
class SOP_Prepare : public SOP_Node
{
private:
	float m_time;
	SOPBoss m_boss;
	UT_String m_operatorName;

	GU_Detail* m_input1;
	GU_Detail* m_input2;

public:
	SOP_Prepare(OP_Network *net, const char *name, OP_Operator *op);
    virtual ~SOP_Prepare();
    static PRM_Template myTemplateList[];
    static OP_Node *myConstructor(OP_Network*, const char *, OP_Operator *);


protected:
    virtual const char* inputLabel(unsigned idx) const;
    virtual OP_ERROR cookMySop(OP_Context &context);
	virtual bool updateParmsFlags();



private:
	int INDEX_START() const { INT_PARM("index_start", 0, m_time) }
	int ORIGINAL_GEO() const { INT_PARM("original_geo", 0, m_time) }
	float PADDING() const { FLT_PARM("padding", 0, m_time) }
	int NORMAL_TYPE() const { INT_PARM("normal_type", 0, m_time) }
	float PADDING_EXTRA() const { FLT_PARM("padding_extra", 0, m_time) }

	int WORKGROUPORCOPY() const { INT_PARM("work_group_or_copy", 0, m_time) }
		UT_String& GROUP_NAME(UT_String &str) const	{ evalString(str, "group_name", 0, 0, m_time);	return str; }
		int DELETEALLGROUPS() const { INT_PARM("group_delete_all", 0, m_time) }
		int DELETEGROUPS() const { INT_PARM("group_delete", 0, m_time) }

		int COPYNUMPOINTS() const { INT_PARM("copy_npoints", 0, m_time) }
		int COPYNUMPRIMS() const { INT_PARM("copy_nprims", 0, m_time) }

	int WORK_COMP_DECOMP() const { INT_PARM("work_comp_decomp", 0, m_time) }
		int COMPOUND_NUM() const { INT_PARM("compound_num", 0, m_time) }
		int COMPOUND_FIRST() const { INT_PARM("compound_first", 0, m_time) }

		int DECOMP_WORK_TETRA_VOXEL() const { INT_PARM("decomp_work_tetra_voxel", 0, m_time) }
			int DECOMP_CONVEX_PRECISE() const { INT_PARM("decomp_convex_precise", 0, m_time) }
			float DECOMP_CONVEX_TOLER() const {FLT_PARM("decomp_convex_toler", 0, m_time) }

			int DECOMP_TETRA_NUM_RAY_STEPS() const { INT_PARM("decomp_tetra_num_ray_steps", 0, m_time) }

			float DECOMP_VOXEL_CELL_SIZE() const { FLT_PARM("decomp_voxel_cell_size", 0, m_time) }

		int DECOMP_COMPOUND_GEO() const { INT_PARM("decomp_compound_geo", 0, m_time) }


private:
	enum{
		TAB_GROUP,
		TAB_COPIES,
	};

	enum{
		TAB_COMP,
		TAB_DECOMP,
	};

	enum{
		TAB_TETRA,
		TAB_VOXEL,
	};
};
};
using namespace HDK_BulletSOP_Prepare;
