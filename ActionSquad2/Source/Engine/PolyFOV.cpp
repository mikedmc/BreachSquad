#include "dxstdafx.h"

int CPolyFOV::OccluderNodesComparer(const void* a, const void* b)
{
	const COccluderNode* ln1 = static_cast<const COccluderNode*>(a);
	const COccluderNode* ln2 = static_cast<const COccluderNode*>(b);

	//TODO: aici trebuie verificat neaparat daca le converteste bine si daca le sorteaza cum trebuie

	if (ln1->angle < ln2->angle)
		return -1;
	else if (ln1->angle > ln2->angle)
		return 1;
	// But for ties (common), we want Begin nodes before End nodes
	if (!ln1->start && ln2->start) return 1;
	if (ln1->start && !ln2->start) return -1;
	return 0;
}

bool COccluder::PointOnLeft(Vec2 &p)
{
	float cross = (end.x - start.x) * (p.y - start.y) - (end.y - start.y) * (p.x - start.x);
	return cross < 0;
}

void CPolyFOV::AddPolySegment(double angle1, double angle2, COccluder *wall)
{
	//daca e degenerata nu o adauga	 - are rezultate stranii in unele cazuri deci mai bine nu mai verific
	//if (Math.Abs(angle1 - angle2) < 0.0001f)
	//  return;

	D3DXVECTOR2 p1 = m_vViewerPos;
	D3DXVECTOR2 p2(m_vViewerPos.x + (float)cos((double)angle1), m_vViewerPos.y + (float)sin((double)angle1));
	D3DXVECTOR2 p3(0.0f, 0.0f);
	D3DXVECTOR2 p4(0.0f, 0.0f);

	if (wall != null)
	{
		// Stop the triangle at the intersecting segment
		p3.x = wall->start.x;
		p3.y = wall->start.y;
		p4.x = wall->end.x;
		p4.y = wall->end.y;
	}
	else
	{
		// Stop the triangle at a fixed distance; this probably is
		// not what we want, but it never gets used in the demo
		p3.x = m_vViewerPos.x + (float)cos(angle1) * 500.0f;
		p3.y = m_vViewerPos.y + (float)sin(angle1) * 500.0f;
		p4.x = m_vViewerPos.x + (float)cos(angle2) * 500.0f;
		p4.y = m_vViewerPos.y + (float)sin(angle2) * 500.0f;
	}


	D3DXVECTOR2 pBegin, pEnd;
	bool ok1 = UTMath::LineLineIntersection(p3, p4, p1, p2, &pBegin);

	p2.x = m_vViewerPos.x + (float)cos(angle2);
	p2.y = m_vViewerPos.y + (float)sin(angle2);
	bool ok2 = UTMath::LineLineIntersection(p3, p4, p1, p2, &pEnd);

	if (ok1 && ok2)
	{
		assert(retTriangleBasesCnt < K_PFOV_MAX_FOV_TRIANGLES);

		retTriangleBases[retTriangleBasesCnt].start = pBegin;
		retTriangleBases[retTriangleBasesCnt].end = pEnd;
		retTriangleBasesCnt++;
	}
}


//TODO: aici poate fi optimizat in directX: verific daca cross-ul capetelor lui b sunt de aceeasi parte a lui a. Daca da, verific si ochiul sa fie de partea cealalta
//Trebuie sa accepte si 0.0f ca produs ca sa nu mai scurtez peretii care se intersecteaza
bool segment_in_front_of(COccluder a, COccluder b, D3DXVECTOR2 relativeTo)
{
	// NOTE: we slightly shorten the segments so that
	// intersections of the endpoints (common) don't count as
	// intersections in this algorithm

	//TODO: aici in loc de interpolare pot scoate versorul si sa il scad si adaug din capete ca sa fie mai rapid
	D3DXVECTOR2 tempvec;
	D3DXVec2Lerp(&tempvec, &b.start, &b.end, 0.01f);
	bool A1 = a.PointOnLeft(tempvec);
	D3DXVec2Lerp(&tempvec, &b.end, &b.start, 0.01f);
	bool A2 = a.PointOnLeft(tempvec);
	bool A3 = a.PointOnLeft(relativeTo);
	D3DXVec2Lerp(&tempvec, &a.start, &a.end, 0.01f);
	bool B1 = b.PointOnLeft(tempvec);
	D3DXVec2Lerp(&tempvec, &a.end, &a.start, 0.01f);
	bool B2 = b.PointOnLeft(tempvec);
	bool B3 = b.PointOnLeft(relativeTo);

	// NOTE: this algorithm is probably worthy of a short article
	// but for now, draw it on paper to see how it works. Consider
	// the line A1-A2. If both B1 and B2 are on one side and
	// relativeTo is on the other side, then A is in between the
	// viewer and B. We can do the same with B1-B2: if A1 and A2
	// are on one side, and relativeTo is on the other side, then
	// B is in between the viewer and A.
	if (B1 == B2 && B2 != B3) return true;
	if (A1 == A2 && A2 == A3) return true;
	if (A1 == A2 && A2 != A3) return false;
	if (B1 == B2 && B2 == B3) return false;

	// If A1 != A2 and B1 != B2 then we have an intersection.
	// Expose it for the GUI to show a message. A more robust
	// implementation would split segments at intersections so
	// that part of the segment is in front and part is behind.
	//demo_intersectionsDetected.push([a.p1, a.p2, b.p1, b.p2]);
	return false;

	// NOTE: previous implementation was a.d < b.d. That's simpler
	// but trouble when the segments are of dissimilar sizes. If
	// you're on a grid and the segments are similarly sized, then
	// using distance will be a simpler and faster implementation.
}


void CPolyFOV::SetOccluders(D3DXVECTOR2 & vViewerPos, COccluder * p_arrOccluders, int nOccludersCount)
{
	assert(nOccludersCount < K_PFOV_MAX_OCCLUDERS);
	//salvez pozitie viewer
	m_vViewerPos = vViewerPos;
	//copiez date occluders
	memcpy(occluders, p_arrOccluders, sizeof(COccluder) * nOccludersCount);
	//salvez nr de occluders
	occludersCnt = nOccludersCount;
}

CPolyFOV::CSegment* CPolyFOV::ComputeFOV(int &retBasesCnt)
{
	//reset return array counter
	retTriangleBasesCnt = 0;

	CLinkedList<COccluder*> open;

	if (occludersCnt <= 0)
	{
		retBasesCnt = 0;
		return NULL;
	}

	//TODO: poate ar merge mai repede fara alocare dinamica... ?
	COccluderNode *checkNodes = new COccluderNode[occludersCnt * 2];  //punctele prin care trec razele, sortate dupa unghi
	int checkNodesCnt = 0;

	//1. se adauga toate punctele prin care trece sweep-ray, sortate dupa unghi. Se seteaza si start-end in fn de unghi
	//TODO: calculul asta se face doar cand se schimba pozitia luminii... poate foloseste la vreo optimizare
	//pt algoritm vezi: https://briangordon.github.io/2014/08/the-skyline-problem.html
	for (int kk = 0; kk < occludersCnt; kk++)
	{
		COccluder* wall = &occluders[kk];
		//excludem peretii coliniari cu lumina
		//TODO: nu stiu daca nu cumva face sa mearga mai greu verificarea coliniaritatii
		//if (PointsAreCollinear(lightPos, wall.start, wall.end))
		//continue;

		COccluderNode *n1 = &checkNodes[checkNodesCnt++];
		n1->pos.x = wall->start.x; n1->pos.y = wall->start.y;
		n1->parentWall = wall; n1->angle = atan2((double)(n1->pos.y - m_vViewerPos.y), (double)(n1->pos.x - m_vViewerPos.x));
		if (n1->angle < 0.0f)
			n1->angle += DOUBLE_PI; //aduce unghiul in 0-2pi - se poate optimiza?

		COccluderNode *n2 = &checkNodes[checkNodesCnt++];
		n2->pos.x = wall->end.x; n2->pos.y = wall->end.y;
		n2->parentWall = wall; n2->angle = atan2((double)(n2->pos.y - m_vViewerPos.y), (double)(n2->pos.x - m_vViewerPos.x));
		if (n2->angle < 0.0f)
			n2->angle += DOUBLE_PI;

		//seteaza start clockwise - NECESAR
		double dAngle = n2->angle - n1->angle;
		if (dAngle <= -PI) { dAngle += 2 * PI; }
		if (dAngle > PI) { dAngle -= 2 * PI; }
		n1->start = (dAngle > 0.0f);
		n2->start = !n1->start;
	}

	//2.sorteaza punctele dupa unghi (folosesc o lambda pt regula de sortare)
	qsort(checkNodes, checkNodesCnt, sizeof(COccluderNode), OccluderNodesComparer);

	//acesta este cursorul curent. cand se schimba se adauga triunghi
	COccluder* lastWall = NULL;
	double lastAngle = 0.0f;

	//pasul 1 - seteaza unghiurile si enable-urile peretilor
	for (int kk = 0; kk < checkNodesCnt; kk++)
	{
		COccluderNode* node = &checkNodes[kk];
		//daca e nod de start se activeaza peretele. daca este de end se dezactiveaza peretele

		COccluder* nearestWall = null;
		if (open.Count() > 0)
		{
			nearestWall = open.GetFirst()->m_data;
		}

		if (node->start)
		{
			// Insert into the right place in the list
			CLinkedList<COccluder*>::CLinkedListNode *listnode = open.GetFirst();
			while ((listnode != null) && (segment_in_front_of(*node->parentWall, *listnode->m_data, m_vViewerPos)))
			{
				listnode = listnode->m_pNext;
			}
			if (listnode == null)
			{
				open.AddLast(node->parentWall);
			}
			else
			{
				open.AddBefore(listnode, node->parentWall);
			}
		}
		else
		{
			open.Remove(node->parentWall);
		}

		lastWall = (open.Count() == 0) ? null : open.GetFirst()->m_data;
		if (nearestWall != lastWall)
		{
			lastAngle = node->angle;
		}
	}

	//pasul 2 - desenare
	for (int kk = 0; kk < checkNodesCnt; kk++)
	{
		COccluderNode *node = &checkNodes[kk];
		//daca e nod de start se activeaza peretele. daca este de end se dezactiveaza peretele

		COccluder *nearestWall = (open.Count() == 0) ? null : open.GetFirst()->m_data;

		if (node->start)
		{
			// Insert into the right place in the list
			CLinkedList<COccluder*>::CLinkedListNode *listnode = open.GetFirst();
			while ((listnode != null) && (segment_in_front_of(*node->parentWall, *listnode->m_data, m_vViewerPos)))
			{
				listnode = listnode->m_pNext;
			}
			if (listnode == null)
			{
				open.AddLast(node->parentWall);
			}
			else
			{
				open.AddBefore(listnode, node->parentWall);
			}
		}
		else
		{
			open.Remove(node->parentWall);
		}

		lastWall = (open.Count() == 0) ? null : open.GetFirst()->m_data;
		if (nearestWall != lastWall)
		{
			AddPolySegment(lastAngle, node->angle, nearestWall);
			lastAngle = node->angle;
		}
	}
	//clear list
	open.RemoveAll();
	//release temp nodes
	SAFE_DELETE_ARRAY(checkNodes);
	//return values
	retBasesCnt = retTriangleBasesCnt;
	return retTriangleBases;
}


void COccluderSegment::Set(Vec2 vfrom, Vec2 vto, Vec2 vNrm, Vec2 vViewerPos)
{
	vStart = vfrom;
	vEnd = vto;
	vN = vNrm;
	// compute angles now
	fStartAng = atan2(vStart.y- vViewerPos.y, vStart.x - vViewerPos.x);
	fEndAng = atan2(vEnd.y - vViewerPos.y, vEnd.x - vViewerPos.x);
	//#TODO: try and measure faster approximated version: UTMath::atan2_approximation2
}

void COccluderSegment::ComputeAngles(Vec2 vViewerPos)
{
	fStartAng	= atan2(vStart.y - vViewerPos.y, vStart.x - vViewerPos.x);
	fEndAng		= atan2(vEnd.y - vViewerPos.y, vEnd.x - vViewerPos.x);
}
