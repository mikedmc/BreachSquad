#pragma once

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
	Vec2 pos;
	bool start;
	COccluder* parentWall;
	double angle;

	COccluderNode()
	{
		pos.x = pos.y = 0.0f;
		start = false;
		parentWall = nullptr;
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
		Vec2 start;
		Vec2 end;
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
	Vec2 m_vViewerPos; //pozitia "centrului"

	void AddPolySegment(double angle1, double angle2, COccluder *wall);
public:
	CPolyFOV() : occludersCnt(0), retTriangleBasesCnt(0), m_vViewerPos(Vec2(0.0f, 0.0f))
	{ };

	void SetOccluders(Vec2 &vViewerPos, COccluder *p_arrOccluders, int nOccludersCount); //seteaza harta de occluderi si pozitia centrului
	//calculeaza FOV-ul si intoarce pointeri la array-urile de rezultate in retBasesArr si retBasesCnt
	//in retBasesArr fiecare segment impreuna cu viewerPos formeaza un triunghi.
	CSegment* ComputeFOV(int &retBasesCnt);
};