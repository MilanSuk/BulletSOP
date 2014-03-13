/**
BulletSOP v2.0
Copyright (c)2010-2013 Suk Milan. All Rights Reserved.
This software is provided 'as-is', without any express or implied warranty.
In no event will the authors be held liable for any damages arising from the use of this software.
This file may not be redistributed in whole or significant part.
This notice may not be removed or altered from any source distribution.
*/
#pragma once

#include "SOP_Solver.h"


namespace HDK_BulletSOP_Loader {
/**
SOP_Loader put to ouput all data(m_data vector) from solver( SOLVER_PATH() )
*/
class SOP_Loader : public SOP_Node
{
private:
	SOPBoss m_boss;
	UT_String m_operatorName;
	float m_time;

	MyVec<GU_Detail*> m_data;	///data from solvers

public:
	static MyVec<SOP_Loader*> s_loaders;	///list of all loaders


public:
	SOP_Loader(OP_Network *net, const char *name, OP_Operator *op);
    virtual ~SOP_Loader();

    static PRM_Template myTemplateList[];
    static OP_Node *myConstructor(OP_Network*, const char *, OP_Operator *);

public:
	bool isSolverNode(SOP_Solver* n) const;
	void saveData(GU_Detail* g);
	void deleteData();		///clear m_data vector


protected:
    virtual const char* inputLabel(unsigned idx) const;
    virtual OP_ERROR cookMySop(OP_Context &context);


///GUI
private:
    void SOLVER_PATH(UT_String &str) const	{ evalString(str, "solver_path", 0, m_time); }

public:
	int LOAD_GEO() const { INT_PARM("load_geo", 0, m_time) }
	int LOAD_PROPERTIES() const { INT_PARM("load_poperties", 0, m_time) }
	int LOAD_FORCES() const { INT_PARM("load_forces", 0, m_time) }
	int LOAD_CONSTRAINTS() const { INT_PARM("load_constraints", 0, m_time) }
	int LOAD_IMPACTS() const { INT_PARM("load_impacts", 0, m_time) }
};
};
using namespace HDK_BulletSOP_Loader;
