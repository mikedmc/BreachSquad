#include "dxstdafx.h"
#include "CFOVUtil.h"
#include <algorithm> // std::sort

void COccluderSegment::Set(Vec2 vfrom, Vec2 vto, Vec2 vNrm, Vec2 vViewerPos, DWORD wallID, float wallH)
{
	vStart = vfrom;
	vEnd = vto;
	vN = vNrm;
	dwWallID = wallID;
	fWallH = wallH;
	// compute angles now
	fStartAng = atan2(vStart.y - vViewerPos.y, vStart.x - vViewerPos.x);
	fEndAng = atan2(vEnd.y - vViewerPos.y, vEnd.x - vViewerPos.x);
	//#TODO: try and measure faster approximated version: UTMath::atan2_approximation2
}

void COccluderSegment::MoveStart(Vec2 vPos, Vec2 vViewerPos)
{
	vStart = vPos;
	fStartAng = atan2(vStart.y - vViewerPos.y, vStart.x - vViewerPos.x);
}

void COccluderSegment::MoveEnd(Vec2 vPos, Vec2 vViewerPos)
{
	vEnd = vPos;
	fEndAng = atan2(vEnd.y - vViewerPos.y, vEnd.x - vViewerPos.x);
}

bool OccluderIntersectionSorter(sOccluderIntersection & a, sOccluderIntersection & b)
{
	return (a.fAngle < b.fAngle);
}

int FOVUtil::BuildOccludedVolume(Vec2 vEye, DWORD dwColor, COccluderSegment* arrOcc, int nOccludersCnt, _VERTEX_PNCT4T4 *outVerts, int outVertsMaxCnt)
{
	// send rays to each occluder end point and to +/-0.0001 rad of it to see the back collisions, sort collision points by angle and create poly from them
	// if lateral (corner) ray hits the same occluder we skip it as it means we're sending the ray inside the same poly
	// optimize by getting rid of the occluders which are not in the intersection path (by using the angles of the ends)
	//#TODO: could be optimized by having a small cache for the last 2 angles so we don't process same angle many times.

	const int nReservedSize = 200;
	std::vector<sOccluderIntersection> arrVerts;
	arrVerts.reserve(nReservedSize);

	for (int ii = 0; ii < nOccludersCnt; ii++)
	{
		COccluderSegment* occ = &arrOcc[ii];
		// ray to start
		Vec2 vRetPt;
		// do vStart then vEnd
		for (int ll = 0; ll < 2; ll++)
		{
			// selects occ->start then occ->end
			Vec2 vTarget;
			float fTargetAng;
			if (ll == 0)
			{
				vTarget = occ->vStart;
				fTargetAng = occ->fStartAng;
			}
			else
			{
				vTarget = occ->vEnd;
				fTargetAng = occ->fEndAng;
			}

			//1. trace to occluder end, register collision

			COccluderSegment* occcol = RayOccludersIntersection(vEye, vTarget, fTargetAng, arrOcc, nOccludersCnt, vRetPt);
			if (occcol)
			{
				arrVerts.push_back(sOccluderIntersection(vRetPt, fTargetAng, occcol->dwWallID, occcol->fWallH));
			}


			//2. check with ray at angle -0.0001 and +0.0001, excluding ray if it hits segment to detect collisions around corners (back collisions)
			/// 2.a. version 1: sends both rays and detects which one falls inside the source occluder to cancel it. 
			/// Occluders don't have to be defined clockwise (vStart before vEnd in CCW order)

			/*
			// left ray
			float fang = fTargetAng - 0.00001f;
			Vec2 vTo(100.0f * cos(fang) + vEye.x, 100.0f * sin(fang) + vEye.y);
			//make sure we don't send ray inside our current occluder, just outside (see hints above)
			if (!UTMath::RaySegmentIntersection(vEye, vTo, occ->vStart, occ->vEnd, nullptr))
			{
				COccluderSegment* retocc = RayOccludersIntersection(vEye, vTo, fang, arrOcc, nOccludersCnt, vRetPt);
				if (retocc)
				{
					arrVerts.push_back(sOccluderIntersection(vRetPt, fang, retocc->dwWallID, retocc->fWallH));
				}
			}
			// right ray
			fang = fTargetAng + 0.00001f;
			vTo = Vec2(100.0f * cos(fang) + vEye.x, 100.0f * sin(fang) + vEye.y);
			//make sure we don't send ray inside our current occluder, just outside (see hints above)
			if (!UTMath::RaySegmentIntersection(vEye, vTo, occ->vStart, occ->vEnd, nullptr))
			{
				COccluderSegment* retocc = RayOccludersIntersection(vEye, vTo, fang, arrOcc, nOccludersCnt, vRetPt);
				if (retocc)
				{
					arrVerts.push_back(sOccluderIntersection(vRetPt, fang, retocc->dwWallID, retocc->fWallH));
				}
			}
			*/
			
			
			/// 2.b. version 2:
			/// OCCLUDERS NEED TO BE DEFINED CLOCKWISE => vStart sends ray at -0.0001 rad, vEnd sends at angle + 0.0001 rad
			float fang = ( ll == 0 ) ? fTargetAng - 0.00001f : fTargetAng + 0.00001f;

			Vec2 vTo(1000.0f * cos(fang) + vEye.x, 1000.0f * sin(fang) + vEye.y);
			COccluderSegment* retocc = RayOccludersIntersection(vEye, vTo, fang, arrOcc, nOccludersCnt, vRetPt);
			if (retocc)
			{
				arrVerts.push_back(sOccluderIntersection(vRetPt, fang, retocc->dwWallID, retocc->fWallH));
			}
			
		}
	}
#if defined(_DEBUG) || defined(DEBUG)
	if (arrVerts.size() > nReservedSize)
	{
		LOG("[WARN] BuildOccludedVolume:: vector buffer too small!");
	}
#endif

	//4. sort std::vector by angle and add polygons (could be a trianglestrip but we need another buffered painter class for that)
	std::sort(arrVerts.begin(), arrVerts.end(), OccluderIntersectionSorter);

	//5. add triangles to geometry
	int nVertCnt = 0;
	int arrVertsSize = arrVerts.size();
	// cached data
	Vec3 vEye3D(vEye.x, vEye.y, 0.0f);
	Vec3 vWallH(0.0f, -K_WALL_HEIGHT_SCREEN, 0.0f);

	for (int kk = 1; kk <= arrVertsSize; kk++)
	{
		_ASSERT(nVertCnt < outVertsMaxCnt);

		sOccluderIntersection* pt = &arrVerts[kk % arrVertsSize];
		sOccluderIntersection* ptold = &arrVerts[(kk - 1) % arrVertsSize];

		Vec3 ptpos(pt->vPos.x, pt->vPos.y, 0.0f);
		Vec3 ptoldpos(ptold->vPos.x, ptold->vPos.y, 0.0f);

		outVerts[nVertCnt].pos = vEye3D; outVerts[nVertCnt].color = dwColor; nVertCnt++;
		outVerts[nVertCnt].pos = ptpos; outVerts[nVertCnt].color = dwColor; nVertCnt++;
		outVerts[nVertCnt].pos = ptoldpos; outVerts[nVertCnt].color = dwColor; nVertCnt++;

		// extend on wall
		if ((pt->fWallH > 0.0f) && (ptold->fWallH > 0.0f) && (pt->dwWallID == ptold->dwWallID))
		{
			outVerts[nVertCnt].pos = ptpos; outVerts[nVertCnt].color = dwColor; nVertCnt++;
			outVerts[nVertCnt].pos = ptpos; outVerts[nVertCnt].pos.y -= pt->fWallH; outVerts[nVertCnt].color = dwColor; nVertCnt++;
			outVerts[nVertCnt].pos = ptoldpos; outVerts[nVertCnt].color = dwColor; nVertCnt++;

			outVerts[nVertCnt].pos = ptpos; outVerts[nVertCnt].pos.y -= pt->fWallH; outVerts[nVertCnt].color = dwColor; nVertCnt++;
			outVerts[nVertCnt].pos = ptoldpos; outVerts[nVertCnt].color = dwColor; nVertCnt++;
			outVerts[nVertCnt].pos = ptoldpos; outVerts[nVertCnt].pos.y -= ptold->fWallH; outVerts[nVertCnt].color = dwColor; nVertCnt++;
		}
	}

	return nVertCnt;
}



COccluderSegment* FOVUtil::RayOccludersIntersection(Vec2 vEye, Vec2 vTo, float fRayAngle, COccluderSegment* arrOcc, int nOccludersCnt, Vec2 & vRetPt)
{
	float	minr = 100000.0f;
	float	colls = 0.0f;
	int		closestidx = -1;
	Vec2	vDir = vTo - vEye;

	int excluded = 0;
	for (int kk = 0; kk < nOccludersCnt; kk++)
	{
		float r, s;
		COccluderSegment* occto = &arrOcc[kk];
		//1.a. verify if segment can intersect (by angles and by !normal backface culling!) and all possible cheap things
		// backface culling (normals have same direction)
		if (MUVec2Dot(&occto->vN, &vDir) > 0.0f)
		{
			excluded++;
			continue;
		}
		// check by angle (see if ray is inside the solid angle created by the occluder). Might be expensive (12 ops worse case vs 20-22 ops the collision)
		// good and aggressive but doesn't work so well....
		/*
		float up1p2 = fabs(occto->fEndAng - occto->fStartAng); if (up1p2 > DOUBLE_PI) up1p2 = DOUBLE_PI - up1p2;
		float up1v = fabs(occto->fStartAng - fRayAngle); if (up1v > DOUBLE_PI) up1v = DOUBLE_PI - up1v;
		float up2v = fabs(occto->fEndAng - fRayAngle); if (up2v > DOUBLE_PI) up2v = DOUBLE_PI - up2v;
		if (up1v + up2v > up1p2)
		{
			excluded++;
			continue;
		}
		*/
		// exclude by quadrant position

		if (vDir.x > 0.0f)
		{
			if (occto->vStart.x < vEye.x && occto->vEnd.x < vEye.x)
				continue;
		}
		else if (vDir.x < 0.0f)
		{
			if (occto->vStart.x > vEye.x && occto->vEnd.x > vEye.x)
				continue;
		}

		if (vDir.y > 0.0f)
		{
			if (occto->vStart.y < vEye.y && occto->vEnd.y < vEye.y)
				continue;
		}
		else if (vDir.y < 0.0f)
		{
			if (occto->vStart.y > vEye.y && occto->vEnd.y > vEye.y)
				continue;
		}

		// 2. save smallest denominator of ray and occluder index to be fast, and compute point at the end
		if (UTMath::RaySegmentIntersection_denom(vEye, vTo, occto->vStart, occto->vEnd, r, s, nullptr))
		{
			if (r < minr)
			{
				closestidx = kk;
				minr = r;
				colls = s;
			}
		}
	}

	if (closestidx < 0)
		return nullptr;

	// get intersection point
	COccluderSegment* occto = &arrOcc[closestidx];
	vRetPt.x = occto->vStart.x * (1.0f - colls) + occto->vEnd.x * colls;
	vRetPt.y = occto->vStart.y * (1.0f - colls) + occto->vEnd.y * colls;
	return occto;
}
