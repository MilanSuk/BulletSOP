/**
BulletSOP v2.0
Copyright (c)2010-2013 Suk Milan. All Rights Reserved.
This software is provided 'as-is', without any express or implied warranty.
In no event will the authors be held liable for any damages arising from the use of this software.
This file may not be redistributed in whole or significant part.
This notice may not be removed or altered from any source distribution.
*/
#pragma once




class SShape;

namespace HDK_BulletSOP_Transform {
/**
SOP_Transform transform geometry data(2nd input) according object properties(1st input).
Firstly It allocates objects struct which fill with transform matrixes compute from properties then It transforms geometry with object data.
*/
class SOP_Transform : public SOP_Node
{
private:
	double m_time;
	SOPBoss m_boss;
	UT_String m_operatorName;
	GU_Detail* m_input_proper;
	GU_Detail* m_input_geo;

	class OBJ
	{
	public:
		bool use;
		bool high;
		bool subCompound;
		UT_Matrix4 mat;
		UT_Vector3 cog;
		UT_Vector3 v;
		UT_Vector3 av;

		OBJ()
		{
			use = false;
			high = false;
			subCompound = false;
		}
	};
	MyVec<OBJ> m_objs;


public:
	SOP_Transform(OP_Network *net, const char *name, OP_Operator *op);
    virtual ~SOP_Transform();

    static PRM_Template myTemplateList[];
    static OP_Node *myConstructor(OP_Network*, const char *, OP_Operator *);

protected:
    virtual const char* inputLabel(unsigned idx) const;
    virtual OP_ERROR cookMySop(OP_Context &context);
	virtual bool updateParmsFlags();

private:
	void computeTransform();	///Computes transform for every object
	void transformGeo(SShape* shape);	///Transforms geometry(output)
	void deleteUnused(SShape* shape);	///Deletes primitives which dont have properties in objs(no infromation in m_input_proper)
	UT_Vector3 getPointVelocity(UT_Vector3 rel, OBJ &obj);	///@return absolute velocity of object(obj) in relative coordinates of point(rel) on obj
	void deletePreferHigh(SShape* shape);	///Extracts geometry for object
	void deleteIndexAttributes();
	void updateEmitFrame(int act_frame);
	void extractEmitPoly(int act_frame);


private:
	int	DELETE_GEO() const { INT_PARM("delete_geo", 0, m_time) }					///Deletes primitives, which dont have transform information in 1st input.
	int	DELETE_SUBCOMPOUND() const { INT_PARM("delete_subcompound", 0, m_time) }	///Deletes points & primitives which have bt_cindex > 0
	int DELETE_INDEX_ATTRS() const { INT_PARM("delete_index_attrs", 0, m_time) }	///Removes "bt_index" primitive and point attributes.
	int ADD_POINT_VELOCITY() const { INT_PARM("add_point_velocity", 0, m_time) }	///Adds and compute "v" point attribute.
	int WORK_OUTPUT() const { INT_PARM("work_output", 0, m_time) }					///Extracts geometry according bt_index

	int UPDATE_EMIT_FRAME() const { INT_PARM("update_emit_frame", 0, m_time) }
	int EXTRACT_EMIT_POLY() const { INT_PARM("extract_emit_poly", 0, m_time) }
	int EXTRACT_EMIT_POLY_MAX_AGE() const { INT_PARM("extract_emit_poly_max_age", 0, m_time) }

	enum	//WORK_OUTPUT
	{
		OUT_PREFER_HIGH,
		OUT_HIGH,
		OUT_LOW,
		OUT_BOTH,
	};
};
};
using namespace HDK_BulletSOP_Transform;
