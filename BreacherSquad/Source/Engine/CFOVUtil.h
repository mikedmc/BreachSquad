#pragma once

class COccluderSegment
{
public:
	Vec2		vStart;
	Vec2		vEnd;
	Vec2		vN;
	float		fStartAng;
	float		fEndAng;

	DWORD		dwWallID;	//0 means empty. contains id of collision box or Y position in tileset if it's a wall. Allows us to detect if we're hitting the same wall.
	float		fWallH;		//0 means no light on wall height

	void		Set(Vec2 vfrom, Vec2 vto, Vec2 vNrm, Vec2 vViewerPos, DWORD wallID = 0, float wallH = 0);
	// Moves the start point and recomputes the angle
	void		MoveStart(Vec2 vPos, Vec2 vViewerPos);
	// Moves the end point and recomputes the angle
	void		MoveEnd(Vec2 vPos, Vec2 vViewerPos);

	COccluderSegment() :
		fStartAng(0.0f), fEndAng(0.0f),
		vN(0.0f, 0.0f), vStart(0.0f, 0.0f), vEnd(0.0f, 0.0f),
		fWallH(0.0f), dwWallID(0)
	{}
};

// structure used for sorting the occluder intersections
struct sOccluderIntersection
{
	Vec2 vPos;
	DWORD dwWallID;
	float fWallH;
	float fAngle;

	sOccluderIntersection(Vec2 pos, float angle, DWORD wallID = 0, float wallH = 0.0f) :
		vPos(pos), fWallH(wallH), dwWallID(wallID), fAngle(angle)
	{}
};


namespace FOVUtil
{
	// Builds the FOV as a list of triangles, intersecting with all given shadowing occluders
	int						BuildOccludedVolume(Vec2 vEye, DWORD dwColor, COccluderSegment* arrOcc, int nOccludersCnt, _VERTEX_PNCT4T4 *outVerts, int outVertsMaxCnt);
	// Gets closest occluder that intersects ray, from occluders array. Occluders MUST have ongles set according to checked vEye!
	// Used by BuildOccludedVolume
	COccluderSegment*		RayOccludersIntersection(Vec2 vEye, Vec2 vTo, float fRayAngle, COccluderSegment* arrOcc, int nOccludersCnt, Vec2 & vRetPt);
}
