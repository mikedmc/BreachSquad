#pragma once

class COccluderSegment
{
public:
	Vec2 vStart;
	Vec2 vEnd;
	Vec2 vN;
	float fStartAng;
	float fEndAng;

	void Set(Vec2 vfrom, Vec2 vto, Vec2 vNrm, Vec2 vViewerPos);
	// Computes angles from vOrigin
	void ComputeAngles(Vec2 vViewerPos);

	COccluderSegment() :
		fStartAng(0.0f), fEndAng(0.0f),
		vN(0.0f, 0.0f), vStart(0.0f, 0.0f), vEnd(0.0f, 0.0f)
	{}
};

// structure used for sorting the occluder intersections
struct sOccluderIntersection
{
	Vec2 vPos;
	Vec2 vN;
	float fAngle;

	sOccluderIntersection(Vec2 pos, Vec2 vnormal, float angle) :
		vPos(pos), vN(vnormal), fAngle(angle)
	{}
};


class COccluder
{
public:
	Vec2 start;
	Vec2 end;

	COccluder()
	{
		start.x = start.y = 0.0f;
		end.x = end.y = 0.0f;
	}
	//este punctul la stanga Occluderului?
	bool PointOnLeft(Vec2 &p);
};

class COccluderNode
{
public:
	D3DXVECTOR2 pos;
	bool start;
	COccluder* parentWall;
	double angle;

	COccluderNode()
	{
		pos.x = pos.y = 0.0f;
		start = false;
		parentWall = NULL;
		angle = 0.0f;
	}

	COccluderNode(const COccluderNode &newval)
	{
		angle = newval.angle;
		parentWall = newval.parentWall;
		pos = newval.pos;
		start = newval.start;
	}
};

#define K_PFOV_MAX_OCCLUDERS 1000
#define K_PFOV_MAX_FOV_TRIANGLES 1000

class CPolyFOV
{
public:
	class CSegment
	{
	public:
		D3DXVECTOR2 start;
		D3DXVECTOR2 end;
	};
private:
	//comparatorul de noduri pentru qsort
	static int OccluderNodesComparer(const void* a, const void* b);
	//internal occluders array
	COccluder	occluders[K_PFOV_MAX_OCCLUDERS];
	int			occludersCnt; //numar occluders
	//internal return array
	CSegment    retTriangleBases[K_PFOV_MAX_FOV_TRIANGLES]; //aici salveaza FOV-ul calculat doar ca baze ale triunghiurilor (varful este mereu in viewerPos)
	int			retTriangleBasesCnt; //numarul de baze de triunghi returnate
	//viewer
	D3DXVECTOR2 m_vViewerPos; //pozitia "centrului"

	void AddPolySegment(double angle1, double angle2, COccluder *wall);
public:
	CPolyFOV() : occludersCnt(0), retTriangleBasesCnt(0), m_vViewerPos(D3DXVECTOR2(0.0f, 0.0f))
	{ };

	void SetOccluders(D3DXVECTOR2 &vViewerPos, COccluder *p_arrOccluders, int nOccludersCount); //seteaza harta de occluderi si pozitia centrului
	//calculeaza FOV-ul si intoarce pointeri la array-urile de rezultate in retBasesArr si retBasesCnt
	//in retBasesArr fiecare segment impreuna cu viewerPos formeaza un triunghi.
	CSegment* ComputeFOV(int &retBasesCnt);
};