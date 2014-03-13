/**
BulletSOP v2.0
Copyright (c)2010-2013 Suk Milan. All Rights Reserved.
This software is provided 'as-is', without any express or implied warranty.
In no event will the authors be held liable for any damages arising from the use of this software.
This file may not be redistributed in whole or significant part.
This notice may not be removed or altered from any source distribution.
*/
#pragma once

#include "btBulletDynamicsCommon.h"


class BConstraint : public btGeneric6DofSpringConstraint
{
private:
	bool m_update;
	int m_hId;

	bool m_weariness;
	float m_actForce;
	float m_maxForce;

public:
	BConstraint(int index, btRigidBody& rbA, btRigidBody& rbB, const btTransform& frameInA, const btTransform& frameInB ,bool useLinearReferenceFrameA);

	void resetEquilibrium();

	void setLU(UT_Vector3 limit_h);
	void setLL(UT_Vector3 limit_l);
	void setAL(UT_Vector3 limit_al);
	void setAU(UT_Vector3 limit_ah);
	void setAllAxisStiffness(float stiffness);
	void setAllAxisDamping(float damping);
	void setBreaking(float maxforce, bool weariness);
	void setIters(int n);
	void setUpdate(bool update);

	bool isThis(btRigidBody *body_1, btRigidBody *body_2) const;
	bool isUpdated() const;
	UT_Vector3 getWorldPositionA() const;
	UT_Vector3 getWorldPositionB() const;
	float getActForce() const;
	int getIndex() const;

	///@return true If constraint needs to be break(remove from world)
	bool tryBreakIt(int fps, int substeps);
};

