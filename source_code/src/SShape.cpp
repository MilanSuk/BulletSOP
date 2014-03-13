/**
BulletSOP v2.0
Copyright (c)2010-2013 Suk Milan. All Rights Reserved.
This software is provided 'as-is', without any express or implied warranty.
In no event will the authors be held liable for any damages arising from the use of this software.
This file may not be redistributed in whole or significant part.
This notice may not be removed or altered from any source distribution.
*/
#include "stdafx.h"
#include "SShape.h"
#include "SObject.h"


SShape::SShape(void)
{
}

SShape::~SShape(void)
{
}

void SShape::init(SOPBoss* boss, GU_Detail* gdp, bool add_cindex)
{
	setBasic(boss, gdp);

	m_attr_bt_index_point = SHelper::addIntPointAttr(m_gdp, "bt_index", 1);
	m_attr_bt_index_prim = SHelper::addIntPrimitiveAttr(m_gdp, "bt_index", 1);

	m_attr_bt_max_index = SHelper::addIntDetailAttr(m_gdp, "bt_max_index", 1);
	m_attr_bt_padding = SHelper::addFloatDetailAttr(m_gdp, "bt_padding", 1);
	float bbox[3] = {1, 1, 1};
	m_attr_bt_bbox = SHelper::addFloatVectorDetailAttr(m_gdp, "bt_bbox", bbox);

	if(add_cindex)
	{
		m_attr_bt_cindex_point = SHelper::addIntPointAttr(m_gdp, "bt_cindex", 1);
		m_attr_bt_cindex_prim = SHelper::addIntPrimitiveAttr(m_gdp, "bt_cindex", 1);
	}

	m_gdp->addVariableName("bt_index", "BI");

	if(add_cindex)
		m_gdp->addVariableName("bt_cindex", "BCI");
}


void SShape::initFind(SOPBoss* boss, GU_Detail* gdp, bool detailAttrs)
{
	setBasic(boss, gdp);

	m_attr_bt_index_point = SHelper::findPointAttrI(m_gdp, "bt_index");
	m_attr_bt_index_prim = SHelper::findPrimitiveAttrI(m_gdp, "bt_index");

	m_attr_bt_max_index = SHelper::findDetailAttrI(m_gdp, "bt_max_index");
	m_attr_bt_padding = SHelper::findDetailAttrF(m_gdp, "bt_padding");
	m_attr_bt_bbox = SHelper::findDetailAttrV3(m_gdp, "bt_bbox");


	//option
	m_attr_bt_cindex_point = SHelper::findPointAttrI(m_gdp, "bt_cindex");
	m_attr_bt_cindex_prim = SHelper::findPrimitiveAttrI(m_gdp, "bt_cindex");
	m_attr_N = SHelper::findPointAttrV3(m_gdp, "N");
	m_attr_Up = SHelper::findPointAttrV3(m_gdp, "up");

	if(m_attr_bt_index_point.isInvalid())		THROW_SOP("SShape: No \"bt_index\" point attribute", 0);
	if(m_attr_bt_index_prim.isInvalid())		THROW_SOP("SShape: No \"bt_index\" primitive attribute", 0);

	if(detailAttrs)
	{
		if(m_attr_bt_max_index.isInvalid())	THROW_SOP("SShape: No \"bt_max_index\" detail attribute", 0);
		if(m_attr_bt_padding.isInvalid())	THROW_SOP("SShape: No \"bt_padding\" detail attribute", 0);
	}
}


bool SShape::hasCompoundIndexAttr() const
{
	return !(m_attr_bt_cindex_point.isInvalid() || m_attr_bt_cindex_prim.isInvalid());
}
bool SShape::hasNandUpAttribs() const
{
	return !(m_attr_N.isInvalid() || m_attr_Up.isInvalid());
}



int SShape::getNumPoints()
{
	return (int)m_gdp->getPointMap().indexSize();
}
int SShape::getNumPrims()
{
	return (int)m_gdp->getPrimitiveMap().indexSize();
}

GU_Detail* SShape::getGeo()
{
	return m_gdp;
}




UT_Vector3 SShape::getN(const GA_Offset &off) const
{
	return m_attr_N(off);
}
UT_Vector3 SShape::getUp(const GA_Offset &off) const
{
	return m_attr_Up(off);
}
int SShape::getPointIndex(const GA_Offset &off) const
{
	return m_attr_bt_index_point(off);
}
int SShape::getPrimitiveIndex(const GA_Offset &off) const
{
	return m_attr_bt_index_prim(off);
}
int SShape::getPointCIndex(const GA_Offset &off) const
{
	if(m_attr_bt_cindex_point.isInvalid())
		return 0;
	return m_attr_bt_cindex_point(off);
}
int SShape::getPrimitiveCIndex(const GA_Offset &off) const
{
	if(m_attr_bt_cindex_prim.isInvalid())
		return 0;
	return m_attr_bt_cindex_prim(off);
}



void SShape::setPointIndex(const GA_Offset &off, int i)
{
	m_attr_bt_index_point.set(off, i);
}
void SShape::setPrimitiveIndex(const GA_Offset &off, int i)
{
	m_attr_bt_index_prim.set(off, i);
}


void SShape::setPointCIndex(const GA_Offset &off, int i)
{
	if(!m_attr_bt_cindex_point.isInvalid())
		m_attr_bt_cindex_point.set(off, i);
}
void SShape::setPrimitiveCIndex(const GA_Offset &off, int i)
{
	if(!m_attr_bt_cindex_prim.isInvalid())
		m_attr_bt_cindex_prim.set(off, i);
}




int SShape::updateMaxIndex()
{
	int max = getMaxIndex();
	setMaxIndex(max);
	return max;
}

int SShape::getMaxIndex() const
{
	int i;
	int next = 0;
	GEO_Primitive* pr;
	GA_Offset ptoff;

	GA_FOR_ALL_PTOFF(m_gdp, ptoff)
	{
		i = getPointIndex(ptoff);
		if(i > next)
			next = i;
	}

	GA_FOR_ALL_PRIMITIVES(m_gdp, pr)
	{
		i = getPrimitiveIndex(pr->getMapOffset());
		if(i > next)
			next = i;
	}

	if(hasCompoundIndexAttr())
	{
		GA_FOR_ALL_PTOFF(m_gdp, ptoff)
		{
			i = getPointCIndex(ptoff);
			if(i > next)
				next = i;
		}

		GA_FOR_ALL_PRIMITIVES(m_gdp, pr)
		{
			int index = getPrimitiveCIndex(pr->getMapOffset());
			if(index > next)
				next = index;
		}
	}

	return next;
}
void SShape::setMaxIndex(int max)
{
	m_attr_bt_max_index.set(0, max);
}



float SShape::getPadding() const
{
	if(m_attr_bt_padding.isInvalid())
		return 0.01f;
	return m_attr_bt_padding(0);
}
void SShape::setPadding(float padding)
{
	m_attr_bt_padding.set(0, padding);
}


UT_Vector3 SShape::getBBox() const
{
	if(m_attr_bt_bbox.isInvalid())
		return UT_Vector3(1,1,1);
	return m_attr_bt_bbox(0);
}
void SShape::setBBox(const UT_Vector3 bbox)
{
	m_attr_bt_bbox.set(0, bbox);
}




///check If group name has right structure
int isBulletPiece(GA_PrimitiveGroup* gr, const char* group_name)
{
	if(!gr->isEmpty())
	{
		UT_String name;
		name.harden(gr->getName());
		if(name.substitute(group_name, "") > 0 )	//work only with our groups
			if(name.isInteger() )
				return name.toInt();	//convert group number(X in "groupnameX") to integer
	}
	return -1;
}
int getNumGroups(GU_Detail* gdp)
{
	int num_groups = 0;
	GA_PrimitiveGroup* curr;
	GA_FOR_ALL_PRIMGROUPS(gdp, curr)
		num_groups++;
	return num_groups;
}




void SShape::fillAtrributeGroup(int start_i, UT_String group_name, bool deletePieceGroups)
{
	BOSSP_START;

	GA_PrimitiveGroup* curr;
	int num_group = getNumGroups(m_gdp);
	MyVec<GA_PrimitiveGroup*> destroy_groups;
	destroy_groups.setExtraAlloc(EXTRA_ALLOC);

	int i=0;
	int index = start_i;
	GA_FOR_ALL_PRIMGROUPS(m_gdp, curr)	//iterate over all groups
	{
		if( isBulletPiece(curr, group_name.buffer()) != -1 )
		{
			if(curr->entries())	//is not empty
			{
				GEO_Primitive *prim;
				GA_FOR_ALL_OPT_GROUP_PRIMITIVES(m_gdp, curr, prim)	//iterate over all primitives in group
				{
					setPrimitiveIndex(prim->getMapOffset(), index);

					GA_Primitive::const_iterator itv;
					for(prim->beginVertex(itv); !itv.atEnd(); ++itv)	//iterate over all points in primitive
						setPointIndex(itv.getPointOffset(), index);

				}

				index++;
			}
			destroy_groups.push_back(curr);
		}

		BOSSP_INTERRUPT(i, num_group);
		i++;
	}

	if(deletePieceGroups)
	{
		for(int i=0; i < destroy_groups.size(); i++)
			m_gdp->primitiveGroups().destroy( destroy_groups[i] );
	}


	updateMaxIndex();

	destroy_groups.clear();
	BOSSP_END;
}





void SShape::fillAtrributeCopyPoints(int start_i, int copy)
{
	BOSSP_START;

	int index;

	GA_Offset ptoff;

	int i = 0;
	index = start_i-1;
	GA_FOR_ALL_PTOFF(m_gdp, ptoff)
	{
		if(i%copy==0)
			index++;

		setPointIndex(ptoff, index);

		BOSSP_INTERRUPT_EMPTY;
		i++;
	}

	updateMaxIndex();

	BOSSP_END;
}



void SShape::fillAtrributeCopyPrimitives(int start_i, int copy)
{
	BOSSP_START;

	int index;

	GEO_Primitive* prim;
	int i = 0;
	index = start_i-1;
	GA_FOR_ALL_PRIMITIVES(m_gdp, prim)
	{
		if(i%copy==0)
			index++;

		setPrimitiveIndex(prim->getMapOffset(), index);

		BOSSP_INTERRUPT_EMPTY;
		i++;
	}

	updateMaxIndex();

	BOSSP_END;
}





void SShape::setCompounds(int num, int start_index)
{
	if(num<=0)
		return;

	BOSSP_START;

	int next_index = getMaxIndex()+1;
	
	//points
	GA_Offset ptoff;
	GA_FOR_ALL_PTOFF(m_gdp, ptoff)
	{
		int index = getPointIndex(ptoff);
		int compound_index = next_index + (index - start_index) / num;
		setPointCIndex(ptoff, compound_index);
	}

	//primitives
	GEO_Primitive* prim;
	GA_FOR_ALL_PRIMITIVES(m_gdp, prim)
	{
		int index = getPrimitiveIndex(prim->getMapOffset());
		int compound_index = next_index + (index - start_index) / num;
		setPrimitiveCIndex(prim->getMapOffset(), compound_index);
	}

	updateMaxIndex();

	BOSSP_END;
}




void SShape::setCompoundsFirst(int num, int start_index)
{
	if(num<=0)
		return;

	BOSSP_START;
	
	//points
	GA_Offset ptoff;
	GA_FOR_ALL_PTOFF(m_gdp, ptoff)
	{
		int index = getPointIndex(ptoff);
		int compound_index = index - (index - start_index) % num;

		if(compound_index==index)
			compound_index = 0;	//-1

		setPointCIndex(ptoff, compound_index);
	}

	//primitives
	GEO_Primitive* prim;
	GA_FOR_ALL_PRIMITIVES(m_gdp, prim)
	{
		int index = getPrimitiveIndex(prim->getMapOffset());
		int compound_index = index - (index - start_index) % num;

		if(compound_index==index)
			compound_index = 0;	//-1;

		setPrimitiveCIndex(prim->getMapOffset(), compound_index);
	}


	BOSSP_END;
}




///@returns normal on point
///method for computing normal is based on angles betwwen edges from point
UT_Vector3 getWeightNormal(GU_Detail* gdp, GA_Offset pt_offset, float padding)
{
	GEO_PrimPoly *poly;
	GEO_PrimPoly* last_poly;
	
	UT_Vector3 pt_pos = gdp->getPos3(pt_offset);
	UT_Vector3 line_s = pt_pos;
	UT_Vector3 line_n(0,0,0);

	GA_OffsetArray prims;
	gdp->getPrimitivesReferencingPoint(prims, pt_offset);
	for(GA_OffsetArray::const_iterator prims_it = prims.begin(); !prims_it.atEnd(); ++prims_it)
	{
		poly = (GEO_PrimPoly*)gdp->getPrimitiveList().get(*prims_it);

	    UT_Vector3 edges[2];
	    int num_edges = 0;
	    UT_Vector3 poly_normal = poly->computeNormal();

		GA_Primitive::const_iterator it;
		for(poly->beginVertex(it); !it.atEnd(); ++it)
		{
			GA_Offset ver_off = it.getPointOffset();
			if(ver_off != pt_offset && ( poly->hasDEdge(pt_offset, ver_off) || poly->hasDEdge(ver_off, pt_offset) ))
			{
				edges[num_edges] = pt_pos - gdp->getPos3(ver_off);
				num_edges++;
				if (num_edges >= 2)
					break;
			}
		}

	    if(num_edges >= 2)
		{
			edges[0].normalize();
			edges[1].normalize();
			float angle = SYSacos(edges[0].dot(edges[1]));
			angle = (float)SYSmin(angle, 2 * M_PI - angle);
			angle = (float)SYSmax(0.0, angle);
			line_n += poly_normal * angle;
		}

		last_poly = poly;
	}
	line_n.normalize();

	UT_Vector3 plane_n = last_poly->computeNormal();
	UT_Vector3 plane_p = (UT_Vector4)pt_pos - last_poly->computeNormal()*padding;

	//compute intersection of ray(v_s, v_n) with plane(p_p, p_n)
	float t = (plane_p - line_s).dot(plane_n) / line_n.dot(plane_n);
	return -line_n * t;
}



///@returns normal on point
///method for computing normal is based on computing intersection between 3 planes(shift to inside -> pading)
UT_Vector3 getWeightNormal2(GU_Detail* gdp, GA_Offset pt_offset, float padding)
{
	GEO_PrimPoly *poly;
	GEO_PrimPoly* last_poly;

	//get 3 different planes
	UT_Vector3 plane_n[3];
	int num_planes = 0;

	GA_OffsetArray prims;
	gdp->getPrimitivesReferencingPoint(prims, pt_offset);
	for(GA_OffsetArray::const_iterator prims_it = prims.begin(); !prims_it.atEnd(); ++prims_it)
	{
		poly = (GEO_PrimPoly*)gdp->getPrimitiveList().get(*prims_it);

		last_poly = poly;
	    UT_Vector3 nor = poly->computeNormal();

		bool same = false;
		const float TOLER = (float)cos(M_PI/180 * 20);	//20degrees

		float higher_cosA = -1.0f;
		int higher_i = -1;

		for(int j=0; j < num_planes; j++)
		{
			float cosA = absoluteValue(plane_n[j].dot(nor));
			if( cosA > TOLER )
			{
				same = true;
				break;
			}

			if(cosA > higher_cosA)
			{
				higher_cosA = cosA;
				higher_i = j;
			}
		}
		if(!same)
		{
			if(num_planes==3 && higher_i > -1)
			{
				plane_n[higher_i] = nor;
			}
			else
			{
				plane_n[num_planes] = nor;
				num_planes++;
			}
		}
	}
	

	float d[3];
	//UT_Vector3 p = pt->getPos();
	UT_Vector3 p = gdp->getPos3(pt_offset);

	for(int j=0; j < num_planes; j++)
		d[j] = p.dot(plane_n[j]) - padding;	//move inside

	//If we don't have 3rd plane, we comput it
	if(num_planes==2)
	{
		plane_n[num_planes] = cross(plane_n[0], plane_n[1]);
		d[num_planes] = p.dot(plane_n[num_planes]);
		num_planes++;
	}


	//compute intersection
	if(num_planes==3)
	{
		//base on http://paulbourke.net/geometry/pointlineplane/ (bottom page)
		UT_Vector3 t1 = cross(plane_n[1], plane_n[2])*d[0] + 
						cross(plane_n[2], plane_n[0])*d[1] + 
						cross(plane_n[0], plane_n[1])*d[2];

		float t2 = plane_n[0].dot( cross(plane_n[1], plane_n[2]) );

		if(t2)
		{
			UT_Vector3 planes_intersection = t1 /t2;
			return p - planes_intersection;
		}
	}

	return last_poly->computeNormal()*padding;
}


#include <GQ/GQ_Stitch.h>
#include <GQ/GQ_PolyDelaunay.h>

void SShape::shiftGeo(GU_Detail* original, float padding, int normal_type)
{
	setPadding(padding);

	if(padding==0)
		return;
	
	if(m_gdp->getNumPrimitives()==0)
		return;


	BOSSP_START;

	int i = 0;
	int N = (int)m_gdp->getPointMap().indexSize();
	GA_Offset ptoff;
	GA_FOR_ALL_PTOFF(original, ptoff)	//iterate over original
	{
		//get normal
		UT_Vector3 n;
		if(normal_type==0)	n = getWeightNormal(original, ptoff, padding);
		else
		if(normal_type==1)	n = getWeightNormal2(original, ptoff, padding);

		//set new position to m_gdp
		m_gdp->setPos3(m_gdp->pointOffset(i), original->getPos3(ptoff) - n);

		BOSSP_INTERRUPT(i, N);
		i++;
	}

	BOSSP_END;
}



void SShape::convertToHighPoly()
{
	BOSSP_START;

	GA_Offset ptoff;
	GA_FOR_ALL_PTOFF(m_gdp, ptoff)
		setPointIndex(ptoff, getPointIndex(ptoff)*-1);

	GEO_Primitive* prim;
	GA_FOR_ALL_PRIMITIVES(m_gdp, prim)
		setPrimitiveIndex(prim->getMapOffset(), getPrimitiveIndex(prim->getMapOffset())*-1);

	BOSSP_END;
}




void SShape::createConvexGroup(bool advance, float toler)
{
	BOSSP_START;

	GU_Detail objectGeo;
	SObject object;
	object.init(m_boss, &objectGeo, this);

	//set If object is convex/concave to object structure
	object.computeConvex(advance, toler, true);	

	//create groups
	GA_PrimitiveGroup* group_prConvex = m_gdp->newPrimitiveGroup("bulletConvex", false);
	GA_PrimitiveGroup* group_prConcave = m_gdp->newPrimitiveGroup("bulletConcave", false);
	GA_PointGroup* group_ptConvex = m_gdp->newPointGroup("bulletConvex", false);
	GA_PointGroup* group_ptConcave = m_gdp->newPointGroup("bulletConcave", false);

	//add points to groups
	GA_Offset ptoff;
	GA_FOR_ALL_PTOFF(m_gdp, ptoff)
	{
		int index = getPointIndex(ptoff);
		bool convex = object.getConvex(index);
	
		if(convex)	group_ptConvex->addOffset(ptoff);
		else		group_ptConcave->addOffset(ptoff);
	}

	BOSSP_INTERRUPT_EMPTY;

	//add primitives to groups
	GEO_Primitive* prim;
	GA_FOR_ALL_PRIMITIVES(m_gdp, prim)
	{
		int index = getPrimitiveIndex(prim->getMapOffset());
		bool convex = object.getConvex(index);
	
		if(convex)	group_prConvex->add(prim);
		else		group_prConcave->add(prim);
	}


	BOSSP_END;
}


///send ray and loop until hit "no_hit_prim"
float sendRay(GU_RayIntersect *ray, UT_Vector3 start, UT_Vector3 dir, GEO_Primitive* no_hit_prim)
{
	bool done = false;
	float t;
	while(!done)
	{
		t = 0.01f;
		GU_RayInfo info;
		info.myPrim = no_hit_prim;
		if(ray->sendRay(start, dir, info) > 0)
		{
			if(info.myPrim!=no_hit_prim)
			{
				t = info.myT;
				done = true;
			}
		}
		else
			done = true;
		start += dir * t;
	}

	return t;
}


///divides ray(according raySteps) and them changes "end" to this points and check If new ray has collision
UT_Vector3 stepRay(GEO_Primitive* prim, GU_RayIntersect *ray, UT_Vector3 start, UT_Vector3 end, int raySteps=10)
{
	UT_Vector3 dir = end - start;

	for(float i=(float)raySteps; i > 0; i--)
	{
		UT_Vector3 end2 = start + (end-start)*(i/raySteps);	//compute new end
		bool hit = false;
		for(int v=0; v < prim->getVertexCount(); v++ )
		{
			UT_Vector3 sv = getPrimPos3(prim, v);;	//new start
			UT_Vector3 dv = end2 - sv;										//new vec(dir)

			sv += dv * 0.001f;
			if(sendRay(ray, sv, dv, prim) < 0.999f)	//if something is hit
			{
				hit = true;
				break;
			}
		}

		if(!hit)	//if All edges don't hit anything, this end is OK
			return end2;
	}

	return start + dir/raySteps;	//step by 1.0f/raySteps
}



///creates Tetra hedra geometry from base points and up_point vertex
///@returns list of new Primitives
void createTetra(MyVec<GEO_PrimPoly*> &ret, GU_Detail* geo, const MyVec<UT_Vector3> &base, UT_Vector3 upPoint)
{
	//create base poly
	GU_PrimPoly* prim_base = GU_PrimPoly::build(geo, (int)base.size(), GU_POLY_CLOSED);
	for(int v=0; v < base.size(); v++ )
		geo->setPos3( prim_base->getPointOffset(v), base[v]);
	ret.push_back(prim_base);


	//create up vertex
	GA_Offset ptUp_off = geo->appendPointOffset();
	geo->setPos3(ptUp_off, upPoint);


	//create polys between base_poly and end point
	int old_v = (int)base.size()-1;
	for(int v=0; v < base.size(); v++ )	
	{
		GA_PrimitiveTypeId p(GA_PRIMPOLY);
		GEO_PrimPoly* prim2 = dynamic_cast<GEO_PrimPoly *>(geo->appendPrimitive(p));
		prim2->setSize(0);

		prim2->appendVertex(prim_base->getPointOffset(v));
		prim2->appendVertex(prim_base->getPointOffset(old_v));
		prim2->appendVertex(ptUp_off);

		prim2->close();
		old_v = v;

		ret.push_back(prim2);
	}
}



void SShape::decompositionTetra(int raySteps, bool includeCompoundGeo)
{
	BOSSP_START;

	int next_index = getMaxIndex()+1;

	GU_Detail decompGeo;
	SShape myDecompGeo;
	myDecompGeo.init(m_boss, &decompGeo, true);

	//get group
	GA_PrimitiveGroup* group_prConcave = m_gdp->findPrimitiveGroup("bulletConcave");

	//instance ray class
	GU_RayIntersect *ray = new GU_RayIntersect;
	ray->init(m_gdp, group_prConcave);


	int N = (int)m_gdp->getPrimitiveMap().indexSize();
	int i = 0;
	GEO_Primitive* prim;
	MyVec<GEO_PrimPoly*> tetraPrims;	tetraPrims.setExtraAlloc(EXTRA_ALLOC);
	MyVec<UT_Vector3> base;				base.setExtraAlloc(EXTRA_ALLOC);

	GA_FOR_ALL_GROUP_PRIMITIVES(m_gdp, group_prConcave, prim)
	{
		if(prim->calcArea() < 0.001f)
			continue;

		UT_Vector3 dir = prim->computeNormal() * -1;

		//compute start of ray
		UT_Vector3 start(0,0,0);
		for(int v=0; v < prim->getVertexCount(); v++ )
			start += getPrimPos3(prim, v);
		start /= (float)prim->getVertexCount();

		//compute end => raytracing
		UT_Vector3 end = start + dir * sendRay(ray, start, dir, prim);

		//step ray
		end = stepRay(prim, ray, start, end, raySteps);

		//create tetra
		base.resize(0);
		for(int v=0; v < prim->getVertexCount(); v++ )
			base.push_back( getPrimPos3(prim, v) );
		
		tetraPrims.resize(0);
		createTetra(tetraPrims, &decompGeo, base, end);

		//set indexes for new objects
		int indexCurrent = getPrimitiveIndex(prim->getMapOffset());
		for(int j=0; j < tetraPrims.size(); j++)
		{
			GEO_PrimPoly* prim2 = tetraPrims[j];
			myDecompGeo.setPrimitiveCIndex(prim2->getMapOffset(), indexCurrent);
			myDecompGeo.setPrimitiveIndex(prim2->getMapOffset(), next_index);

			for(int v=0; v < prim2->getVertexCount(); v++)
			{
				GA_Offset ptoffv = prim2->getPointOffset(v);
				myDecompGeo.setPointCIndex(ptoffv, indexCurrent);
				myDecompGeo.setPointIndex(ptoffv, next_index);
			}
		}
		
		BOSSP_INTERRUPT(i, N);
		i++;

		next_index++;
	}

	//delete original concave objects
	if(!includeCompoundGeo)
		m_gdp->deletePrimitives(*group_prConcave, true);

	//add tetras to m_gdp
	m_gdp->copy(decompGeo, GEO_COPY_ADD);

	//set detail attributes
	setPadding( getPadding() );
	updateMaxIndex();

	BOSSP_END;
}




void SShape::decompositionVoxel(float cellSize, float padding, bool includeCompoundGeo)
{
	BOSSP_START;

	int next_index = getMaxIndex()+1;

	GU_Detail decompGeo;
	SShape myDecompGeo;
	myDecompGeo.init(m_boss, &decompGeo, true);

	//get group
	GA_PrimitiveGroup* group_prConcave = m_gdp->findPrimitiveGroup("bulletConcave");

	//instance ray class for computing the closest point on geometry
	GU_RayIntersect *ray = new GU_RayIntersect;
	ray->init(m_gdp, group_prConcave);

	//get bbox and compute size
	UT_BoundingBox bbox;
	m_gdp->getBBox( &bbox, group_prConcave );
	UT_Vector3I size = getVector3Int(bbox.size() / cellSize);

	//real padding
	padding += cellSize;	

	//create voxels
	const int N = (int)size.y();
	for(int y = 0; y < size.y(); y++)
	{
		for(int z = 0; z < size.z(); z++)
		{
			for(int x = 0; x < size.x(); x++)
			{
				UT_Vector3 pos = (UT_Vector3)bbox.minvec() + UT_Vector3((float)x, (float)y, (float)z)*cellSize;

				GU_MinInfo infoMin;
				if(ray->minimumPoint(pos, infoMin, 0) > 0)	//try to get the closest point on geometry
				{
					UT_Vector4 end;
					infoMin.prim->evaluateInteriorPoint(end, infoMin.u1, infoMin.v1);	//compute minumum distance position
					
					UT_Vector3 v = (UT_Vector3)end-pos;
					UT_Vector3 vn = v;
					vn.normalize();

					if(infoMin.prim->computeNormal().dot(vn) > 0)	//If Original point is inside
					{
						float dist = v.length();
						if(dist > padding)	//compare distances
						{
							//add and set point
							GA_Offset ptoff = decompGeo.insertPointOffset();
							decompGeo.setPos3(ptoff, pos);

							//set bt_index and bt_cindex
							int index = getPrimitiveIndex(infoMin.prim->getMapOffset());
							myDecompGeo.setPointCIndex(ptoff, index);
							myDecompGeo.setPointIndex(ptoff, next_index);

							next_index++;
						}
					}
				}
			}
		}

		BOSSP_INTERRUPT(y, N);
	}

	//delete original concave objects
	if(!includeCompoundGeo)
		m_gdp->deletePrimitives(*group_prConcave, true);


	//add tetras to m_gdp
	m_gdp->copy(decompGeo, GEO_COPY_ADD);

	//set detail attributes
	setPadding(padding);
	updateMaxIndex();
	setBBox(UT_Vector3(cellSize, cellSize, cellSize));

	BOSSP_END;
}



void SShape::deleteHigh()
{
	BOSSP_START;

	//delete primitives
	GEO_PrimitivePtrArray delArray;
	GEO_Primitive* prim;
	GA_FOR_ALL_PRIMITIVES(m_gdp, prim)
	{
		int index = getPrimitiveIndex(prim->getMapOffset());
		if(index >= 0)
			delArray.append(prim);

		BOSSP_INTERRUPT_EMPTY;
	}
	m_gdp->deletePrimitives(delArray, true);	//real deleting
	
	//delete points
	MyVec<GA_Offset> delPoints;
	delPoints.setExtraAlloc(EXTRA_ALLOC);

	GA_Offset ptoff;
	GA_FOR_ALL_PTOFF(m_gdp, ptoff)
	{
		int index = getPointIndex(ptoff);
		if(index >= 0)
			delPoints.push_back(ptoff);

		BOSSP_INTERRUPT_EMPTY;
	}

	for(int i=0; i < delPoints.size(); i++)
		m_gdp->destroyPointOffset(delPoints[i]);

	BOSSP_END;
}



void SShape::deleteLow()
{
	BOSSP_START;

	//delete primitives
	GEO_PrimitivePtrArray delArray;
	GEO_Primitive* prim;
	GA_FOR_ALL_PRIMITIVES(m_gdp, prim)
	{
		int index = getPrimitiveIndex(prim->getMapOffset());
		if(index <= 0)
			delArray.append(prim);

		BOSSP_INTERRUPT_EMPTY;
	}
	m_gdp->deletePrimitives(delArray, true);	//real deleting
	
	//delete points
	MyVec<GA_Offset> delPoints;
	delPoints.setExtraAlloc(EXTRA_ALLOC);

	GA_Offset ptoff;
	GA_FOR_ALL_PTOFF(m_gdp, ptoff)
	{
		int index = getPointIndex(ptoff);
		if(index <= 0)
			delPoints.push_back(ptoff);

		BOSSP_INTERRUPT_EMPTY;
	}
	for(int i=0; i < delPoints.size(); i++)
		m_gdp->destroyPointOffset(delPoints[i]);


	BOSSP_END;
}



void SShape::deleteSubCompund()
{
	BOSSP_START;

	if(hasCompoundIndexAttr())
	{
		//delete primitives
		GEO_PrimitivePtrArray delArray;
		GEO_Primitive* prim;
		GA_FOR_ALL_PRIMITIVES(m_gdp, prim)
		{
			int cindex = getPrimitiveCIndex(prim->getMapOffset());
			if(cindex > 0)
				delArray.append(prim);

			BOSSP_INTERRUPT_EMPTY;
		}
		m_gdp->deletePrimitives(delArray, true);	//real deleting

		//delete points
		MyVec<GA_Offset> delPoints;
		delPoints.setExtraAlloc(EXTRA_ALLOC);

		GA_Offset ptoff;
		GA_FOR_ALL_PTOFF(m_gdp, ptoff)
		{
			int cindex = getPointCIndex(ptoff);
			if(cindex > 0)
				delPoints.push_back(ptoff);

			BOSSP_INTERRUPT_EMPTY;
		}
		for(int i=0; i < delPoints.size(); i++)
			m_gdp->destroyPointOffset(delPoints[i]);
	}

	BOSSP_END;
}


