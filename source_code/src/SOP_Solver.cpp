/**
BulletSOP v2.0
Copyright (c)2010-2013 Suk Milan. All Rights Reserved.
This software is provided 'as-is', without any express or implied warranty.
In no event will the authors be held liable for any damages arising from the use of this software.
This file may not be redistributed in whole or significant part.
This notice may not be removed or altered from any source distribution.
*/
#include "stdafx.h"
#include "SOP_Solver.h"

#include "SOP_Loader.h"
#include "SOP_Build.h"

#include "BWorld.h"

#include "SShape.h"
#include "SObject.h"
#include "SConstraint.h"
#include "SForce.h"

SOP_Solver::SOP_Solver(OP_Network *net, const char *name, OP_Operator *op)
	: SOP_Node(net, name, op)
{
	getFullPath(m_operatorName);

	m_bullet = 0;
	m_foreign_bullet = false;
	m_lastFrame = -1000000000;
}
SOP_Solver::~SOP_Solver()
{
	deleteBullet();
}


BWorld* SOP_Solver::getBullet()
{
	return m_bullet;
}

void SOP_Solver::deleteBullet()
{
	if(!m_foreign_bullet)	//Foreign node can't remove orignal bullet world
	{
		if(m_bullet)
			delete m_bullet;
	}
	m_bullet = 0;
}

void SOP_Solver::createBullet()
{
	UT_String path;
	SOLVER_PATH(path);
	OP_Node* foreignNode = this->findNode(path);
	if(foreignNode)
	{
		if(foreignNode->getOperator()->getName().equal("btSolver"))
		{
			m_bullet = ((SOP_Solver*)foreignNode)->getBullet();	//get bullet from other node
			m_foreign_bullet = true;
			return;
		}
		else
			THROW_SOP("solver_path isnt \"btSolver\" operator", 0);

	}
	else if(path.length())
		THROW_SOP("wrong solver_path", 0);


	//create new Bullet World
	m_bullet = new BWorld(	DELETE_THREAD()!=0,
							SHARE_SHAPES()!=0,
							ADV_defaultMaxPersistentManifoldPoolSize(),
							ADV_defaultMaxCollisionAlgorithmPoolSize(),

							ADV_btBroadphaseInterface(), 
							ADV_WORLD_CENTER(),
							ADV_WORLD_SIZE(),
							ADV_maxSmallProxies(),
							ADV_maxLargeProxies(),
							ADV_maxPairsPerBody(),
							ADV_maxBodiesPerCell(),
							ADV_cellFactorAABB(),

							ADV_btConstraintSolver(),
							ADV_solver_numThreads());

	m_foreign_bullet = false;	//this is original
}




OP_ERROR SOP_Solver::cookMySop(OP_Context &context)
{
	m_time = (float)context.getTime();
 	m_boss = SOPBoss( UTgetInterrupt() );
    if(lockInputs(context) >= UT_ERROR_ABORT)
		return error();
	OP_Node::flags().timeDep = 1;	//update every frame

	try
	{
		m_resetFrame = RESET_FRAME();
		int currframe = (int)OPgetDirector()->getChannelManager()->getSample(m_time);

		//get inputs
		m_input_prop = (GU_Detail*)inputGeo(0, context);
		m_input_geo = (GU_Detail*)inputGeo(1, context);
		m_input_const = (GU_Detail*)inputGeo(2, context);
		m_input_forc = (GU_Detail*)inputGeo(3, context);

		//recheck inputs
		if(m_input_geo)		if(m_input_geo->getPointMap().indexSize()==0)	m_input_geo = 0;
		if(m_input_const)	if(m_input_const->getPointMap().indexSize()==0)	m_input_const = 0;
		if(m_input_forc)	if(m_input_forc->getPointMap().indexSize()==0)	m_input_forc = 0;


		if(currframe <= m_resetFrame)	//delete solver or reset?
		{
			deleteBullet();
			if(currframe==m_resetFrame)	//reset
			{
				createBullet();	//create solver

				m_bullet->getCollisionFilter()->run(false);	//we need to turn off collisions on first frame, because we create groups after creating objects - call updateCollisionGroups() after updateObjects()
				updateObjects();
				updateConstraints();
				updateCollisionGroups();
				updateIgnoreCollisionGroups();
				m_bullet->getCollisionFilter()->run(true);
			}
		}


		if(!m_bullet)	//solver doesn't exist
		{
			if(OUTPUT()!=OUT_ZERO)
			{
				duplicateSource(0, context);	//copy 1st input to output

				if(DELETE_SUBCOMPOUNDS())
					deleteOuputSubcompounds(gdp);
			}
			THROW_SOP(0, "Solver doesn't exist! Go to reset_frame");
		}


		if(currframe > m_resetFrame)	//after reset frame - update objects
		{
			if( currframe < m_lastFrame )	//skip frame back
			{
				char t[255];	sprintf(t, "You step back before last frame: %d", m_lastFrame);
				THROW_SOP(0, t);
			}
			if( currframe==m_lastFrame && !SOLVE_ON_SAME_FRAME() )	// trying solve same frame again
			{
				char t[255];	sprintf(t, "You trying solve same frame again: %d", m_lastFrame);
				THROW_SOP(0, t);
			}


			if(GROUND_PLANE())	m_bullet->addFloor();
			else				m_bullet->removeFloor();

			//finally, It runs solving
			step();
		}


		if(OUTPUT()!=OUT_ZERO)
		{
			duplicateSource(0, context);	//copy first input to output
			if(DELETE_SUBCOMPOUNDS())
				deleteOuputSubcompounds(gdp);

			//output
			if(OUTPUT()==OUT_FULL)					setOutputFull(gdp, currframe>m_resetFrame);
			else
			if(OUTPUT()==OUT_TRANSFORM)				setOutputTransform(false, gdp, currframe>m_resetFrame);
			else
			if(OUTPUT()==OUT_TRANSFORM_VELOCITY)	setOutputTransform(true, gdp, currframe>m_resetFrame);
			else
			if(OUTPUT()==OUT_INSTANCE)				setOutputInstance(gdp, currframe>m_resetFrame);

		}

		backup(currframe);	//send data to SOP_Loader(s)

		m_lastFrame = currframe;


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




bool SOP_Solver::isImpactDataNeeded()
{
	for(int i=0; i < SOP_Loader::s_loaders.size(); i++)
	{
		SOP_Loader* l = SOP_Loader::s_loaders[i];
		if(l->isSolverNode(this) && l->LOAD_IMPACTS())
			return true;
	}
	return false;
}


void SOP_Solver::backup(int currframe)
{
	BOSS_START

	for(int i=0; i < SOP_Loader::s_loaders.size(); i++)
	{
		SOP_Loader* l = SOP_Loader::s_loaders[i];
		if(!l->isSolverNode(this))	//needs this loader some data from this solver?
			continue;

		l->deleteData();

	/*GEO*/
		if(l->LOAD_GEO())
		{
			if(m_input_geo)
				l->saveData(new GU_Detail(m_input_geo));
		}

		BOSS_INTERRUPT_EMPTY;	//interrupt

	/*PROPERTIES*/
		if(l->LOAD_PROPERTIES())
		{
			if(m_input_prop)
			{
				GU_Detail* prop;
				if(OUTPUT()==OUT_FULL && DELETE_SUBCOMPOUNDS()==0)
				{
					prop = new GU_Detail(gdp);
				}
				else
				{
					prop = new GU_Detail(m_input_prop);
					setOutputFull(prop, currframe > m_resetFrame);
				}

				l->saveData(prop);
			}
		}

		BOSS_INTERRUPT_EMPTY;	//interrupt

	/**FORCES*/
		if(l->LOAD_FORCES())
		{
			if(m_input_forc)
				l->saveData(new GU_Detail(m_input_forc));
		}

		BOSS_INTERRUPT_EMPTY;	//interrupt

	/*CONSTRAINTS*/
		if(l->LOAD_CONSTRAINTS())
		{
			if(m_input_const)
			{
				GU_Detail* constr = new GU_Detail(m_input_const);
				setOutputConstraint(constr);

				l->saveData(constr);
			}
		}

		BOSS_INTERRUPT_EMPTY;	//interrupt

	/*IMPACTS*/
		if(l->LOAD_IMPACTS())
		{
			if(m_bullet)
				l->saveData(new GU_Detail(m_bullet->getImpact()->getData()));
		}
	}


	BOSS_END;
}



void SOP_Solver::updateCollisionGroups()
{
	BOSS_START;

	SObject o;
	o.initFind(&m_boss, m_input_prop);

	BCollisionFilter* cf = m_bullet->getCollisionFilter();
	cf->clearGroups();

	UT_String group_name;
	COLLISION_GROUP_NAME(group_name);
	
	GA_PointGroup* curr;
	GA_FOR_ALL_POINTGROUPS(m_input_prop, curr)	//iterate over all groups
	{
		if(!curr->isEmpty())
		{
			UT_String name;
			name.harden(curr->getName());
			if(name.substitute(group_name, "") > 0)	//check If group has right name
			{
				if(name.isInteger())
				{
					int group_i = cf->addGroup();

					GA_Offset ptoff;
					GA_FOR_ALL_GROUP_PTOFF(m_input_prop, curr, ptoff)	//iterate over all points in group
					{
						int index = o.getIndex(ptoff);
						BRigidBody* body = m_bullet->getBody(index);
						if(!body)
						{
							BOSS_END;
							char t[255];	sprintf(t, "Object hasn't found: %d", index);
							addWarning(SOP_MESSAGE, t);
						}

						//printf("%d, ", index);
						cf->addProxy(group_i, body->getBroadphaseProxy());	//add "body" to group
					}
				}
			}
		}

		BOSS_INTERRUPT_EMPTY;	//set how many procent has done and check for "ESC"
	}

	BOSS_END;
}



void SOP_Solver::updateIgnoreCollisionGroups()
{
	BOSS_START;

	BCollisionFilter* cf = m_bullet->getCollisionFilter();
	cf->clearIgnoreGroups();

	if(!m_input_const)
		return;

	SConstraint cstr;
	cstr.initFind(&m_boss, m_input_const);

	GEO_Primitive* prim;
	int i=0;
	int num_prims = (int)m_input_const->getPrimitiveMap().indexSize();
	GA_FOR_ALL_PRIMITIVES(m_input_const, prim)	//iterate over all primitives(constraints)
	{
		GA_Offset primoff = prim->getMapOffset();

		int indexA = cstr.getIndexA(primoff);
		int indexB = cstr.getIndexB(primoff);
		BRigidBody* bodyA = m_bullet->getBody(indexA);
		BRigidBody* bodyB = m_bullet->getBody(indexB);

		if(!cstr.getCollision(primoff))
		{
			int group_i = cf->addIgnoreGroup();
			cf->addIgnoreProxy(group_i, bodyA->getBroadphaseProxy());
			cf->addIgnoreProxy(group_i, bodyB->getBroadphaseProxy());
		}

		BOSS_INTERRUPT(i, num_prims);
		i++;
	}

	cf->optimizeIgnoreGroups();	//delete duplicity pairs

	BOSS_END;
}








void SOP_Solver::setOutputConstraint(GU_Detail* gout)
{
	BOSS_START;

	SConstraint cstr;
	cstr.initFind(&m_boss, gout);


	GEO_PrimitivePtrArray arr;
	int i=0;
	int num_prims = (int)gout->getPrimitiveMap().indexSize();
	GEO_Primitive* prim;
	GA_FOR_ALL_PRIMITIVES(gout, prim)	//iterate over all primitives(constraints)
	{
		GA_Offset primoff = prim->getMapOffset();

		int index = cstr.getIndex(primoff);
		BRigidBody* bodyA = m_bullet->getBody( cstr.getIndexA(primoff) );
		BRigidBody* bodyB = m_bullet->getBody( cstr.getIndexB(primoff) );

		BConstraint* c = m_bullet->getConstraints(index, bodyA, bodyB);
		if(c)
		{
			cstr.setActualForce(primoff, c->getActForce());
			cstr.setPosA(prim, c->getWorldPositionA());
			cstr.setPosB(prim, c->getWorldPositionB());
		}
		else
			arr.append(prim);	//add to delete array


		BOSS_INTERRUPT(i, num_prims);
		i++;
	}
	gout->deletePrimitives(arr, true);	//delete primitives(and points), which are not in the world(broken, ...)

	BOSS_END;
}




void SOP_Solver::updateConstr(SConstraint* cstr, GEO_Primitive* prim)
{
	GA_Offset primoff = prim->getMapOffset();

	int index = cstr->getIndex(primoff);
	int indexA = cstr->getIndexA(primoff);
	int indexB = cstr->getIndexB(primoff);
	BRigidBody* bodyA = m_bullet->getBody(indexA);
	BRigidBody* bodyB = m_bullet->getBody(indexB);

	if(!bodyA || !bodyB)
	{
		char t[255];	sprintf(t, "Can't find body index(es) for constraint: %d", index);
		addWarning(SOP_MESSAGE, t);
		return;
	}


	bool update = cstr->getUpdate(primoff);
	bool recreate = cstr->getRecreate(primoff);
	BConstraint* c = m_bullet->getConstraints(index, bodyA, bodyB);
	if(!recreate && !update && c)
	{
		c->setUpdate(true);
		return;
	}

	if(m_bullet->hasConstraintBroken(index))	//If constraint was broken, don't update it( don't call setUpdate(true) ) => deletes it with call endUpdateConstraints()
		return;

	UT_Vector3 wpA = cstr->getPosA(prim);
	UT_Vector3 wpB = cstr->getPosB(prim);

	if(!c || recreate)
		c = m_bullet->addConstraints(index, bodyA, bodyB, wpA, wpB);

	if(c->isUpdated())
	{
		char t[255];	sprintf(t, "Constraint duplicity: %d", index);
		THROW_SOP(t, 0);
	}


	//update settings
	c->setLU( cstr->getLinupperlimit(primoff) );
	c->setLL( cstr->getLinlowerlimit(primoff) );
	c->setAU( cstr->getAngupperlimit(primoff)*DEG_TO_RAD);
	c->setAL( cstr->getAnglowerlimit(primoff)*DEG_TO_RAD );
	c->setAllAxisStiffness( cstr->getStiffness(primoff) );
	c->setAllAxisDamping( cstr->getDamping(primoff) );
	c->setBreaking( cstr->getMaxforce(primoff), cstr->getWeariness(primoff) );
	c->setIters( cstr->getIter(primoff) );
	c->setUpdate(true);
}

void SOP_Solver::updateConstraints()
{
	if(!m_input_const)
	{
		//delete all constraints
		m_bullet->startUpdateConstraints();
		m_bullet->endUpdateConstraints();
		return;
	}

	BOSS_START;

	SConstraint cstr;
	cstr.initFind(&m_boss, m_input_const);
	
	m_bullet->startUpdateConstraints();
	
	GEO_Primitive* prim;
	//first update constraint with the highest bt_index - only for speed => don't resize constraints array every step()
	GEO_Primitive* prim_max_index = 0;
	int max_index = 0;
	GA_FOR_ALL_PRIMITIVES(m_input_const, prim)	//find max_id constraint
	{
		int index = cstr.getIndex(prim->getMapOffset());
		if(index > max_index)
		{
			max_index = index;
			prim_max_index = prim;
		}
	}
	if(prim_max_index)
		updateConstr(&cstr, prim_max_index);

	m_bullet->startUpdateConstraints();	//again we need to setUpdate(FALSE) for last constraints(prim_max_index)!

	//update constraints
	int i=0;
	int num_prims = (int)m_input_const->getPrimitiveMap().indexSize();
	GA_FOR_ALL_PRIMITIVES(m_input_const, prim)	//iterate over all primitives(constraints)
	{
		updateConstr(&cstr, prim);

		BOSS_INTERRUPT(i, num_prims);
		i++;
	}

	m_bullet->endUpdateConstraints();

	BOSS_END;
}



void SOP_Solver::updateForces()
{
	if(!m_input_forc)
		return;

	BOSS_START;

	SForce f;
	f.initFind(&m_boss, m_input_forc);


	m_bullet->clearForces();

	int i=0;
	int num_points = (int)m_input_forc->getPointMap().indexSize();
	GA_Offset ptoff;
	GA_FOR_ALL_PTOFF(m_input_forc, ptoff)	//iterate over all points(forces)
	{
		UT_Vector3 pos = f.getPos(ptoff);
		int type = f.getType(ptoff);
		UT_Vector3 dir = f.getDir(ptoff);
		float size = f.getScale(ptoff);

		int index = f.getIndex(ptoff);
		float rad = f.getRad(ptoff);
		float fade = f.getFade(ptoff);

		dir.normalize();

		if(index >= 0)	//force on object
		{
			BRigidBody* body = m_bullet->getBody(index);
			if(!body)
			{
				char t[255];	sprintf(t, "Can't apply force on object: %d", index);
				addWarning(SOP_MESSAGE, t);
				continue;
			}
			else
				m_bullet->addObjectForce(body, pos, type, dir, size);
		}
		else	//force for all object(s) inside radius
			m_bullet->applyForceRad(pos, type, dir, size, rad, fade);


		BOSS_INTERRUPT(i, num_points);
		i++;
	}

	BOSS_END;
}




void SOP_Solver::updateObjects()
{
	SShape shape;
	SShape* shapeptr = 0;
	if(m_input_geo)
	{
		shape.initFind(&m_boss, m_input_geo, false);
		shapeptr = &shape;
	}

	SObject o;
	o.initFind(&m_boss, m_input_prop, shapeptr);

	m_bullet->startUpdateObjects();

		updateObjectsProperties(o, true);	//compounds
		updateObjectsProperties(o, false);	//others(non-compounds)

	m_bullet->endUpdateObjects();
}



void SOP_Solver::updateObjectsProperties(SObject &o, bool updateOnlyCompound)
{
	BOSS_START;

	int i = 0;
	int num_points = (int)m_input_prop->getPointMap().indexSize();

	GA_Offset ptoff;
	GA_FOR_ALL_PTOFF(m_input_prop, ptoff)	//iterate over all points(object properties)
	{
		int type = o.getType(ptoff);

		//what We want to update? compound or non-compound hull
		if(type==SOP_Build::TYPE_COMPOUND)
		{
			if(!updateOnlyCompound)
				continue;
		}
		else	//non-compounds
		{
			if(updateOnlyCompound)
				continue;
		}

		//get body
		int index = o.getIndex(ptoff);
		bool update = o.getUpdate(ptoff);
		bool recreate = o.getRecreate(ptoff);
		BRigidBody* body = m_bullet->getBody(index);


		//get compound(parent) body
		BRigidBody* cbody = 0;
		int cindex = o.getCIndex(ptoff);
		if(type==SOP_Build::TYPE_COMPOUND)
		{
			if(cindex > 0)
			{
				char t[255];	sprintf(t, "Compound object can't have parent: %d", index);
				THROW_SOP(t, 0);
			}
		}
		if(cindex > 0)
		{
			cbody = m_bullet->getBody(cindex);
			if(cbody)
			{
				if(!cbody->isCompound() || !cbody->isUpdated())	//isUpdated() => body will be deleted
				{
					char t[255];	sprintf(t, "Object parent isn't compound: %d", index);
					THROW_SOP(t, 0);
				}
			}
			else
			{
				char t[255];	sprintf(t, "Compound object doesn't exist: %d", index);
				THROW_SOP(t, 0);
			}
		}


		if(body)
		{
			//note: this need to be done, even for non-update properties, because otherwise can create crash!
			int old_cindex = body->getOldCIndex();
		
			if(old_cindex < 1 && cindex > 0)						m_bullet->convert_world_to_parent(body, cbody);
			else
			if(old_cindex > 0 && cindex < 1)						m_bullet->convert_parent_to_world(body);
			else
			if(old_cindex > 0 && cindex > 0 && old_cindex!=cindex)	m_bullet->convert_parent_to_parent(body, cbody);
		}

		if(!recreate && !update && body)
		{
			body->setUpdate(true);
			body->setLine(i);
			i++;
			continue;
		}


		//get some settings
		UT_Vector3 t = o.getPos(ptoff);
		UT_Vector3 r = o.getRotation(ptoff) * DEG_TO_RAD;
		UT_Vector3 cog_r = o.getCOGRotation(ptoff) * DEG_TO_RAD;
		UT_Vector3 av = o.getAV(ptoff) * DEG_TO_RAD;
		float mass = o.getMass(ptoff);
		if(type==SOP_Build::TYPE_DEFORM)
			mass = 0;


		bool vertexes = type==SOP_Build::TYPE_DEFORM || type==SOP_Build::TYPE_CONVEX;
		if(vertexes)
		{
			if(!o.getShape())
				THROW_SOP("Solver needs shape(s) - No data on 2nd input", 0);
		}


		if(recreate)
		{
			if(body && mass <= 0)	body = createObject(o, ptoff, body->getTranslateFull(), body->getRotateFull(), cog_r, index, type, cbody, vertexes);	//set old pos/rot for smooth aprox. to new position
			else					body = 0;
		}

		if(!body)
		{
			body = createObject(o, ptoff, t, r, cog_r, index, type, cbody, false);	//create object If doesn't exist
			if(!body)
				THROW_SOP("createObject() problem", 0);
		}

		if(body->isUpdated())
		{
			char t[255];	sprintf(t, "Object duplicity: %d", index);
			THROW_SOP(t, 0);
		}


		//set settings
		body->setMass(mass);
		body->setVelocity( o.getV(ptoff) );
		body->setAngVelocity(av);
		body->setRestitution( o.getBouncing(ptoff) );
		body->setFriction( o.getFriction(ptoff) );
		body->setCollisionMargin( o.getPadding(ptoff) );
		body->setDamping( o.getLinear_damping(ptoff), o.getAngular_damping(ptoff) );
		body->setSleeping( o.getSleeping(ptoff) );

		if(mass <= 0)
			body->setBodyTransform(t, r);	//must be after calling setMass()

		body->setUpdate(true);
		body->setLine(i);

		BOSS_INTERRUPT(i, num_points);
		i++;
	}


	BOSS_END;
}




//@return half of smaller value between X and Z axis
float getGroundRadius(UT_Vector3 bbox)
{
	if(bbox[0] > bbox[2])	return bbox[0] * 0.5f;
	else					return bbox[2] * 0.5f;
}

BRigidBody* SOP_Solver::createObject(const SObject &o, const GA_Offset &ptoff, UT_Vector3 t, UT_Vector3 r, UT_Vector3 cog_r, int index, int type, BRigidBody* compound, bool save_vertexes)
{
	UT_Vector3 bbox = o.getBBox(ptoff);
	float padding = o.getPadding(ptoff);

	if(type==SOP_Build::TYPE_CONVEX)
	{
		UT_Vector3 cog = o.getCOG(ptoff);
		return m_bullet->addConvex(index, t, r, cog_r, cog, o, compound, save_vertexes);
	}
	else
	if(type==SOP_Build::TYPE_SPHERE)
	{
		float rad = bbox.maxComponent() * 0.5f;
		return m_bullet->addSphere(index, t, r, cog_r, rad+padding, compound);
	}
	else
	if(type==SOP_Build::TYPE_BOX)
	{
		return m_bullet->addBox(index, t, r, cog_r, bbox*0.5f+padding, compound);
	}
	if(type==SOP_Build::TYPE_CAPSULE)
	{
		float rad = getGroundRadius(bbox);
		return m_bullet->addCapsule(index, t, r, cog_r, rad-8*padding, bbox[1] - 2*rad, compound);
	}
	else
	if(type==SOP_Build::TYPE_CONE)
	{
		float rad = getGroundRadius(bbox);
		return m_bullet->addCone(index, t, r, cog_r, rad, bbox[1]-padding, compound);
	}
	else
	if(type==SOP_Build::TYPE_CYLINDER)
	{
		float rad = getGroundRadius(bbox);
		return m_bullet->addCylinder(index, t, r, cog_r, rad, bbox[1], compound);
	}
	else
	if(type==SOP_Build::TYPE_DEFORM)
	{
		UT_Vector3 cog = o.getCOG(ptoff);
		return m_bullet->addDeform(index, t, r, cog_r, cog, o, compound, save_vertexes);
	}
	if(type==SOP_Build::TYPE_COMPOUND)
	{
		return m_bullet->addCompound(index, t, r, cog_r, compound);
	}
	else
	{
		char t[255];	sprintf(t, "Unknow type of geometry object(%d): %d", index, type);
		THROW_SOP(t, 0);
	}

	return 0;
}




void SOP_Solver::step()
{
	BOSS_START;

	//update
	updateObjects();
	updateConstraints();
	updateCollisionGroups();
	updateIgnoreCollisionGroups();


	//turn on or off creating impact data
	m_bullet->getImpact()->resetData();
	bool impactNeed = isImpactDataNeeded();
	m_bullet->getImpact()->setRun(impactNeed);

	m_bullet->clearForces();

	BOSS_INTERRUPT(2, 100);

	const float timestep = 1.0f/(float)FPS();
	const float stepSize = timestep / (float)SUBSTEPS();
	for(int i=0; i < SUBSTEPS(); i++)
	{
		if(impactNeed)
			m_bullet->backUpVelocities();

		if(i==0 || FORCE_SUBSTEPPING())
			updateForces();

		if(i==0 || CONSTRAINT_SUBSTEPPING())
			m_bullet->breakConstraints(FPS(), SUBSTEPS());

		m_bullet->setGravity(GFORCE());
		m_bullet->stepWorld(stepSize, i, SUBSTEPS());

		BOSS_INTERRUPT(i+1, SUBSTEPS());
	}


	BOSS_END;
}



void SOP_Solver::deleteOuputSubcompounds(GU_Detail* gout)
{
	SObject o;
	o.initFind(&m_boss, gout);
	o.deleteOuputSubcompounds();
}


void SOP_Solver::setOutputFull(GU_Detail* gout, bool update)
{
	if(!update)
		return;

	BOSS_START;

	SObject o;
	o.initFind(&m_boss, gout);

	int num_points = (int)gout->getPointMap().indexSize();
	int i=0;
	GA_Offset ptoff;
	GA_FOR_ALL_PTOFF(gout, ptoff)
	{
		int index = o.getIndex(ptoff);
		BRigidBody* body = m_bullet->getBody(index);
		if(body)
		{
			UT_Vector3 t = body->getTranslateWithoutInitRot();
			UT_Vector3 r = body->getRotateWithoutInitRot() * RAD_TO_DEG;
			UT_Vector3 v = body->getVelocity();
			UT_Vector3 av = body->getAngVelocity() * RAD_TO_DEG;

			o.setPos(ptoff, t);
			o.setRotation(ptoff, r);
			o.setLinearVelocity(ptoff, v);
			o.setAlgularVelocity(ptoff, av);
		}
		else
		{
			BOSS_END;
			char t[255];	sprintf(t, "Can't find object: %d", index);
			addWarning(SOP_MESSAGE, t);
			continue;
		}

		BOSS_INTERRUPT(i, num_points);
		i++;
	}

	BOSS_END;
}

void SOP_Solver::setOutputTransform(bool add_velocity, GU_Detail* gout, bool update)
{
	SObject o;
	o.initFind(&m_boss, gout);

	//deletes some attributes
	if(add_velocity)	o.destroyForTransformVelocityOutput();
	else				o.destroyForTransformOutput();

	if(!update)
		return;

	BOSS_START;

	int num_points = (int)gout->getPointMap().indexSize();
	int i=0;
	GA_Offset ptoff;
	GA_FOR_ALL_PTOFF(gout, ptoff)
	{
		int index = o.getIndex(ptoff);

		BRigidBody* body = m_bullet->getBody(index);
		if(body)
		{
			UT_Vector3 t = body->getTranslateWithoutInitRot();
			UT_Vector3 r = body->getRotateWithoutInitRot() * RAD_TO_DEG;

			if(add_velocity)
			{
				UT_Vector3 v = body->getVelocity();
				UT_Vector3 av = body->getAngVelocity();
				av *= RAD_TO_DEG;

				o.setPos(ptoff, t);
				o.setRotation(ptoff, r);
				o.setLinearVelocity(ptoff, v);
				o.setAlgularVelocity(ptoff, av);
			}
			else
			{
				o.setPos(ptoff, t);
				o.setRotation(ptoff, r);
			}
		}
		else
		{
			BOSS_END;
			char t[255];	sprintf(t, "Can't find object: %d", index);
			addWarning(SOP_MESSAGE, t);
			continue;
		}

		BOSS_INTERRUPT(i, num_points);
		i++;
	}

	BOSS_END;
}


void SOP_Solver::setOutputInstance(GU_Detail* gout, bool update)
{
	BOSS_START;

	SObject o;
	o.initFind(&m_boss, gout);

	//add new attributes
	GA_RWHandleV3 attr_N = SHelper::addFloatVectorPointAttr(gout, "N");
	GA_RWHandleV3 attr_up = SHelper::addFloatVectorPointAttr(gout, "up");

	UT_Matrix4 mat;	UT_XformOrder order(UT_XformOrder::TSR, UT_XformOrder::XYZ);
	int num_points = (int)gout->getPointMap().indexSize();
	int i=0;
	GA_Offset ptoff;
	GA_FOR_ALL_PTOFF(gout, ptoff)
	{
		int index = o.getIndex(ptoff);

		BRigidBody* body = m_bullet->getBody(index);
		if(body)
		{
			UT_Vector3 t = body->getTranslateFull();
			UT_Vector3 r = body->getRotateFull();

			//create rotation matrix
			mat = UT_Matrix4::getIdentityMatrix();
			mat.rotate(r[0], r[1], r[2], order);

			o.setPos(ptoff, t);

			//set rotated axis
			attr_N.set(ptoff, UT_Vector3(0, 0, 1)*mat);
			attr_up.set(ptoff, UT_Vector3(0, 1, 0)*mat);
		}
		else
		{
			BOSS_END;
			char ttt[255];	sprintf(ttt, "Can't find object: %d", index);
			addWarning(SOP_MESSAGE, ttt);
			continue;
		}

		BOSS_INTERRUPT(i, num_points);
		i++;
	}


	//deletes some attributes
	o.destroyForInstanceOutput();

	//delete attribute
	gout->destroyAttribute(GA_ATTRIB_POINT, "bt_index");

	BOSS_END;
}




const char *SOP_Solver::inputLabel(unsigned inum) const
{
    switch (inum)
    {
		case 0: return "Physics Properties";
		case 1:	return "Shapes(Geometry)";
		case 2:	return "Constraints";
		case 3:	return "Forces";
    }
    return "Unknown source";
}


bool SOP_Solver::updateParmsFlags()
{
	bool changed = SOP_Node::updateParmsFlags();

	changed |= enableParm("worldCenter", ADV_btBroadphaseInterface()>0);
	changed |= enableParm("worldSize", ADV_btBroadphaseInterface()>0);
	changed |= enableParm("maxSmallProxies", ADV_btBroadphaseInterface()>0);

	changed |= enableParm("maxLargeProxies", ADV_btBroadphaseInterface()>1);
	changed |= enableParm("maxPairsPerBody", ADV_btBroadphaseInterface()>1);
	changed |= enableParm("maxBodiesPerCell", ADV_btBroadphaseInterface()>1);
	changed |= enableParm("cellFactorAABB", ADV_btBroadphaseInterface()>1);

	changed |= enableParm("solver_numThreads", ADV_btConstraintSolver()==1);

	return changed;
}


OP_Node *SOP_Solver::myConstructor(OP_Network *net, const char *name, OP_Operator *op)
{
    return new SOP_Solver(net, name, op);
}




static PRM_Name adv_btBroadphaseInterface[] = {
	PRM_Name("0",	"btDbvtBroadphase"),
	PRM_Name("1",	"bt32BitAxisSweep3"),
	PRM_Name("2",	"btGpu3DGridBroadphase"),
	PRM_Name(0)
};
static PRM_Name adv_btConstraintSolver[] = {
	PRM_Name("0",	"btSequentialImpulseConstraintSolver"),
	PRM_Name("1",	"btParallelConstraintSolver"),
	PRM_Name("2",	"btMLCPSolver"),
	PRM_Name(0)
};

static PRM_Name output[] = {
	PRM_Name("0",	"Full"),
	PRM_Name("1",	"Transform"),
	PRM_Name("2",	"Transform_Velocity"),
	PRM_Name("3",	"Instance"),
	PRM_Name("4",	"Zero"),
	PRM_Name(0)
};

static PRM_ChoiceList   adv_btBroadphaseInterfaceMenu((PRM_ChoiceListType)(PRM_CHOICELIST_SINGLE), &(adv_btBroadphaseInterface[0]) );
static PRM_ChoiceList   adv_btConstraintSolverMenu((PRM_ChoiceListType)(PRM_CHOICELIST_SINGLE), &(adv_btConstraintSolver[0]) );

static PRM_ChoiceList   outputMenu((PRM_ChoiceListType)(PRM_CHOICELIST_SINGLE), &(output[0]) );




static PRM_Name names[] = {
    PRM_Name("tabs",	"tabs"),

	//basic
    PRM_Name("solver_path",	"Solver Path"),
	PRM_Name("reset_frame",	"Reset Frame"),

	PRM_Name("sep0", "sep0"),

	PRM_Name("solve_on_same_frame", "Solve On Same Frame"),
    PRM_Name("substeps", "Substeps"),
	PRM_Name("fps", "Fps"),
    PRM_Name("gforce",	"Gravity"),

	PRM_Name("sep1", "sep1"),

	PRM_Name("ground_plane", "Ground Plane"),

	PRM_Name("sep2", "sep2"),

	PRM_Name("share_shapes", "Share Shapes"),

	PRM_Name("sep3", "sep3"),

	PRM_Name("collision_group_name",	"Collision Group Name"),

	PRM_Name("sep4", "sep4"),

	PRM_Name("force_substepping",	"Force Substepping"),
	PRM_Name("constraint_substepping",	"Constraint Substepping"),

	PRM_Name("sep5", "sep5"),

	PRM_Name("output", "Output"),
	PRM_Name("delete_subcompounds", "Delete SubCompound Objects"),

	PRM_Name("sep6", "sep6"),

	PRM_Name("delete_thread", "Free World In Extra Thread"),

};


static PRM_Name names2[] = {
	//advanced
	PRM_Name("btBroadphaseInterface", "btBroadphaseInterface"),

	PRM_Name("worldCenter", "worldCenter"),
	PRM_Name("worldSize", "worldSize"),
	PRM_Name("maxSmallProxies", "maxSmallProxies"),
	PRM_Name("maxLargeProxies", "maxLargeProxies"),

	PRM_Name("maxPairsPerBody", "maxPairsPerBody"),
	PRM_Name("maxBodiesPerCell", "maxBodiesPerCell"),
	PRM_Name("cellFactorAABB", "cellFactorAABB"),

	PRM_Name("sep7", "sep7"),

	PRM_Name("btConstraintSolver", "btConstraintSolver"),
	PRM_Name("solver_numThreads", "solver_numThreads"),

	PRM_Name("sep8", "sep8"),

	PRM_Name("defaultMaxPersistentManifoldPoolSize", "MaxPersistentManifoldPoolSize"),
	PRM_Name("defaultMaxCollisionAlgorithmPoolSize", "MaxCollisionAlgorithmPoolSize"),
};









static PRM_Default  tabs[] = {
	PRM_Default(21, "Basic"),
    PRM_Default(14, "Advanced"),
};


//basic
static PRM_Default	substeps(10);
static PRM_Default	reset(2);
static PRM_Default	fps(0, "$FPS");
static PRM_Default gforce[] = {PRM_Default(0), PRM_Default(-9.81), PRM_Default(0)};
static PRM_Default	col_group_name(0, "bt_collide");

//advanced
static PRM_Default collisionConfiguration(4096);
static PRM_Default solver_numThreads(4);
static PRM_Default size[] = {PRM_Default(100), PRM_Default(100), PRM_Default(100)};

static PRM_Default broadphase_maxSmallProxies(100000);
static PRM_Default broadphase_maxLargeProxies(10);
static PRM_Default broadphase_maxPairsPerBody(24);
static PRM_Default broadphase_maxBodiesPerCell(8);




static PRM_Name g_name_info("tool_name", TOOL_NAME);

PRM_Template SOP_Solver::myTemplateList[] = {
	PRM_Template(PRM_HEADING ,1, &g_name_info, 0),

	PRM_Template(PRM_SWITCHER, 2, &names[0], tabs),

		//basic
		PRM_Template(PRM_STRING_E, PRM_TYPE_DYNAMIC_PATH,	1, &names[1], 0, 0, 0, 0, &PRM_SpareData::sopPath),
		PRM_Template(PRM_INT_E,	1, &names[2], &reset),

		PRM_Template(PRM_SEPARATOR, 1, &names[3]),

		PRM_Template(PRM_TOGGLE,1, &names[4], 0),
		PRM_Template(PRM_INT_J,	1, &names[5], &substeps),
		PRM_Template(PRM_INT_J,	1, &names[6], &fps),
		PRM_Template(PRM_XYZ_J,	3, &names[7], gforce),

		PRM_Template(PRM_SEPARATOR, 1, &names[8]),

		PRM_Template(PRM_TOGGLE,1, &names[9]),

		PRM_Template(PRM_SEPARATOR, 1, &names[10]),

		PRM_Template(PRM_TOGGLE,1, &names[11], PRMoneDefaults),

		PRM_Template(PRM_SEPARATOR, 1, &names[12]),

		PRM_Template(PRM_STRING,	1, &names[13], &col_group_name),

		PRM_Template(PRM_SEPARATOR, 1, &names[14]),

		PRM_Template(PRM_TOGGLE,1, &names[15]),
		PRM_Template(PRM_TOGGLE,1, &names[16]),

		PRM_Template(PRM_SEPARATOR, 1, &names[17]),

		PRM_Template(PRM_INT,	1, &names[18], PRMoneDefaults, &outputMenu),
		PRM_Template(PRM_TOGGLE,	1, &names[19], PRMoneDefaults),
	
		PRM_Template(PRM_SEPARATOR, 1, &names[20]),

		PRM_Template(PRM_TOGGLE_E,	1, &names[21], PRMoneDefaults),
		

		//advanced
		PRM_Template(PRM_INT_E,	1, &names2[0], 0, &adv_btBroadphaseInterfaceMenu),
		PRM_Template(PRM_XYZ_E,	3, &names2[1]),
		PRM_Template(PRM_XYZ_E,	3, &names2[2], size),
		PRM_Template(PRM_INT_E,	1, &names2[3], &broadphase_maxSmallProxies),
		PRM_Template(PRM_INT_E,	1, &names2[4], &broadphase_maxLargeProxies),
		PRM_Template(PRM_INT_E,	1, &names2[5], &broadphase_maxPairsPerBody),
		PRM_Template(PRM_INT_E,	1, &names2[6], &broadphase_maxBodiesPerCell),
		PRM_Template(PRM_FLT_E,	1, &names2[7], PRMoneDefaults),

		PRM_Template(PRM_SEPARATOR, 1, &names2[8]),

		PRM_Template(PRM_INT_E,	1, &names2[9], 0, &adv_btConstraintSolverMenu),
		PRM_Template(PRM_INT_E,	1, &names2[10], &solver_numThreads),

		PRM_Template(PRM_SEPARATOR, 1, &names2[11]),

		PRM_Template(PRM_INT_E,	1, &names2[12], &collisionConfiguration),
		PRM_Template(PRM_INT_E,	1, &names2[13], &collisionConfiguration),

	PRM_Template()
};

