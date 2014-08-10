/**
BulletSOP v2.0
Copyright (c)2010-2013 Suk Milan. All Rights Reserved.
This software is provided 'as-is', without any express or implied warranty.
In no event will the authors be held liable for any damages arising from the use of this software.
This file may not be redistributed in whole or significant part.
This notice may not be removed or altered from any source distribution.
*/
#pragma once

class BWorld;
class BRigidBody;
class SObject;
class SConstraint;

namespace HDK_BulletSOP_Solver {
/**
SOP_Solver solves simulation.
It's bridge between Houdini and Bullet Library.
It loads data from geometry to Bullet Solver, then It runs solving, then update data on output from solver data.
If some SOP_Loader needs data from this solver. This solver send this data to loader after solving.
*/
class SOP_Solver : public SOP_Node
{
private:
	SOPBoss m_boss;
	UT_String m_operatorName;
	float m_time;

	GU_Detail* m_input_prop;
	GU_Detail* m_input_geo;
	GU_Detail* m_input_const;
	GU_Detail* m_input_forc;

	BWorld* m_bullet;
	bool m_foreign_bullet;

	int m_resetFrame;
	int m_lastFrame;

public:
	SOP_Solver(OP_Network *net, const char *name, OP_Operator *op);
    virtual ~SOP_Solver();

    static PRM_Template myTemplateList[];
    static OP_Node *myConstructor(OP_Network*, const char *, OP_Operator *);

protected:
    virtual const char* inputLabel(unsigned idx) const;
    virtual OP_ERROR cookMySop(OP_Context &context);
	virtual bool updateParmsFlags();

private:
	BWorld* getBullet();
	void createBullet();
	void deleteBullet();

	void step();	///simulates

	BRigidBody* createObject(const SObject &o, const GA_Offset &ptoff, UT_Vector3 t, UT_Vector3 r, UT_Vector3 cog_r, int index, int type, BRigidBody* compound, bool save_vertexes);	///creates RBD object

	void updateObjects();
	void updateObjectsProperties(SObject &o, bool updateOnlyCompound);
	void updateConstraints();
	void updateConstr(SConstraint* cstr, GEO_Primitive* prim);
	void updateForces();
	void updateCollisionGroups();
	void updateIgnoreCollisionGroups();

	void setOutputFull(GU_Detail* g, bool update);
	void setOutputTransform(bool add_velocity, GU_Detail* g, bool update);
	void setOutputInstance(GU_Detail* g, bool update);
	void setOutputConstraint(GU_Detail* gout);
	void deleteOuputSubcompounds(GU_Detail* gout);	///deletes properties, which has bt_cindex > 0

	void backup(int currframe);	///sends data to SOP_Loader(s)
	bool isImpactDataNeeded();	///checks all SOP_Loader(s), If They need Impact data


public:
    int RESET_FRAME() const { INT_PARM("reset_frame", 0, 0) }
private:
    UT_String& SOLVER_PATH(UT_String &str) const { evalString(str, "solver_path", 0, 0);	return str; }
    int SUBSTEPS() const { INT_PARM("substeps", 0, m_time) }
    int FPS() const { INT_PARM("fps", 0, m_time) }
	UT_Vector3 GFORCE() const { V3_PARM("gforce", m_time) }

	int GROUND_PLANE() const { INT_PARM("ground_plane", 0, m_time) }

	int SHARE_SHAPES() const { INT_PARM("share_shapes", 0, m_time) }

	int FORCE_SUBSTEPPING() const { INT_PARM("force_substepping", 0, m_time) }
	int CONSTRAINT_SUBSTEPPING() const { INT_PARM("constraint_substepping", 0, m_time) }

	int OUTPUT() const { INT_PARM("output", 0, m_time) }
	int DELETE_THREAD() const { INT_PARM("delete_thread", 0, m_time) }
	int SOLVE_ON_SAME_FRAME() const { INT_PARM("solve_on_same_frame", 0, m_time) }
	int DELETE_SUBCOMPOUNDS() const { INT_PARM("delete_subcompounds", 0, m_time) }

    UT_String& COLLISION_GROUP_NAME(UT_String &str) const	{ evalString(str, "collision_group_name", 0, 0);	return str; }

	int ADV_btBroadphaseInterface() const { INT_PARM("btBroadphaseInterface", 0, 0) }
	int ADV_btConstraintSolver() const { INT_PARM("btConstraintSolver", 0, 0) }

	UT_Vector3 ADV_WORLD_CENTER() const	{ V3_PARM("worldCenter", m_time) }
	UT_Vector3 ADV_WORLD_SIZE() const	{ V3_PARM("worldSize", m_time) }
	int ADV_maxSmallProxies() const { INT_PARM("maxSmallProxies", 0, 0) }
	int ADV_maxLargeProxies() const { INT_PARM("maxLargeProxies", 0, 0) }

	int ADV_maxPairsPerBody() const { INT_PARM("maxPairsPerBody", 0, 0) }
	int ADV_maxBodiesPerCell() const { INT_PARM("maxBodiesPerCell", 0, 0) }
	float ADV_cellFactorAABB() const { FLT_PARM("cellFactorAABB", 0, 0) }

	int ADV_solver_numThreads() const { INT_PARM("solver_numThreads", 0, 0) }

	int ADV_defaultMaxPersistentManifoldPoolSize() const { INT_PARM("defaultMaxPersistentManifoldPoolSize", 0, 0) }
	int ADV_defaultMaxCollisionAlgorithmPoolSize() const { INT_PARM("defaultMaxCollisionAlgorithmPoolSize", 0, 0) }


public:
	enum{OUT_FULL, OUT_TRANSFORM, OUT_TRANSFORM_VELOCITY, OUT_INSTANCE, OUT_ZERO};
	enum{BROAD_DBVT, BROAD_AXIS3_32BIT, BROAD_GPU3D};
	enum{SOLVER_SEQ, SOLVER_PARALLEL, SOLVER_MLCP};
};
};
using namespace HDK_BulletSOP_Solver;
