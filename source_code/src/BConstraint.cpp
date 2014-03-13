/**
BulletSOP v2.0
Copyright (c)2010-2013 Suk Milan. All Rights Reserved.
This software is provided 'as-is', without any express or implied warranty.
In no event will the authors be held liable for any damages arising from the use of this software.
This file may not be redistributed in whole or significant part.
This notice may not be removed or altered from any source distribution.
*/
#include "stdafx.h"
#include "BConstraint.h"
#include "BWorld.h"


BConstraint::BConstraint(int index, btRigidBody& rbA, btRigidBody& rbB, const btTransform& frameInA, const btTransform& frameInB ,bool useLinearReferenceFrameA)
: btGeneric6DofSpringConstraint(rbA, rbB, frameInA, frameInB, useLinearReferenceFrameA)
{
	m_hId = index;
	m_update = false;

	m_weariness = false;
	m_actForce = 0;
	m_maxForce = 0;
}

void BConstraint::resetEquilibrium()
{
	setEquilibriumPoint();
}

bool BConstraint::isThis(btRigidBody *body_1, btRigidBody *body_2) const
{
	if(&getRigidBodyA()==body_1 && &getRigidBodyB()==body_2)
		return true;
	return false;
}

void BConstraint::setBreaking(float maxforce, bool weariness)
{
	m_maxForce = maxforce;
	m_weariness = weariness;
}

void BConstraint::setIters(int n)
{
	setOverrideNumSolverIterations(n);
}

void BConstraint::setLU(UT_Vector3 limit_h)
{
	setLinearUpperLimit( get_bullet_V3(limit_h) );
}
void BConstraint::setLL(UT_Vector3 limit_l)
{
	setLinearLowerLimit( get_bullet_V3(limit_l) );
}

void BConstraint::setAL(UT_Vector3 limit_al)
{
	setAngularLowerLimit( get_bullet_V3(limit_al) );
}
void BConstraint::setAU(UT_Vector3 limit_ah)
{
	setAngularUpperLimit( get_bullet_V3(limit_ah) );
}

void BConstraint::setAllAxisStiffness(float stiffness)
{
	for(int i = 0; i < 6; i++)
		enableSpring(i, stiffness > 0);

	for(int i = 0; i < 6; i++)
		setStiffness(i, stiffness);
}

void BConstraint::setAllAxisDamping(float damping)
{
	for(int i = 0; i < 6; i++)
		setDamping(i, damping);
}

void BConstraint::setUpdate(bool update)
{
	m_update = update;
}

bool BConstraint::isUpdated() const
{
	return m_update;
}

int BConstraint::getIndex() const
{
	return m_hId;
}

UT_Vector3 BConstraint::getWorldPositionA() const
{
	return get_hdk_V3( getCalculatedTransformA().getOrigin() );
}

UT_Vector3 BConstraint::getWorldPositionB() const
{
	return get_hdk_V3( getCalculatedTransformB().getOrigin() );
}

bool BConstraint::tryBreakIt(int fps, int substeps)
{
	btTranslationalLimitMotor *m_linearLimits = getTranslationalLimitMotor();

	float force_square = m_linearLimits->m_maxMotorForce.dot(m_linearLimits->m_maxMotorForce);
	float constr_force = sqrt(force_square)*fps;

	if(m_weariness)	m_actForce += constr_force/substeps;	//adds force to old force
	else			m_actForce = constr_force;

	if(m_maxForce <= 0)
		return false;

	if(m_actForce > m_maxForce)
		return true;	//break the constraint

	return false;
}

float BConstraint::getActForce() const
{
	return m_actForce;
}


