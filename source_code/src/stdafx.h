/**
BulletSOP v2.0
Copyright (c)2010-2013 Suk Milan. All Rights Reserved.
This software is provided 'as-is', without any express or implied warranty.
In no event will the authors be held liable for any damages arising from the use of this software.
This file may not be redistributed in whole or significant part.
This notice may not be removed or altered from any source distribution.
*/
#pragma once

#include <SYS/SYS_Math.h>
#include <UT/UT_Interrupt.h>
#include <UT/UT_Vector3.h>
#include <UT/UT_Vector4.h>
#include <UT/UT_Matrix4.h>

//#include <GEO/GEO_AdjPolyIterator.h>	//!!!

#include <GU/GU_Detail.h>
#include <GU/GU_PrimPoly.h>
#include <GU/GU_RayIntersect.h>

#include <PRM/PRM_Include.h>
#include <PRM/PRM_ChoiceList.h>
#include <PRM/PRM_SpareData.h>
#include <PRM/PRM_Parm.h>

#include <OP/OP_Operator.h>
#include <OP/OP_OperatorTable.h>
#include <OP/OP_Director.h>

#include <SOP/SOP_Node.h>


#include <time.h>
#include <stdio.h>
#include <cstdlib>


#define RAD_TO_DEG 57.2957795f		//57.295779513082320876798154814105
#define DEG_TO_RAD 0.0174532925f	//0.01745329251994329576923690768489

#define TOOL_NAME "BulletSOP 2.0.11"

#define EXTRA_ALLOC 10000
#define EXTRA_ALLOC_SMALL 10


template<typename T>
class MyVec
{
private:
	T* m_data;
	size_t m_data_size;
	size_t m_data_size_alloc;

	size_t m_extra_alloc;

public:
	MyVec()
	{
		m_data = 0;
		m_data_size = 0;
		m_data_size_alloc = 0;
		m_extra_alloc = 10;
	}

	MyVec(const MyVec& v)
	{
		//clear();
		copyIt(v);
	}


	MyVec& operator=(const MyVec& v)
	{
		clear();
		copyIt(v);
		return *this;
	}

	~MyVec()
	{
		clear();
	}

	void setExtraAlloc(size_t size)
	{
		m_extra_alloc = size;
	}

	size_t size() const
	{
		return m_data_size;
	}


	bool resizeAdd(size_t size)
	{
		if(size > m_data_size)
		{
			resize(size);
			return true;
		}
		return false;
	}

	size_t resize(size_t size)	///returns old size
	{
		reserve(size + m_extra_alloc);

		size_t old_size = m_data_size;
		m_data_size = size;
		return old_size;
	}

	void reserve(size_t size)
	{
		if(size > m_data_size_alloc)
		{
			m_data_size_alloc = size;

			T* new_data = 0;
			if(m_data_size_alloc)
			{
				new_data = new T[m_data_size_alloc];
				for(int i=0; i < m_data_size; i++)
					new_data[i] = m_data[i];
			}

			delete[] m_data;
			m_data = new_data;
		}

	}

	void clear()
	{
		delete[] m_data;
		m_data = 0;
		m_data_size = 0;
		m_data_size_alloc = 0;
	}

	void push_back(T& value)
	{
		size_t pos = resize(m_data_size+1);
		m_data[pos] = value;
	}
	void push_back(const T& value)
	{
		size_t pos = resize(m_data_size+1);
		m_data[pos] = value;
	}

	void erase(size_t i)
	{
		if(i < m_data_size-1)
			memmove(m_data+i, m_data+i+1, (size()-i-1)*sizeof(T));	//copy
		m_data_size--;
	}

	T& operator[](int i) const
	{
		return m_data[i];
	}

	T& get(int i) const
	{
		return m_data[i];
	}


	bool find(T v) const
	{
		for(int i=0; i < m_data_size; i++)
			if(m_data[i]==v)
				return true;
		return false;
	}

	bool find(T v1, T v2) const
	{
		bool find1, find2;
		find1 = find2 = false;
		for(int i=0; i < m_data_size; i++)
		{
			if( m_data[i]==v1 )
				find1 = true;

			if( m_data[i]==v2 )
				find2 = true;

			if(find1 && find2)
				return true;
		}
		return false;
	}

private:
	void copyIt(const MyVec& v)
	{
		m_data = 0;
		if(v.m_data)
		{
			m_data = new T[v.m_data_size_alloc];
			memcpy(m_data, v.m_data, v.m_data_size*sizeof(T));
		}

		m_data_size = v.m_data_size;
		m_data_size_alloc = v.m_data_size_alloc;
		m_extra_alloc = v.m_extra_alloc;
	}

};



/**
Reading parameters(GUI)
*/
#define INT_PARM(name, vidx, t)	\
	    return (int)evalInt(name, vidx, t);
#define FLT_PARM(name, vidx, t)	\
	    return (float)evalFloat(name, vidx, t);
#define V3_PARM(name, t)	\
	    return UT_Vector3( (float)evalFloat(name, 0, t), (float)evalFloat(name, 1, t), (float)evalFloat(name, 2, t));

#define INT_PARM_MULT(name, vidx, t, inst)	\
	    return (int)evalIntInst(name, inst, vidx, t, 1);
#define FLT_PARM_MULT(name, vidx, t, inst)	\
	    return (float)evalFloatInst(name, inst, vidx, t, 1);
#define V3_PARM_MULT(name, t, inst)	\
	    return UT_Vector3( (float)evalFloatInst(name, inst, 0, t, 1), (float)evalFloatInst(name, inst, 1, t, 1), (float)evalFloatInst(name, inst, 2, t,1));

/**
Paramters range <>
*/
static PRM_Range g_range_zero_more(PRM_RANGE_RESTRICTED, 0.0, PRM_RANGE_FREE, 10000000000);	//0+
static PRM_Range g_range_one_more(PRM_RANGE_RESTRICTED, 1, PRM_RANGE_FREE, 10000000000);	//1+
static PRM_Range g_range_zero_min_more(PRM_RANGE_RESTRICTED, 0.0001f, PRM_RANGE_FREE, 10000000000);	//0.0001+
static PRM_Range g_range_minus_one_more(PRM_RANGE_RESTRICTED, -1, PRM_RANGE_FREE, 10000000000);	//(-1)+



inline float absoluteValue(float v)
{
	if(v < 0)	return -v;
	else		return v;
}
inline int absoluteValue(int v)
{
	if(v < 0)	return -v;
	else		return v;
}

///@return abs vector
inline UT_Vector3 getAbsV3(UT_Vector3 p)
{
	return UT_Vector3(	absoluteValue(p.x()),
						absoluteValue(p.y()),
						absoluteValue(p.z()));
}


inline UT_Vector3I getVector3Int(UT_Vector3 p)
{
	return UT_Vector3I(	(int)p.x(),
						(int)p.y(),
						(int)p.z());
}


//because of back compatibility with H12, which doesn't has prim->getPos3(i) !!!
inline UT_Vector3 getPrimPos3(GEO_Primitive* prim, GA_Size i)
{
	return prim->getDetail().getPos3( prim->getPointOffset(i) );
}





#define BOSS_START if(!m_boss.opStart(__FUNCTION__))	throw SOPException(__FUNCTION__, "Interrupt", NULL);
#define BOSS_INTERRUPT(i, N) if(m_boss.opInterrupt(i, N))	throw SOPException(__FUNCTION__, "Interrupt", NULL);
#define BOSS_INTERRUPT_EMPTY if(m_boss.opInterrupt())	throw SOPException(__FUNCTION__, "Interrupt", NULL);
#define BOSS_END m_boss.opEnd()

#define BOSSP_START if(!m_boss->opStart(__FUNCTION__))	throw SOPException(__FUNCTION__, "Interrupt", NULL);
#define BOSSP_INTERRUPT(i, N) if(m_boss->opInterrupt(i, N))	throw SOPException(__FUNCTION__, "Interrupt", NULL);
#define BOSSP_INTERRUPT_EMPTY if(m_boss->opInterrupt())	throw SOPException(__FUNCTION__, "Interrupt", NULL);
#define BOSSP_END m_boss->opEnd()

#define THROW_SOP(err, warn) throw SOPException(__FUNCTION__, err, warn);


class SOPBoss
{
private:
	UT_Interrupt* m_boss;
	int m_depth;

public:
	SOPBoss()
	{
		m_boss = new UT_Interrupt();
		m_depth = 0;
	}
	SOPBoss(UT_Interrupt* boss)
	{
		m_boss = boss;
		m_depth = 0;
	}

	int opStart(const char* name)
	{
		m_depth++;
		return m_boss->opStart(name);
	}

	void opEnd()
	{
		m_boss->opEnd();
		m_depth--;
	}

	int opInterrupt()
	{
		return m_boss->opInterrupt();
	}
	int opInterrupt(int i, int N)
	{
		return m_boss->opInterrupt(100*i/N);
	}
	int opInterrupt(int procent)
	{
		return m_boss->opInterrupt(procent);
	}

	void exit()
	{
		while(m_depth > 0)
			opEnd();
	}
};


class SOPException : public std::exception
{
public:
	const char* m_func;
	const char* m_err;
	const char* m_warning;

public:
	SOPException(const char* func, const char* err, const char* warning) throw()
	{
		m_func = func;
		m_err = err;
		m_warning = warning;
	}
	virtual ~SOPException() throw()
	{
	}

	virtual const char* what() const throw ()
	{
		return m_err;
	}
};











