/**
BulletSOP v2.0
Copyright (c)2010-2013 Suk Milan. All Rights Reserved.
This software is provided 'as-is', without any express or implied warranty.
In no event will the authors be held liable for any damages arising from the use of this software.
This file may not be redistributed in whole or significant part.
This notice may not be removed or altered from any source distribution.
*/
#pragma once

#include "SConstraint.h"
#include "SShape.h"
#include "SEmit.h"


/**
Grid class computes hash of every point.
Then You can use this for optimalization structure for faster computing of minimum distance constraints and so on ...
*/
class Grid
{
private:
	size_t* m_hash_first_point_i;
	size_t m_num_hash;
	size_t* m_point_next_i;

	float m_toler_distance2;

	UT_Vector3 m_min;
	UT_Vector3I m_size;
	UT_Vector3 m_cell_size;

	float m_memory_need;
	float m_memory_used;


public:
	Grid()	
	{
		m_hash_first_point_i = 0;
		m_point_next_i = 0;

		m_memory_need = 0;
		m_memory_used = 0;
	}

	bool init(GU_Detail *geo, float toler_distance, float memory_mult_MB)
	{
		m_toler_distance2 = toler_distance*toler_distance;

		float t = computeToler(geo, toler_distance, memory_mult_MB*1024*1024);
		setBBox(geo, t);

		if(!allocAndClearGrid(geo))
			return false;

		fillGrid(geo);
		return true;
	}

	~Grid()
	{
		if(m_hash_first_point_i)
			delete[] m_hash_first_point_i;
		if(m_point_next_i)
			delete[] m_point_next_i;
	}




	void createMinimumDistancePairGroups(GU_Detail* gdp_points, int i, SEmit* emit, int &emit_id)
	{
		GA_Offset primOff = emit->getGeo()->primitiveOffset(i);
		float connectDistance = emit->getConnectDistance(primOff);
		
		GA_Offset ptoff = gdp_points->pointOffset(i);
		UT_Vector3 pos_1 = gdp_points->getPos3(ptoff);
		UT_Vector3I gridPos = getGridPos(pos_1);

		for(int z = -1; z <= 1; z++)
			for(int y = -1; y <= 1; y++)
				for(int x = -1; x <= 1; x++)
				{
					long long int hash = getGridHash(gridPos + UT_Vector3I(x, y, z));
					if(!checkHash(hash))	continue;

					size_t next_i = m_hash_first_point_i[hash];
					while(next_i!=-1)
					{
						GA_Offset primOff_next = emit->getGeo()->primitiveOffset(next_i);

						if(primOff!=primOff_next)
						{
							GA_Offset pt_nextoff = gdp_points->pointOffset(next_i);
							UT_Vector3 pos_2 = gdp_points->getPos3(pt_nextoff);

							float len = (pos_1 - pos_2).length();
							if(len < connectDistance)
							{
								char t[255];
								sprintf(t, "bt_emit%d", emit_id++);
								GA_PrimitiveGroup* group = emit->getGeo()->newPrimitiveGroup(t, false);
								group->addOffset(primOff);
								group->addOffset(primOff_next);
							}
						}
						next_i = m_point_next_i[next_i];
					}
				}
	}


	///i - index of points from We want to create constraints
	///constr - constraints output
	void createMinimumDistanceConstraints(SShape* shape, int i, SConstraint* constr, float stiffness, float damping, float max_force, bool weariness, int iter, bool collision)
	{
		GA_Offset ptoff = shape->getGeo()->pointOffset(i);

		int index1 = shape->getPointIndex(ptoff);
		int cindex1 = shape->getPointCIndex(ptoff);
		if(index1 <= 0)
			return;

		UT_Vector3 pos_1 = shape->getGeo()->getPos3(ptoff);
		UT_Vector3I gridPos = getGridPos(pos_1);

		for(int z = -1; z <= 1; z++)
			for(int y = -1; y <= 1; y++)
				for(int x = -1; x <= 1; x++)
				{
					long long int hash = getGridHash(gridPos + UT_Vector3I(x, y, z));
					if(!checkHash(hash))	continue;

					size_t next_i = m_hash_first_point_i[hash];
					while(next_i!=-1)
					{
						GA_Offset ptoff_next = shape->getGeo()->pointOffset(next_i);
						int index2 = shape->getPointIndex(ptoff_next);
						int cindex2 = shape->getPointCIndex(ptoff_next);
						if(index2 > 0 && index2 > index1 && (cindex1<=0 || cindex1!=cindex2))
						{
							UT_Vector3 pos_2 = shape->getGeo()->getPos3(ptoff_next);

							float l = (pos_2 - pos_1).length2();
							if(l < m_toler_distance2)
							{
								GEO_Primitive* pr = constr->add();
								GA_Offset proff = pr->getMapOffset();

								constr->setIndexA(proff, index1);
								constr->setIndexB(proff, index2);
								constr->setPosA(pr, pos_1);
								constr->setPosB(pr, pos_2);

								constr->setStiffness(proff, stiffness);
								constr->setDamping(proff, damping);
								constr->setMaxForce(proff, max_force);
								constr->setWeariness(proff, weariness);
								constr->setIter(proff, iter);
								constr->setCollision(proff, collision);
								constr->setLinearLock(pr);
								constr->setAL(proff, UT_Vector3(0,0,0));
								constr->setAU(proff, UT_Vector3(0,0,0));
							}
						}
						next_i = m_point_next_i[next_i];
					}
				}
	}

	float getMemoryIfNeed()
	{
		if(m_memory_used > m_memory_need)
			return m_memory_used;
		return 0.0f;
	}


private:

	///computes toler_distance of grid according toler and amount of allocated memory
	float computeToler(GU_Detail *geo, float toler, float mem_byte_max)
	{
		UT_BoundingBox bbox;
		geo->getBBox(&bbox);

		//compute max_size what we need for fastest calculation
		UT_Vector3 min = (UT_Vector3)bbox.minvec() - toler*1.1f;
		UT_Vector3 max = (UT_Vector3)bbox.maxvec() + toler*1.1f;
		UT_Vector3 size = (max - min)/toler + 1;
		float s = size[0] * size[1] * size[2];

		//printf("we need mem: %fMB\n", s/1024/1024);
		m_memory_need = mem_byte_max;
		m_memory_used = s;
		if(s > mem_byte_max)
		{
			//printf("performance warning - Set max. memory close or higher than %fMB!\n", s/1024/1024);
			return (float)(toler * (s / mem_byte_max));
		}

		return toler;
	}

	void setBBox(GU_Detail *geo, float toler)
	{
		UT_BoundingBox bbox;
		geo->getBBox(&bbox);

		m_min = bbox.minvec();
		UT_Vector3 max = bbox.maxvec();

		m_min -= toler * 1.1f;
		max += toler * 1.1f;

		UT_Vector3 bbox_size = max - m_min;
		m_size = getVector3Int((bbox_size / toler) + 1);
		m_cell_size = toler;
	}

	bool allocAndClearGrid(GU_Detail *geo)
	{
		m_num_hash = m_size.x()*m_size.y()*m_size.z();
		size_t num_points = geo->getNumPoints();

		//alloc
		m_hash_first_point_i = new size_t[m_num_hash];
		m_point_next_i = new size_t[num_points];

		if(!m_hash_first_point_i || !m_point_next_i)
			return false;

		//clear
		for(int i = 0; i < m_num_hash; i++)
			m_hash_first_point_i[i] = -1;
		for(int i = 0; i < num_points; i++)
			m_point_next_i[i] = -1;
		return true;
	}

	///computes hash of every point and set last point_hash to voxel -> linked list of points in voxel
	void fillGrid(GU_Detail *geo)
	{
		int i = 0;
		GA_Offset ptoff;
		GA_FOR_ALL_PTOFF(geo, ptoff)
		{
			//UT_Vector3 pos = pt->getPos();
			UT_Vector3 pos = geo->getPos3(ptoff);

			UT_Vector3I gridPos = getGridPos(pos);
			size_t hash = getGridHash(gridPos);

			if(!checkHash(hash))	continue;

			size_t point_last_i = m_hash_first_point_i[hash];
			if(point_last_i!=-1)
				m_point_next_i[i] = point_last_i;
			m_hash_first_point_i[hash] = i;
			i++;
		}
	}


	inline bool checkHash(size_t hash)
	{
		if(/*hash < 0 || */hash >= m_num_hash)
		{
			printf("error: Hash boundary!\n");
			return false;
		}
		return true;
	}

	inline UT_Vector3I getGridPos(UT_Vector3 &pp)
	{
		return getVector3Int( (pp-m_min) / m_cell_size );
	}

	#define UMAD(a, b, c)  ( (a) * (b) + (c) )
	inline size_t getGridHash(UT_Vector3I gridPos)
	{
		return UMAD( UMAD(gridPos.z(), m_size.y(), gridPos.y()), m_size.x(), gridPos.x() );
	}

};


