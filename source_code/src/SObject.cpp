/**
BulletSOP v2.0
Copyright (c)2010-2013 Suk Milan. All Rights Reserved.
This software is provided 'as-is', without any express or implied warranty.
In no event will the authors be held liable for any damages arising from the use of this software.
This file may not be redistributed in whole or significant part.
This notice may not be removed or altered from any source distribution.
*/
#include "stdafx.h"
#include "SObject.h"
#include "SShape.h"
#include "SOP_Build.h"

SObject::SObject(void)
{
}

SObject::~SObject(void)
{
}


void SObject::init(SOPBoss* boss, GU_Detail* gdp, SShape* input)
{
	setBasic(boss, gdp);
	m_input = input;

	float bbox[3] = {1, 1, 1};

	m_attr_bt_index = SHelper::addIntPointAttr(m_gdp, "bt_index", 1);
	m_attr_bt_cindex = SHelper::addIntPointAttr(m_gdp, "bt_cindex", 1);
	m_attr_bt_type = SHelper::addIntPointAttr(m_gdp, "bt_type", 1);
	m_attr_bt_r = SHelper::addFloatVectorPointAttr(m_gdp, "bt_r");
	m_attr_bt_cog = SHelper::addFloatVectorPointAttr(m_gdp, "bt_cog");
	m_attr_bt_cog_r = SHelper::addFloatVectorPointAttr(m_gdp, "bt_cog_r");
	m_attr_bt_mass = SHelper::addFloatPointAttr(m_gdp, "bt_mass", 1, 1.0f);
	m_attr_bt_restitution = SHelper::addFloatPointAttr(m_gdp, "bt_restitution", 1);
	m_attr_bt_friction = SHelper::addFloatPointAttr(m_gdp, "bt_friction", 1, 0.8f);
	m_attr_bt_padding = SHelper::addFloatPointAttr(m_gdp, "bt_padding", 1, 0.01f);
	m_attr_bt_v = SHelper::addFloatVectorPointAttr(m_gdp, "bt_v");
	m_attr_bt_av = SHelper::addFloatVectorPointAttr(m_gdp, "bt_av");
	m_attr_bt_lin_damp = SHelper::addFloatPointAttr(m_gdp, "bt_lin_damp", 1);
	m_attr_bt_ang_damp = SHelper::addFloatPointAttr(m_gdp, "bt_ang_damp", 1);
	m_attr_bt_boxsize = SHelper::addFloatVectorPointAttr(m_gdp, "bt_boxsize", bbox);
	m_attr_bt_sleeping = SHelper::addIntPointAttr(m_gdp, "bt_sleeping", 1, 1);
	m_attr_bt_update = SHelper::addIntPointAttr(m_gdp, "bt_update", 1);
	m_attr_bt_recreate = SHelper::addIntPointAttr(m_gdp, "bt_recreate", 1);
	m_attr_bt_max_obj_index = SHelper::addIntDetailAttr(m_gdp, "bt_max_obj_index", 1);

	m_gdp->addVariableName("P", "BT");
	m_gdp->addVariableName("bt_index", "BI");
	m_gdp->addVariableName("bt_cindex", "BCI");
	m_gdp->addVariableName("bt_type", "BTYPE");
	m_gdp->addVariableName("bt_r", "BR");
	m_gdp->addVariableName("bt_cog", "BCOG");
	m_gdp->addVariableName("bt_cog_r", "BCOGR");
	m_gdp->addVariableName("bt_mass", "BMASS");
	m_gdp->addVariableName("bt_restitution", "BREST");
	m_gdp->addVariableName("bt_friction", "BFRI");
	m_gdp->addVariableName("bt_padding", "BPAD");
	m_gdp->addVariableName("bt_v", "BV");
	m_gdp->addVariableName("bt_av", "BAV");
	m_gdp->addVariableName("bt_lin_damp", "BLDMP");
	m_gdp->addVariableName("bt_ang_damp", "BADMP");
	m_gdp->addVariableName("bt_boxsize", "BB");
	m_gdp->addVariableName("bt_convex", "BCVX");
	m_gdp->addVariableName("bt_sleeping", "BSLEEP");
	m_gdp->addVariableName("bt_update", "BUPDATE");
	m_gdp->addVariableName("bt_recreate", "BRECREATE");

	computeIndexes(true);
}



void SObject::initFind(SOPBoss* boss, GU_Detail* gdp, SShape* input)
{
	setBasic(boss, gdp);
	m_input = input;

	m_attr_bt_index = SHelper::findPointAttrI(m_gdp, "bt_index");
	m_attr_bt_cindex = SHelper::findPointAttrI(m_gdp, "bt_cindex");
	m_attr_bt_type = SHelper::findPointAttrI(m_gdp, "bt_type");
	m_attr_bt_r = SHelper::findPointAttrV3(m_gdp, "bt_r");
	m_attr_bt_cog = SHelper::findPointAttrV3(m_gdp, "bt_cog");
	m_attr_bt_cog_r = SHelper::findPointAttrV3(m_gdp, "bt_cog_r");
	m_attr_bt_mass = SHelper::findPointAttrF(m_gdp, "bt_mass");
	m_attr_bt_restitution = SHelper::findPointAttrF(m_gdp, "bt_restitution");
	m_attr_bt_friction = SHelper::findPointAttrF(m_gdp, "bt_friction");
	m_attr_bt_padding = SHelper::findPointAttrF(m_gdp, "bt_padding");
	m_attr_bt_v = SHelper::findPointAttrV3(m_gdp, "bt_v");
	m_attr_bt_av = SHelper::findPointAttrV3(m_gdp, "bt_av");
	m_attr_bt_lin_damp = SHelper::findPointAttrF(m_gdp, "bt_lin_damp");
	m_attr_bt_ang_damp = SHelper::findPointAttrF(m_gdp, "bt_ang_damp");
	m_attr_bt_boxsize = SHelper::findPointAttrV3(m_gdp, "bt_boxsize");
	m_attr_bt_sleeping = SHelper::findPointAttrI(m_gdp, "bt_sleeping");
	m_attr_bt_update = SHelper::findPointAttrI(m_gdp, "bt_update");
	m_attr_bt_recreate = SHelper::findPointAttrI(m_gdp, "bt_recreate");
	m_attr_bt_max_obj_index = SHelper::findDetailAttrI(m_gdp, "bt_max_obj_index");


	if(m_attr_bt_index.isInvalid())			THROW_SOP("SObject: No \"bt_index\" point attribute", 0);
	if(m_attr_bt_cindex.isInvalid())		THROW_SOP("SObject: No \"bt_cindex\" point attribute", 0);
	if(m_attr_bt_type.isInvalid())			THROW_SOP("SObject: No \"bt_type\" point attribute", 0);
	if(m_attr_bt_mass.isInvalid())			THROW_SOP("SObject: No \"bt_mass\" point attribute", 0);
	if(m_attr_bt_cog.isInvalid())			THROW_SOP("SObject: No \"bt_cog\" point attribute", 0);
	if(m_attr_bt_cog_r.isInvalid())			THROW_SOP("SObject: No \"bt_cog_r\" point attribute", 0);
	if(m_attr_bt_restitution.isInvalid())	THROW_SOP("SObject: No \"bt_restitution\" point attribute", 0);
	if(m_attr_bt_friction.isInvalid())		THROW_SOP("SObject: No \"bt_friction\" point attribute", 0);
	if(m_attr_bt_padding.isInvalid())		THROW_SOP("SObject: No \"bt_padding\" point attribute", 0);
	if(m_attr_bt_v.isInvalid())				THROW_SOP("SObject: No \"bt_v\" point attribute", 0);
	if(m_attr_bt_av.isInvalid())			THROW_SOP("SObject: No \"bt_av\" point attribute", 0);
	if(m_attr_bt_r.isInvalid())				THROW_SOP("SObject: No \"bt_r\" point attribute", 0);
	if(m_attr_bt_lin_damp.isInvalid())		THROW_SOP("SObject: No \"bt_lin_damp\" point attribute", 0);
	if(m_attr_bt_ang_damp.isInvalid())		THROW_SOP("SObject: No \"bt_ang_damp\" point attribute", 0);
	if(m_attr_bt_boxsize.isInvalid())		THROW_SOP("SObject: No \"bt_boxsize\" point attribute", 0);
	if(m_attr_bt_sleeping.isInvalid())		THROW_SOP("SObject: No \"bt_sleeping\" point attribute", 0);
	if(m_attr_bt_update.isInvalid())		THROW_SOP("SObject: No \"bt_update\" point attribute", 0);
	if(m_attr_bt_recreate.isInvalid())		THROW_SOP("SObject: No \"bt_recreate\" point attribute", 0);
	if(m_attr_bt_max_obj_index.isInvalid())	THROW_SOP("SObject: No \"bt_max_obj_index\" detail attribute", 0);

	computeIndexes(false);
}


void SObject::initFind_TransformMinimum(SOPBoss* boss, GU_Detail* gdp, bool velocity)
{
	setBasic(boss, gdp);

	m_attr_bt_index = SHelper::findPointAttrI(m_gdp, "bt_index");
	m_attr_bt_cindex = SHelper::findPointAttrI(m_gdp, "bt_cindex");
	m_attr_bt_r = SHelper::findPointAttrV3(m_gdp, "bt_r");
	m_attr_bt_cog = SHelper::findPointAttrV3(m_gdp, "bt_cog");
	m_attr_bt_cog_r = SHelper::findPointAttrV3(m_gdp, "bt_cog_r");

	if(m_attr_bt_index.isInvalid())	THROW_SOP("SObject: No \"bt_index\" point attribute", 0);
	if(m_attr_bt_cindex.isInvalid())THROW_SOP("SObject: No \"bt_cindex\" point attribute", 0);
	if(m_attr_bt_r.isInvalid())		THROW_SOP("SObject: No \"bt_r\" point attribute", 0);
	if(m_attr_bt_cog.isInvalid())	THROW_SOP("SObject: No \"bt_cog\" point attribute", 0);
	if(m_attr_bt_cog_r.isInvalid())	THROW_SOP("SObject: No \"bt_cog_r\" point attribute", 0);

	if(velocity)
	{
		m_attr_bt_v = SHelper::findPointAttrV3(m_gdp, "bt_v");
		m_attr_bt_av = SHelper::findPointAttrV3(m_gdp, "bt_av");

		if(m_attr_bt_v.isInvalid())		THROW_SOP("SObject: No \"bt_v\" point attribute", 0);
		if(m_attr_bt_av.isInvalid())	THROW_SOP("SObject: No \"bt_av\" point attribute", 0);
	}

	//computeIndexes();	//dosn't call it, because m_input is NULL!
}


size_t SObject::getNumIndexes() const
{
	return m_indexes.size();
}

bool SObject::isIndexExist(int index) const
{
	return index > 0 && index < m_indexes.size();
}



bool SObject::resizeObj(int i, bool createPoint)
{
	m_indexes.resizeAdd(i+1);

	if(createPoint && !m_indexes[i].exist)
	{
		m_indexes[i].exist = true;
		m_indexes[i].ptoff = m_gdp->appendPointOffset();
		m_indexes[i].pos = (int)m_gdp->getPointMap().indexSize() - 1;
		return true; 
	}

	return false;
}




bool SObject::createObject(int index, int cindex, bool createPoint)
{
	if(index > 0)
	{
		resizeObj(index, createPoint);
		if(createPoint)
		{
			setIndex(m_indexes[index].ptoff, index);
			setCIndex(m_indexes[index].ptoff, cindex);
		}
	}
	else
		return false;


	if(cindex > 0)	//create compound objects
	{
		resizeObj(cindex, createPoint);
		m_indexes[cindex].parent = true;
		if(createPoint)
		{
			setIndex(m_indexes[cindex].ptoff, cindex);
			setType(m_indexes[cindex].ptoff, SOP_Build::TYPE_COMPOUND);
		}
	}

	return true;
}




void SObject::computeIndexes(bool createPoint)
{
	m_indexes.clear();
	m_indexes.setExtraAlloc(EXTRA_ALLOC);

	if(m_input)
	{
		resizeObj(m_input->getMaxIndex(), false);	//just create some space for future points

		computePointIndexes(m_input, createPoint);
		computePrimitiveIndexes(m_input, createPoint);

		updateMaxObjIndex();
	}
}


void SObject::computePointIndexes(SShape* inputShape, bool createPoint)
{
	BOSSP_START;

	int num_points = inputShape->getNumPoints();

	int i = 0;
	GA_Offset ptoff;
	GA_FOR_ALL_PTOFF(inputShape->getGeo(), ptoff)
	{
		int index = inputShape->getPointIndex(ptoff);
		int cindex = inputShape->getPointCIndex(ptoff);

		if(!createObject(index, cindex, createPoint))
			continue;

		m_indexes[index].addPoint(ptoff);

		if(cindex > 0)
			m_indexes[cindex].addPoint(ptoff);

		BOSSP_INTERRUPT(i, num_points);
		i++;
	}

	BOSSP_END;
}


void SObject::computePrimitiveIndexes(SShape* inputShape, bool createPoint)
{
	BOSSP_START;

	int num_prims = inputShape->getNumPrims();

	GEO_Primitive* primGeo;
	int i = 0;
	GA_FOR_ALL_PRIMITIVES(inputShape->getGeo(), primGeo)
	{
		int index = inputShape->getPrimitiveIndex(primGeo->getMapOffset());
		int cindex = inputShape->getPrimitiveCIndex(primGeo->getMapOffset());

		if(!createObject(index, cindex, createPoint))
			continue;

		m_indexes[index].addPrimitive(primGeo);

		if(cindex > 0)
			m_indexes[cindex].addPrimitive(primGeo);

		BOSSP_INTERRUPT(i, num_prims);
		i++;
	}

	BOSSP_END;
}






void SObject::computeBoxCOG()
{
	BOSSP_START;

	GU_Detail* gdp = m_input->getGeo();

	const int num = (int)m_indexes.size();
	for(int i=0; i < num; i++)
	{
		Index* ind = &m_indexes[i];
		if(ind->pos > -1)
		{
			//compute COG for object
			UT_Vector3 min = UT_Vector3( 10000000000,  10000000000,  10000000000);
			UT_Vector3 max = UT_Vector3(-10000000000, -10000000000, -10000000000);
			for(int j=0; j < ind->getPointSize(); j++)
			{
				UT_Vector3 ppt = gdp->getPos3(ind->getPoint(j));

				if(min.x() > ppt.x())	min[0] = ppt.x();
				if(min.y() > ppt.y())	min[1] = ppt.y();
				if(min.z() > ppt.z())	min[2] = ppt.z();

				if(max.x() < ppt.x())	max[0] = ppt.x();
				if(max.y() < ppt.y())	max[1] = ppt.y();
				if(max.z() < ppt.z())	max[2] = ppt.z();
			}

			setCOG(ind->ptoff, min + (max-min)*0.5f);
		}

		BOSSP_INTERRUPT(i, num);
	}

	BOSSP_END;
}


void SObject::computeAverageCOG()
{
	BOSSP_START;

	GU_Detail* gdp = m_input->getGeo();

	const int num = (int)m_indexes.size();
	for(int i=0; i < num; i++)
	{
		Index* ind = &m_indexes[i];
		if(ind->pos > -1)
		{
			//compute COG for object
			UT_Vector3 sum(0,0,0);
			for(int j=0; j < ind->getPointSize(); j++)
				sum += gdp->getPos3(ind->getPoint(j));

			setCOG(ind->ptoff, sum / ind->getPointSize());
		}

		BOSSP_INTERRUPT(i, num);
	}

	BOSSP_END;
}




//this function is part of algorithm for computing volume of object
float signedVolumeOfTriangle(UT_Vector3 p1, UT_Vector3 p2, UT_Vector3 p3) 
{
    float v321 = p3.x()*p2.y()*p1.z();
    float v231 = p2.x()*p3.y()*p1.z();
    float v312 = p3.x()*p1.y()*p2.z();
    float v132 = p1.x()*p3.y()*p2.z();
    float v213 = p2.x()*p1.y()*p3.z();
    float v123 = p1.x()*p2.y()*p3.z();
    return (1.0f/6.0f)*(-v321 + v231 + v312 - v132 - v213 + v123);
}

void SObject::getVolumeCog(Index* ind, float &volumeSum, UT_Vector3 &cog)
{
	volumeSum = 0;
	cog = UT_Vector3(0,0,0);

	if(ind->getPointSize()==0)
		return;

	GU_Detail* gdp = m_input->getGeo();

	UT_Vector3 center = gdp->getPos3(ind->getPoint(0));

	for(int j=0; j < ind->getPrimitiveSize(); j++)
	{
		GEO_Primitive* prim = ind->getPrimitive(j);
		if(prim->getVertexCount() < 3)
			continue;

		UT_Vector3 p1 = getPrimPos3(prim, 0) - center;
		UT_Vector3 p2 = getPrimPos3(prim, 1) - center;
		for( int v = 2; v < prim->getVertexCount(); v++ )
		{
			UT_Vector3 p3 = getPrimPos3(prim, v) - center;

			float volume = signedVolumeOfTriangle(p1, p2, p3);	//note: its more accure if object is close to init axes
			volumeSum += volume;
			cog += (p1+p2+p3)/4 * volume;

			p2 = p3;
		}
	}

	if(volumeSum)
		cog /= (float)volumeSum;
	cog += center;

	if(volumeSum < 0)
		volumeSum *= -1;
}


void SObject::computeVolumeCOG()
{
	BOSSP_START;

	const int num = (int)m_indexes.size();
	for(int i=0; i < num; i++)
	{
		Index* ind = &m_indexes[i];
		if(ind->pos > -1)
		{
			//compute COG for object
			float volume;
			UT_Vector3 cog;
			getVolumeCog(ind, volume, cog);

			setCOG(ind->ptoff, cog);
		}

		BOSSP_INTERRUPT(i, num);
	}

	BOSSP_END;
}





void SObject::computeMassFromVolume(float density)
{
	BOSSP_START;

	const int num = (int)m_indexes.size();
	for(int i=0; i < num; i++)
	{
		Index* ind = &m_indexes[i];
		if(ind->pos > -1)
		{
			//compute mass for object
			float volume;
			UT_Vector3 cog;
			getVolumeCog(ind, volume, cog);

			setMass(ind->ptoff, (float)(density*volume));
		}

		BOSSP_INTERRUPT(i, num);
	}

	BOSSP_END;
}





///creates rotation matrix from Up and N vector and set new rotation X,Y,Z axes and @return rotation angles
UT_Vector3 computeAxesPlanes(UT_Vector3 up, UT_Vector3 n, UT_Vector3 &nX, UT_Vector3 &nY, UT_Vector3 &nZ)
{
	//if(up[0]==0 && up[1]==-1 && up[2]==0)

	UT_Matrix3 matInit;
	//matInit.identity();
	matInit.lookat(UT_Vector3(0,0,0), UT_Vector3(0,1,0), UT_Vector3(0,0,1));

	UT_Matrix3 matFinal;
	//matFinal.identity();
	matFinal.lookat(UT_Vector3(0,0,0), up, n);

	matInit.invert();
	matFinal = matInit * matFinal;

	//axes rotation
	nX = UT_Vector3(1, 0, 0) * matFinal;
	nY = UT_Vector3(0, 1, 0) * matFinal;
	nZ = UT_Vector3(0, 0, 1) * matFinal;

	//get rotation angles
	UT_Vector3 r;
	UT_XformOrder order(UT_XformOrder::TSR, UT_XformOrder::XYZ);
	matFinal.crack(r, order);
	return r * RAD_TO_DEG;
}



void SObject::computeCogRotation()
{
	if(!m_input)
		THROW_SOP("No Input", 0);

	BOSSP_START;

	if(m_input->hasNandUpAttribs())	//It needs Up and N vector
	{
		const int num = (int)m_indexes.size();
		for(int i=0; i < num; i++)
		{
			Index* ind = &m_indexes[i];
			if(ind->pos > -1 && ind->getPointSize())
			{
				//compute object rotation 
				UT_Vector3 n = m_input->getN(ind->getPoint(0));
				UT_Vector3 up = m_input->getUp(ind->getPoint(0));
				UT_Vector3 nX, nY, nZ;

				UT_Vector3 rot = computeAxesPlanes(up, n, nX, nY, nZ);

				setCOGRotation(ind->ptoff, rot);
			}

			BOSSP_INTERRUPT(i, num);
		}
	}

	BOSSP_END;
}



void SObject::computeBBox()
{
	BOSSP_START;

	GU_Detail* gdp = m_input->getGeo();

	const int num = (int)m_indexes.size();
	for(int i=0; i < num; i++)
	{
		Index* ind = &m_indexes[i];
		if(ind->pos >= 0)
		{
			//BBox is relative to init rotation 
			UT_Vector3 center = getCOG(ind->ptoff);
			UT_Vector3 rot = getCOGRotation(ind->ptoff) * DEG_TO_RAD;

			UT_Matrix4 mat = UT_Matrix4::getIdentityMatrix();
			UT_XformOrder order(UT_XformOrder::TSR, UT_XformOrder::XYZ);
			mat.rotate(rot.x(), rot.y(), rot.z(), order);
			mat.invert();	//revers !!!

			//compute object BBox
			UT_Vector3 max(0,0,0);
			for(int j=0; j < ind->getPointSize(); j++)
			{
				UT_Vector3 pos = gdp->getPos3(ind->getPoint(j));
				pos -= center;
				pos *= mat;	//revers object rotation

				UT_Vector3 maxd = getAbsV3(pos) * 2;
				if(maxd.x() > max.x())	max[0] = maxd.x();
				if(maxd.y() > max.y())	max[1] = maxd.y();
				if(maxd.z() > max.z())	max[2] = maxd.z();
			}

			if(max==UT_Vector3(0,0,0))
				max = m_input->getBBox();

			setBBox(ind->ptoff, max);
		}

		BOSSP_INTERRUPT(i, num);
	}

	BOSSP_END;
}




void SObject::copyCOGtoPos()
{
	GA_Offset ptoff;
	GA_FOR_ALL_PTOFF(m_gdp, ptoff)
		setPos(ptoff, getCOG(ptoff));
}


void SObject::copyCOGRotToRot()
{
	GA_Offset ptoff;
	GA_FOR_ALL_PTOFF(m_gdp, ptoff)
		setRotation(ptoff, getCOGRotation(ptoff));
}




void SObject::updateMaxObjIndex()
{
	int max = 0;
	GA_Offset ptoff;
	GA_FOR_ALL_PTOFF(m_gdp, ptoff)
	{
		int index = getIndex(ptoff);
		int cindex = getCIndex(ptoff);

		if(index > max)	max = index;
		if(cindex > max)max = cindex;
	}

	m_attr_bt_max_obj_index.set(0, max);
}


void SObject::addToGroup(GA_PointGroup* group)
{
	GA_Offset ptoff;
	GA_FOR_ALL_PTOFF(m_gdp, ptoff)
		group->addOffset(ptoff);
}




void SObject::setAllUpdate(bool update)
{
	GA_Offset ptoff;
	GA_FOR_ALL_PTOFF(m_gdp, ptoff)
		setUpdate(ptoff, update);
}
void SObject::setAllRecreate(bool update)
{
	GA_Offset ptoff;
	GA_FOR_ALL_PTOFF(m_gdp, ptoff)
		setRecreate(ptoff, update);
}



void SObject::setAllMass(float mass)
{
	GA_Offset ptoff;
	GA_FOR_ALL_PTOFF(m_gdp, ptoff)
		setMass(ptoff, mass);
}

void SObject::setAllPadding(float padding)
{
	GA_Offset ptoff;
	GA_FOR_ALL_PTOFF(m_gdp, ptoff)
		setPadding(ptoff, padding);
}


void SObject::setAllNotCompoundType(int type)
{
	GA_Offset ptoff;
	GA_FOR_ALL_PTOFF(m_gdp, ptoff)
	{
		int index = getIndex(ptoff);
		if(!m_indexes[index].parent)
			setType(ptoff, type);
	}
}





void SObject::copyPrimAttrs()
{
	if(!m_input)
		THROW_SOP("No Input", 0);

	BOSSP_START;

	const int num = (int)m_indexes.size();
	for(int i=0; i < num; i++)
	{
		Index* ind = &m_indexes[i];

		if(ind->exist)
		{
			GA_Attribute* src_attr; 
			GA_FOR_ALL_PRIMITIVE_ATTRIBUTES(m_input->getGeo(), src_attr)
			{
				GA_RWHandleF rAttr = SHelper::findPrimitiveAttrF(m_input->getGeo(), src_attr->getName()).getAttribute();
				GA_RWHandleF wAttr( m_gdp->addPointAttrib(src_attr) );

				for(int ii=0; ii < rAttr.getTupleSize(); ii++)
					wAttr.set(ind->ptoff, ii, rAttr.get(ind->getPrimitive(0)->getMapOffset(), ii));


				BOSSP_INTERRUPT_EMPTY;
			}
		}

		BOSSP_INTERRUPT(i, num);
	}

	BOSSP_END;
}




bool SObject::isConvexBasic(int index)
{
	Index* ind = &m_indexes[index];
	if(ind->exist)
	{
		UT_Vector3 center = getCOG(ind->ptoff);

		bool convex = true;
		for(int j=0; j < ind->getPrimitiveSize(); j++)
		{
			GEO_Primitive* prim = ind->getPrimitive(j);

			UT_Vector3 n = prim->computeNormal();
			n.normalize();	//not sure
			float d = -n.dot( getPrimPos3(prim, 0) );

			float dist = n.dot(center) + d;
			if(dist > 0)
			{
				convex = false;
				break;
			}
		}
		
		return convex;
	}
	return false;
}

bool SObject::isConvexAdvance(int index, float toler)
{
	Index* ind = &m_indexes[index];
	if(ind->pos > -1)
	{
		bool convex = true;
		for(int j=0; j < ind->getPrimitiveSize(); j++)
		{
			GEO_Primitive* prim = ind->getPrimitive(j);
			UT_Vector3 n = prim->computeNormal();
			float d = -n.dot( getPrimPos3(prim, 0) );

			for(int jj=j+1; jj < ind->getPrimitiveSize(); jj++)
			{
				GEO_Primitive* prim2 = ind->getPrimitive(jj);

				for(int v = 0; v < prim2->getVertexCount(); v++)
				{
					UT_Vector3 pp = getPrimPos3(prim2, v);
					float dist = n.dot(pp) + d;
					if(dist > toler)
					{
						convex = false;
						break;
					}
				}
				if(!convex)
					break;
			}
		}
		
		return convex;
	}

	return false;
}



void SObject::computeConvex(bool advance, float toler, bool recomputeVolumeCOG)
{
	BOSSP_START;

	const int N = (int)m_indexes.size();
	if(advance)
	{
		for(int i=0; i < N; i++)
		{
			if(m_indexes[i].exist)
				m_indexes[i].convex = isConvexAdvance(i, toler);

			BOSSP_INTERRUPT(i, N);
		}
	}
	else
	{
		if(recomputeVolumeCOG)
			computeVolumeCOG();

		for(int i=0; i < N; i++)
		{
			if(m_indexes[i].exist)
				m_indexes[i].convex = isConvexBasic(i);

			BOSSP_INTERRUPT(i, N);
		}
	}

	BOSSP_END;
}




void SObject::deleteOuputSubcompounds()
{
	BOSSP_START;

	int num_points = (int)m_gdp->getPointMap().indexSize();
	int i=0;
	GA_PointGroup* delArr = m_gdp->newPointGroup("____bulletdel");	//for faster deleting we use group

	GA_Offset ptoff;
	GA_FOR_ALL_PTOFF(m_gdp, ptoff)
	{
		int cindex = getCIndex(ptoff);
		if(cindex > 0)
			delArr->addOffset(ptoff);

		BOSSP_INTERRUPT(i, num_points);
		i++;
	}

	//real deleting
	m_gdp->deletePoints(*delArr);
	m_gdp->destroyGroup(delArr);

	BOSSP_END;
}




void SObject::updateAllCompoundMass()
{
	BOSSP_START;
	
	GA_Offset ptoff;

	{
	GA_FOR_ALL_PTOFF(m_gdp, ptoff)
		if(getType(ptoff)==SOP_Build::TYPE_COMPOUND)
				setMass(ptoff, 0);
	}

	GA_FOR_ALL_PTOFF(m_gdp, ptoff)
		if(getType(ptoff)!=SOP_Build::TYPE_COMPOUND)
		{
			int cindex = getCIndex(ptoff);
			if(cindex > 0)
			{
				GA_Offset ptoffC = m_indexes[cindex].ptoff;
				float new_mass = getMass(ptoff) + getMass(ptoffC);
				setMass(ptoffC, new_mass);
			}
		}

	BOSSP_END;
}






void SObject::setIndex(const GA_Offset &off, int i)
{
	m_attr_bt_index.set(off, i);
}
void SObject::setCIndex(const GA_Offset &off, int i)
{
	m_attr_bt_cindex.set(off, i);
}


void SObject::setPos(const GA_Offset &off, UT_Vector3 v)
{
	m_gdp->setPos3(off, v);
}

void SObject::setCOG(const GA_Offset &off, UT_Vector3 v)
{
	m_attr_bt_cog.set(off, v);
}

void SObject::setCOGRotation(const GA_Offset &off, UT_Vector3 v)
{
	m_attr_bt_cog_r.set(off, v);
}

void SObject::setMass(const GA_Offset &off, float mass)
{
	m_attr_bt_mass.set(off, mass);
}

void SObject::setBBox(const GA_Offset &off, UT_Vector3 v)
{
	m_attr_bt_boxsize.set(off, v);
}




void SObject::setType(const GA_Offset &off, int i)
{
	m_attr_bt_type.set(off, i);
}

void SObject::setRestitution(const GA_Offset &off, float v)
{
	m_attr_bt_restitution.set(off, v);
}
void SObject::setFriction(const GA_Offset &off, float v)
{
	m_attr_bt_friction.set(off, v);
}
void SObject::setPadding(const GA_Offset &off, float v)
{
	m_attr_bt_padding.set(off, v);
}



void SObject::setLinearDamping(const GA_Offset &off, float v)
{
	m_attr_bt_lin_damp.set(off, v);
}
void SObject::setAlgularDamping(const GA_Offset &off, float v)
{
	m_attr_bt_ang_damp.set(off, v);
}
void SObject::setRotation(const GA_Offset &off, UT_Vector3 v)
{
	m_attr_bt_r.set(off, v);
}
void SObject::setLinearVelocity(const GA_Offset &off, UT_Vector3 v)
{
	m_attr_bt_v.set(off, v);
}
void SObject::setAlgularVelocity(const GA_Offset &off, UT_Vector3 v)
{
	m_attr_bt_av.set(off, v);
}
void SObject::setSleep(const GA_Offset &off, int i)
{
	m_attr_bt_sleeping.set(off, i);
}
void SObject::setUpdate(const GA_Offset &off, int i)
{
	m_attr_bt_update.set(off, i);
}
void SObject::setRecreate(const GA_Offset &off, int i)
{
	m_attr_bt_recreate.set(off, i);
}






bool SObject::getConvex(int index) const
{
	return m_indexes[index].convex;
}

int SObject::getIndex(const GA_Offset &off) const
{
	return m_attr_bt_index(off);
}

int SObject::getCIndex(const GA_Offset &off) const
{
	return m_attr_bt_cindex(off);
}



UT_Vector3 SObject::getCOG(const GA_Offset &off) const
{
	return m_attr_bt_cog(off);

}
UT_Vector3 SObject::getCOGRotation(const GA_Offset &off) const
{
	return m_attr_bt_cog_r(off);
}


UT_Vector3 SObject::getV(const GA_Offset &off) const
{
	return m_attr_bt_v(off);
}
UT_Vector3 SObject::getAV(const GA_Offset &off) const
{
	return m_attr_bt_av(off);
}

UT_Vector3 SObject::getRotation(const GA_Offset &off) const
{
	return m_attr_bt_r(off);
}




UT_Matrix4 SObject::getRotationMatrix(const GA_Offset &off) const
{
	UT_Vector3 r = getRotation(off) * DEG_TO_RAD;
	UT_Vector3 cogr = getCOGRotation(off) * DEG_TO_RAD;

	UT_XformOrder order(UT_XformOrder::TSR, UT_XformOrder::XYZ);

	UT_Matrix4 mat, matr, matcogr;
	mat = matr = matcogr = UT_Matrix4::getIdentityMatrix();

	matr.rotate(r.x(), r.y(), r.z(), order);
	matcogr.rotate(cogr.x(), cogr.y(), cogr.z(), order);

	//relative rotation cogr -> r
	mat = matcogr;
	mat.invert();
	mat *= matr;

	return mat;
}


UT_Matrix4 SObject::getTranformationMatrix(const GA_Offset &off) const
{
	UT_Matrix4 mat = getRotationMatrix(off);
	UT_Vector3 t = getPos(off);

	mat.translate(t.x(), t.y(), t.z());	//add translation
	return mat;
}



bool SObject::getUpdate(const GA_Offset &off) const
{
	return m_attr_bt_update(off)!=0;
}
bool SObject::getRecreate(const GA_Offset &off) const
{
	return m_attr_bt_recreate(off)!=0;
}



int SObject::getType(const GA_Offset &off) const
{
	return m_attr_bt_type(off);
}


float SObject::getMass(const GA_Offset &off) const
{
	return m_attr_bt_mass(off);
}
float SObject::getBouncing(const GA_Offset &off) const
{
	return m_attr_bt_restitution(off);
}
float SObject::getFriction(const GA_Offset &off) const
{
	return m_attr_bt_friction(off);
}
float SObject::getPadding(const GA_Offset &off) const
{
	return m_attr_bt_padding(off);
}
float SObject::getLinear_damping(const GA_Offset &off) const
{
	return m_attr_bt_lin_damp(off);
}
float SObject::getAngular_damping(const GA_Offset &off) const
{
	return m_attr_bt_ang_damp(off);
}


bool SObject::getSleeping(const GA_Offset &off) const
{
	return m_attr_bt_sleeping(off)!=0;
}

UT_Vector3 SObject::getPos(const GA_Offset &off) const
{
	return m_gdp->getPos3(off);
}

UT_Vector3 SObject::getBBox(const GA_Offset &off) const
{
	return m_attr_bt_boxsize(off);
}


const MyVec<GA_Offset>& SObject::getGEO_Offsets(int i) const
{
	return m_indexes[i].getPoints();
}
const MyVec<GEO_Primitive*>& SObject::getGEO_Primitives(int i) const
{
	return m_indexes[i].getPrimitives();
}

SShape* SObject::getShape() const
{
	return m_input;
}



void SObject::destroyForTransformOutput()
{
	destroyForTransformVelocityOutput();

	SHelper::destroyPointAttr(m_gdp, m_attr_bt_v);
	SHelper::destroyPointAttr(m_gdp, m_attr_bt_av);
}


void SObject::destroyForTransformVelocityOutput()
{
	SHelper::destroyPointAttr(m_gdp, m_attr_bt_type);
	SHelper::destroyPointAttr(m_gdp, m_attr_bt_mass);
	SHelper::destroyPointAttr(m_gdp, m_attr_bt_restitution);
	SHelper::destroyPointAttr(m_gdp, m_attr_bt_friction);
	SHelper::destroyPointAttr(m_gdp, m_attr_bt_padding);
	SHelper::destroyPointAttr(m_gdp, m_attr_bt_lin_damp);
	SHelper::destroyPointAttr(m_gdp, m_attr_bt_ang_damp);
	SHelper::destroyPointAttr(m_gdp, m_attr_bt_boxsize);
	SHelper::destroyPointAttr(m_gdp, m_attr_bt_sleeping);
	SHelper::destroyPointAttr(m_gdp, m_attr_bt_update);
	SHelper::destroyPointAttr(m_gdp, m_attr_bt_recreate);
}


void SObject::destroyForInstanceOutput()
{
	destroyForTransformOutput();

	SHelper::destroyPointAttr(m_gdp, m_attr_bt_r);
	SHelper::destroyPointAttr(m_gdp, m_attr_bt_cog);
	SHelper::destroyPointAttr(m_gdp, m_attr_bt_cog_r);
}



