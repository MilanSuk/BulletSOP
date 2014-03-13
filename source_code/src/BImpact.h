/**
BulletSOP v2.0
Copyright (c)2010-2013 Suk Milan. All Rights Reserved.
This software is provided 'as-is', without any express or implied warranty.
In no event will the authors be held liable for any damages arising from the use of this software.
This file may not be redistributed in whole or significant part.
This notice may not be removed or altered from any source distribution.
*/
#pragma once

class btDiscreteDynamicsWorld;
class SImpact;


/**
Class creates ouput data from impacts information between objects
*/
class BImpact
{
private:
	btDiscreteDynamicsWorld* m_dynamicsWorld;

	SOPBoss m_boss;
	GU_Detail* m_gdp;
	SImpact* m_impact;
	bool m_run;

public:
	BImpact(btDiscreteDynamicsWorld* dynamicsWorld);
	virtual ~BImpact(void);

	void setRun(bool v);
	bool isRun() const;

	SImpact* getImpact() const;
	GU_Detail* getData() const;

	void resetData();

private:
	void createData();
	void clearData();

};
