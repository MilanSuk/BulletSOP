/**
BulletSOP v2.0
Copyright (c)2010-2013 Suk Milan. All Rights Reserved.
This software is provided 'as-is', without any express or implied warranty.
In no event will the authors be held liable for any damages arising from the use of this software.
This file may not be redistributed in whole or significant part.
This notice may not be removed or altered from any source distribution.
*/
#pragma once

namespace HDK_BulletSOP_ExtraCells {
/**
SOP_ExtraCells ...
*/
class SOP_ExtraCells : public SOP_Node
{
private:
	float m_time;
	UT_String m_operatorName;
	SOPBoss m_boss;


public:
	SOP_ExtraCells(OP_Network *net, const char *name, OP_Operator *op);
    virtual ~SOP_ExtraCells();

    static PRM_Template myTemplateList[];
    static OP_Node *myConstructor(OP_Network*, const char *, OP_Operator *);


protected:
    virtual const char* inputLabel(unsigned idx) const;
    virtual OP_ERROR cookMySop(OP_Context &context);
	virtual bool updateParmsFlags();


private:
    UT_String& ATTRIBUTE_NAME(UT_String &str) const	{ evalString(str, "attribute_name", 0, 0);	return str; }
	int DELETE_EMPTY() const						{ INT_PARM("delete_empty", 0, m_time) }
	UT_String& GROUP_NAME(UT_String &str) const		{ evalString(str, "group_name", 0, 0);	return str; }



private:
	struct OBJ
	{
		MyVec<GEO_Primitive*> i1;
		MyVec<GEO_Primitive*> i2;
	};


public:
	enum{
		BOTH,
		INTPUT_1,
		INTPUT_2,
	};

};
};
using namespace HDK_BulletSOP_ExtraCells;

