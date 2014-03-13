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
SShape inherit from SHelper.
SShape divides geometry to objects with "bt_index" and bt_cindex" attributes
There are few functions for dividing base on number of copies, or group prefix.
There are few functions for geometry decompoisition as well.
*/
class SShape : public SHelper
{
private:
	GA_RWHandleI m_attr_bt_index_point;
	GA_RWHandleI m_attr_bt_index_prim;

	GA_RWHandleI m_attr_bt_max_index;
	GA_RWHandleF m_attr_bt_padding;
	GA_RWHandleV3 m_attr_bt_bbox;

	GA_RWHandleI m_attr_bt_cindex_point;
	GA_RWHandleI m_attr_bt_cindex_prim;

	GA_RWHandleV3 m_attr_N;
	GA_RWHandleV3 m_attr_Up;


public:
	SShape(void);
	virtual ~SShape(void);


public:
	void init(SOPBoss* boss, GU_Detail* gdp, bool add_cindex);				///creates attribute structure
	void initFind(SOPBoss* boss, GU_Detail* gdp, bool detailAttrs=true);	///tries to find attributes in gdp otherwise adds error and @return false

	void setPointIndex(const GA_Offset &off, int i);
	void setPrimitiveIndex(const GA_Offset &off, int i);
	void setPointCIndex(const GA_Offset &off, int i);
	void setPrimitiveCIndex(const GA_Offset &off, int i);

	int getPointIndex(const GA_Offset &off) const;
	int getPrimitiveIndex(const GA_Offset &off) const;
	int getPointCIndex(const GA_Offset &off) const;
	int getPrimitiveCIndex(const GA_Offset &off) const;

	UT_Vector3 getN(const GA_Offset &off) const;
	UT_Vector3 getUp(const GA_Offset &off) const;

	bool hasCompoundIndexAttr() const;	///exists bt_cindex attribute for point && primitive
	bool hasNandUpAttribs() const;		///exists N && Up attribute

	int updateMaxIndex();

	int getMaxIndex() const;		//@return the highest value of "bt_index" or "bt_cindex" attribute
	float getPadding() const;
	UT_Vector3 getBBox() const;

	void setMaxIndex(int max);
	void setPadding(float padding);
	void setBBox(const UT_Vector3 bbox);

	void fillAtrributeGroup(int start_i, UT_String group_name, bool deletePieceGroups);	///sets bt_index according groups
	void fillAtrributeCopyPoints(int start_i, int copy);								///sets point bt_index according copies
	void fillAtrributeCopyPrimitives(int start_i, int copy);							///sets primitive bt_index according copies

	///put geometry objects to compound
	///num - number of object for processing to one compound
	///start_index - the lowest value in bt_index
	void setCompounds(int num, int start_index);

	///put geometry objects to compound - first object in group(parmeter num) is compound geo, other(num-1) objects are inside compound
	///num - number of object for processing to one compound
	///start_index - the lowest value in bt_index
	void setCompoundsFirst(int num, int start_index);


	void shiftGeo(GU_Detail* original, float padding, int normal_type);	///moves geometry inside(outside) according normal and padding

	void convertToHighPoly();	///revers bt_index ... 2 => -2

	void createConvexGroup(bool advance, float toler);	///creates convex and concave groups(point, primitives)

	void decompositionTetra(int raySteps, bool includeCompoundGeo);						///break concave objects to tetraHedras
	void decompositionVoxel(float cellSize, float padding, bool includeCompoundGeo);	///replace concave objects with "3d grid of points"(voxels) which have minimum distance to object higher than padding+cellSize

	GU_Detail* getGeo();
	int getNumPoints();
	int getNumPrims();

	void deleteHigh();			///deletes points/primitives which have  bt_index >= 0
	void deleteLow();			///deletes points/primitives which have  bt_index <= 0
	void deleteSubCompund();	///deletes points/primitives which have  bt_cindex > 0
};
