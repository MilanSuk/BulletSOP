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

class SShape;


/**
SObject inherit from SHelper.
SObject computes properties from geometry(SShape)
SObject includes function for computing COG, Volume, ConvexTest, ...
*/
class SObject : public SHelper
{
private:
	SShape* m_input;

	GA_RWHandleI m_attr_bt_index;
	GA_RWHandleI m_attr_bt_cindex;
	GA_RWHandleV3 m_attr_bt_cog;
	GA_RWHandleV3 m_attr_bt_cog_r;
	GA_RWHandleV3 m_attr_bt_r;
	GA_RWHandleF m_attr_bt_mass;
	GA_RWHandleF m_attr_bt_restitution;
	GA_RWHandleF m_attr_bt_friction;
	GA_RWHandleF m_attr_bt_padding;
	GA_RWHandleI m_attr_bt_type;
	GA_RWHandleV3 m_attr_bt_boxsize;
	GA_RWHandleV3 m_attr_bt_v;
	GA_RWHandleV3 m_attr_bt_av;
	GA_RWHandleF m_attr_bt_lin_damp;
	GA_RWHandleF m_attr_bt_ang_damp;
	GA_RWHandleI m_attr_bt_sleeping;
	GA_RWHandleI m_attr_bt_update;
	GA_RWHandleI m_attr_bt_recreate;

	GA_RWHandleI m_attr_bt_max_obj_index;

private:
	class Index
	{
	public:
		MyVec<GA_Offset> geo_pts;
		MyVec<GEO_Primitive*> geo_prims;

		bool exist;
		int pos;
		GA_Offset ptoff;
		bool convex;
		bool parent;

		Index()
		{
			exist = false;
			pos = -1;
			ptoff = 0;
			convex = true;
			parent = false;

			geo_pts.setExtraAlloc(EXTRA_ALLOC_SMALL);
			geo_prims.setExtraAlloc(EXTRA_ALLOC_SMALL);
		}

		void addPoint(GA_Offset ptoff)
		{
			geo_pts.push_back(ptoff);
		}

		void addPrimitive(GEO_Primitive* pr)
		{
			geo_prims.push_back(pr);
		}

		const MyVec<GA_Offset>& getPoints() const
		{
			return geo_pts;
		}
		const MyVec<GEO_Primitive*>& getPrimitives() const
		{
			return geo_prims;
		}

		GA_Offset getPoint(int i)
		{
			return geo_pts[i];
		}
		GEO_Primitive* getPrimitive(int i)
		{
			return geo_prims[i];
		}

		int getPointSize()
		{
			return (int)getPoints().size();
		}
		int getPrimitiveSize()
		{
			return (int)getPrimitives().size();
		}
	};
	MyVec<Index> m_indexes;


public:
	SObject(void);
	virtual ~SObject(void);


public:
	void init(SOPBoss* boss, GU_Detail* gdp, SShape* input=NULL);					///creates attribute structure
	void initFind(SOPBoss* boss, GU_Detail* gdp, SShape* input=NULL);				///tries to find attributes in gdp otherwise adds error and @return false
	void initFind_TransformMinimum(SOPBoss* boss, GU_Detail* gdp, bool velocity);	///tries to find primary attributes for object transformation in gdp otherwise adds error and @return false

	SShape* getShape() const;	///returns m_input

	size_t getNumIndexes() const;
	bool isIndexExist(int index) const;	///is index in range of m_indexes vector

	void deleteOuputSubcompounds();	///delete all points which have cindex > 0

	void copyPrimAttrs();	///copies geometry primitive attributes to points(properties)

	void computeBoxCOG();		///computes Center of gravity from BBox for every objects
	void computeAverageCOG();	///computes Center of gravity from position arithmetic average for every objects
	void computeVolumeCOG();	///computes True Center of gravity for every objects
	void computeMassFromVolume(float density);	///computes Volume mass for every objects
	void computeCogRotation();	///computes init rotation from Up and N vector for every objects
	void computeBBox();			///computes BBox for every objects
	void computeConvex(bool advance, float toler, bool recomputeVolumeCOG);	///computes If object is convex for every objects

	void destroyForTransformOutput();			///removes all atributes except transform
	void destroyForTransformVelocityOutput();	///removes all atributes except transform and velocity
	void destroyForInstanceOutput();			///removes all atributes, which are not needed for instancing

	bool isConvexBasic(int index);					//checks If COG of object(i) is at inside side of every primitives(planes)
	bool isConvexAdvance(int index, float toler);	//checks If angle between every primitives is acute(sharp)

	void setAllUpdate(bool update);
	void setAllRecreate(bool update);
	void setAllMass(float mass);
	void setAllPadding(float padding);		///sets padding value for all points
	void setAllNotCompoundType(int type);	///sets type for all non-compound objects

	void updateMaxObjIndex();	///computes new max index and set it
	void copyCOGtoPos();	///copies COG attribute to P attribute
	void copyCOGRotToRot();	///copies bt_cog_r attribute to bt_t attribute
	void addToGroup(GA_PointGroup* group);	///adds all points to groups


	void updateAllCompoundMass();

	void setIndex(const GA_Offset &off, int i);
	void setCIndex(const GA_Offset &off, int i);
	void setPos(const GA_Offset &off, UT_Vector3 v);
	void setCOG(const GA_Offset &off, UT_Vector3 v);
	void setCOGRotation(const GA_Offset &off, UT_Vector3 v);
	void setMass(const GA_Offset &off, float mass);
	void setBBox(const GA_Offset &off, UT_Vector3 v);
	void setType(const GA_Offset &off, int i);
	void setRestitution(const GA_Offset &off, float v);
	void setFriction(const GA_Offset &off, float v);
	void setPadding(const GA_Offset &off, float v);
	void setLinearDamping(const GA_Offset &off, float v);
	void setAlgularDamping(const GA_Offset &off, float v);
	void setRotation(const GA_Offset &off, UT_Vector3 v);
	void setLinearVelocity(const GA_Offset &off, UT_Vector3 v);
	void setAlgularVelocity(const GA_Offset &off, UT_Vector3 v);
	void setSleep(const GA_Offset &off, int i);
	void setUpdate(const GA_Offset &off, int i);
	void setRecreate(const GA_Offset &off, int i);

	UT_Vector3 getCOG(const GA_Offset &off) const;
	UT_Vector3 getCOGRotation(const GA_Offset &off) const;
	int getIndex(const GA_Offset &off) const;
	int getCIndex(const GA_Offset &off) const;
	UT_Vector3 getRotation(const GA_Offset &off) const;
	UT_Vector3 getV(const GA_Offset &off) const;
	UT_Vector3 getAV(const GA_Offset &off) const;
	UT_Matrix4 getRotationMatrix(const GA_Offset &off) const;
	UT_Matrix4 getTranformationMatrix(const GA_Offset &off) const;
	bool getUpdate(const GA_Offset &off) const;
	bool getRecreate(const GA_Offset &off) const;
	int getType(const GA_Offset &off) const;
	float getMass(const GA_Offset &off) const;
	float getBouncing(const GA_Offset &off) const;
	float getFriction(const GA_Offset &off) const;
	float getPadding(const GA_Offset &off) const;
	float getLinear_damping(const GA_Offset &off) const;
	float getAngular_damping(const GA_Offset &off) const;
	bool getSleeping(const GA_Offset &off) const;
	UT_Vector3 getPos(const GA_Offset &off) const;
	UT_Vector3 getBBox(const GA_Offset &off) const;
	bool getConvex(int index) const;
	const MyVec<GA_Offset>& getGEO_Offsets(int i) const;			///@return list of Points for object(i)
	const MyVec<GEO_Primitive*>& getGEO_Primitives(int i) const;	///@return list of Primitives for object(i)


private:
	void getVolumeCog(Index* ind, float &volumeSum, UT_Vector3 &cog);	///computes Volume and True Center of gravity(cog) of objects(ind)

	void computeIndexes(bool createPoint);	///adds indexes to objects from points and primitives attributes
	void computePointIndexes(SShape* inputShape, bool createPoint);		///adds indexes to objects from points attributes
	void computePrimitiveIndexes(SShape* inputShape, bool createPoint);	///adds indexes to objects from primitives attributes

	bool createObject(int index, int cindex, bool createPoint);	///adds new object(s) for index and cindex and set indexes a type(for compound)
	bool resizeObj(int i, bool createPoint);	///resizes m_indexes vector and add new point(createPoint option)


};
