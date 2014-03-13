/**
BulletSOP v2.0
Copyright (c)2010-2013 Suk Milan. All Rights Reserved.
This software is provided 'as-is', without any express or implied warranty.
In no event will the authors be held liable for any damages arising from the use of this software.
This file may not be redistributed in whole or significant part.
This notice may not be removed or altered from any source distribution.
*/
#include "stdafx.h"
#include <UT/UT_DSOVersion.h>

#include "SOP_Prepare.h"
#include "SOP_Transform.h"
#include "SOP_Build.h"
#include "SOP_Modify.h"
#include "SOP_Loader.h"
#include "SOP_Solver.h"

#include "SOP_ExtraCells.h"

//main procedure for library(plugin)
void newSopOperator(OP_OperatorTable *table)
{
	table->addOperator(new OP_Operator(
										"btPrepare",
										"btPrepare",
										HDK_BulletSOP_Prepare::SOP_Prepare::myConstructor,
										HDK_BulletSOP_Prepare::SOP_Prepare::myTemplateList,
										1,
										2,
										0));

	table->addOperator(new OP_Operator(
										"btBuild",
										"btBuild",
										HDK_BulletSOP_Build::SOP_Build::myConstructor,
										HDK_BulletSOP_Build::SOP_Build::myTemplateList,
										0,
										1,
										0));

	table->addOperator(new OP_Operator(
										"btTransform",
										"btTransform",
										HDK_BulletSOP_Transform::SOP_Transform::myConstructor,
										HDK_BulletSOP_Transform::SOP_Transform::myTemplateList,
										2,
										2,
										0));

	table->addOperator(new OP_Operator(
										"btModify",
										"btModify",
										HDK_BulletSOP_Modify::SOP_Modify::myConstructor,
										HDK_BulletSOP_Modify::SOP_Modify::myTemplateList,
										1,
										1,
										0));


	table->addOperator(new OP_Operator(
										"btLoader",
										"btLoader",
										HDK_BulletSOP_Loader::SOP_Loader::myConstructor,
										HDK_BulletSOP_Loader::SOP_Loader::myTemplateList,
										0,
										1,
										0));

	table->addOperator(new OP_Operator(
										"btSolver",
										"btSolver",
										HDK_BulletSOP_Solver::SOP_Solver::myConstructor,
										HDK_BulletSOP_Solver::SOP_Solver::myTemplateList,
										1,
										4,
										0));



	table->addOperator(new OP_Operator(
										"btExtraCells",
										"btExtraCells",
										HDK_BulletSOP_ExtraCells::SOP_ExtraCells::myConstructor,
										HDK_BulletSOP_ExtraCells::SOP_ExtraCells::myTemplateList,
										2,
										2,
										0));

}

