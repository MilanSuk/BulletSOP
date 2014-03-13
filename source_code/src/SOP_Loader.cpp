/**
BulletSOP v2.0
Copyright (c)2010-2013 Suk Milan. All Rights Reserved.
This software is provided 'as-is', without any express or implied warranty.
In no event will the authors be held liable for any damages arising from the use of this software.
This file may not be redistributed in whole or significant part.
This notice may not be removed or altered from any source distribution.
*/
#include "stdafx.h"
#include "SOP_Loader.h"
#include "SOP_Solver.h"

MyVec<SOP_Loader*> HDK_BulletSOP_Loader::SOP_Loader::s_loaders;	//list of all loaders

OP_Node *SOP_Loader::myConstructor(OP_Network *net, const char *name, OP_Operator *op)
{
    return new SOP_Loader(net, name, op);
}


SOP_Loader::SOP_Loader(OP_Network *net, const char *name, OP_Operator *op)
	: SOP_Node(net, name, op)
{
	getFullPath(m_operatorName);

	s_loaders.push_back(this);
}


SOP_Loader::~SOP_Loader()
{
	//delete this solver from list of all solvers
	for(int i=0; i < s_loaders.size(); i++)
		if(s_loaders[i]==this)
		{
			s_loaders.erase(i);
			break;
		}

	deleteData();
}



OP_ERROR SOP_Loader::cookMySop(OP_Context &context)
{
    if (lockInputs(context) >= UT_ERROR_ABORT)
		return error();
	m_time = (float)context.getTime();
 	m_boss = SOPBoss( UTgetInterrupt() );
    OP_Node::flags().timeDep = 1;	//update every frame

	try
	{
		gdp->clearAndDestroy();	//clear output

		UT_String path;
		SOLVER_PATH(path);
		OP_Node* node = this->findNode(path);	//get node from path parameter		//OPgetDirector()
		if(node)
		{
			if(node->getOperator()->getName().equal("btSolver"))	//is this right node
			{
				SOP_Solver* solver = static_cast<SOP_Solver*>(node);	//cast

				const int currframe = (int)OPgetDirector()->getChannelManager()->getSample(context.getTime());
				if(currframe > solver->RESET_FRAME())	//only if we are after reset frame
				{
					BOSS_START;
					for(int i=0; i < m_data.size(); i++)
					{
						GU_Detail* d = m_data[i];
						if(d)
							gdp->merge(*d);	//merge geo to output

						BOSS_INTERRUPT_EMPTY;
					}
					BOSS_END;
				}
				else
				{
					//on reset frame, It just duplicates input
					if(inputGeo(0, context))
						duplicateSource(0, context);
				}
			}
			else
				THROW_SOP("Path node is not \"btSolver\"", 0);
		}
		else
			THROW_SOP("Path node doesn't exist!", 0);

	//	deleteData();	//create problem If this function is call more than once on same frame. So we call this in SOP_Solver
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



bool SOP_Loader::isSolverNode(SOP_Solver* n) const
{
	UT_String path;	SOLVER_PATH(path);		//get path
	OP_Node* node = this->findNode(path);	//get node from path parameter
	return node==n;
}

void SOP_Loader::saveData(GU_Detail* g)
{
	m_data.push_back(g);
}

void SOP_Loader::deleteData()
{
	for(int i=0; i < m_data.size(); i++)
		if(m_data[i])
			delete m_data[i];
	m_data.clear();
}



const char *SOP_Loader::inputLabel(unsigned inum) const
{
    switch (inum)
    {
		case 0: return "Source";
    }
    return "Unknown source";
}




static PRM_Name names[] = {
    PRM_Name("solver_path",	"Solver Path"),

	PRM_Name("sep1", "sep1"),

	PRM_Name("load_geo", "Load Shapes(Geometry)"),
	PRM_Name("load_poperties", "Load Properties"),
	PRM_Name("load_forces",	"Load Forces"),
	PRM_Name("load_constraints", "Load Constraints"),
	PRM_Name("load_impacts",	"Load Impacts"),
};

static PRM_Name g_name_info("tool_name", TOOL_NAME);


PRM_Template SOP_Loader::myTemplateList[] = {
	PRM_Template(PRM_HEADING ,1, &g_name_info, 0),
	PRM_Template(PRM_STRING, PRM_TYPE_DYNAMIC_PATH,	1, &names[0], 0, 0, 0, 0, &PRM_SpareData::sopPath),

	PRM_Template(PRM_SEPARATOR, 1, &names[1]),

    PRM_Template(PRM_TOGGLE,1, &names[2], 0),
    PRM_Template(PRM_TOGGLE,1, &names[3], 0),
    PRM_Template(PRM_TOGGLE,1, &names[4], 0),
    PRM_Template(PRM_TOGGLE,1, &names[5], 0),
    PRM_Template(PRM_TOGGLE,1, &names[6], 0),

    PRM_Template()
};
