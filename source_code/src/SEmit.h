/**
BulletSOP v2.0
Copyright (c)2010-2013 Suk Milan. All Rights Reserved.
This software is provided 'as-is', without any express or implied warranty.
In no event will the authors be held liable for any damages arising from the use of this software.
This file may not be redistributed in whole or significant part.
This notice may not be removed or altered from any source distribution.
*/
#pragma once
#include "SHelper.h"


/**
SEmit inherit from SHelper.
SEmit creates attribute structure and store data the closest primitive and break time.
*/
class SEmit : public SHelper
{
private:
	GA_RWHandleF m_attr_bt_emit_connect_dist;
	GA_RWHandleF m_attr_bt_emit_break_dist;
	GA_RWHandleI m_attr_bt_emit_frame;

	static int emit_id;

public:
	SEmit(void);
	virtual ~SEmit(void);

public:
	void init(SOPBoss* boss, GU_Detail* gdp);		///creates attribute structure
	void initFind(SOPBoss* boss, GU_Detail* gdp);	///tries to find attributes in gdp otherwise adds error and @return false

	void addToGroup(GA_PrimitiveGroup* group);	///adds all forces(points) to group
	GU_Detail* getGeo() const;

	void buildEmitGroups(float memory);
	void setFrameFromGroups(int frame);

	float getConnectDistance(const GA_Offset &off) const;
	float getBreakDistance(const GA_Offset &off) const;
	int getFrame(const GA_Offset &off) const;

	void setConnectDistance(const GA_Offset &off, float v);
	void setBreakDistance(const GA_Offset &off, float v);
	void setFrame(const GA_Offset &off, int i);

	void setAllBreakDistance(float dist);
	void setAllConnectDistance(float dist);
};
