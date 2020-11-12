#pragma once

///------------------------------------------------------------------------------------------------------------------------------------------
///  2D AABB
///------------------------------------------------------------------------------------------------------------------------------------------
class CAABB
{
public:
	Vec2 vHalfSize;   //don't set manually!
	Vec2 vCenter;	 //don't set manually!
	Vec2 vMin, vMax;	 //don't set manually!
	Vec2 vSize;		 //don't set manually!
public:
	CAABB() :vHalfSize(0.0f, 0.0f), vCenter(0.0f, 0.0f), vMin(0.0f, 0.0f), vSize(0.0f, 0.0f), vMax(0.0f, 0.0f)
	{}

	CAABB(RECTXYWH_F &srcRectF)
	{
		Set(srcRectF);
	}

	CAABB(Vec2 min, Vec2 max);
	CAABB(const CAABB& src) :
		vMin(src.vMin), vMax(src.vMax), vSize(src.vSize), vCenter(src.vCenter), vHalfSize(src.vHalfSize)
	{}
	CAABB(float vminx, float vminy, float vmaxx, float vmaxy);

	void Set(float xmin, float ymin, float xmax, float ymax);
	void Set(Vec2 min, Vec2 max);
	void Set(RECTXYWH_F rect);
	void Set(RECTLTRB_F rect);
	void Set(CAABB* sourceAABB, Vec2 vOffset);
	//seteaza corect bbox indiferent de pozitia punctelor
	void Set_Corrected(Vec2 pt1, Vec2 pt2);

	bool PointIn(Vec2 pt1);
	bool PointIn(float x, float y);
	bool Intersects(CAABB *dest);
	bool IntersectsCircle(Vec2 center, float radius);

	//Moves AABB with delta vector
	void Move(Vec2 delta);
	//Inflates AABB with delta on each axis
	void Inflate(Vec2 delta);
	void Inflate(float dX, float dY);
	//scales CAABB by percent
	void Scale(float fScalePercent);
	//flips AABB around origin
	void Flip(bool flipX, bool flipY);
	//returns AABB as RECTXYWH
	RECTXYWH as_RECTXYWH();
};

CAABB AABB_Lerp(CAABB &a, CAABB &b, float fFactor);
bool AABB_Intersection(CAABB &a, CAABB &b, CAABB &retVal);
CAABB AABB_Union(CAABB &a, CAABB &b);
//Gaseste AABB care cuprinde toate punctele.
CAABB AABB_FromPoints(D3DXVECTOR2 * vecArr, int vecCnt);
//Gaseste AABB care cuprinde toate punctele. Ignora coord Z
CAABB AABB_FromPoints(D3DXVECTOR3 * vecArr, int vecCnt);
/* 
 * Keeps source box inside destination box without scaling and only if possible 
 * /returns FALSE if can't fit source in destination
*/
bool AABB_KeepInside(CAABB & boxSource, CAABB & boxDest);
/*!
 * \brief Calculeaza intersectia unui segment start-end cu toate aabb-urile din lista si intoarce pe cel cu care s-a intersectat (cel mai apropiat). 
 * Atentie! Daca segmentul pleaca din bbox punctul de coliziune va fi in spatele primului punct (ca sa nu poti trage prin pereti)
 */
CAABB* AABB_Segment_Intersection_Arr(D3DXVECTOR2 & start, D3DXVECTOR2 & end, CAABB * arrBoxes[], int nBoxesCnt, D3DXVECTOR2 * retCollisionPoint = NULL, D3DXVECTOR2 * retNormal = NULL);
/*!
 * \brief AABB segment start-end intersection. 
 * \warning Daca segmentul pleaca din bbox punctul de coliziune va fi in spatele primului punct (ca sa nu poti trage prin pereti)
 */
bool AABB_Segment_Intersection(D3DXVECTOR2 & start, D3DXVECTOR2 & end, CAABB & box, D3DXVECTOR2 * retCollisionPoint = NULL);
/*!
* \brief AABB segment start-end intersection. 
* fRetT is the intersection factor between 0 and 1 where 1 is segment len
*/
bool AABB_Segment_IntersectionEx(D3DXVECTOR2 & start, D3DXVECTOR2 & end, CAABB & box, D3DXVECTOR2 * retCollisionPoint, float &fRetT);
/*!
* \brief AABB segment start-end intersection without checking the heads of the segment.
*/
bool AABB_Segment_Intersection_NoHeads(D3DXVECTOR2 & start, D3DXVECTOR2 & end, CAABB & box, D3DXVECTOR2 * retCollisionPoint);

// Diferenta minkowski a doua AABB-uri (folosita pt coliziune, foarte rapida)
// \returns: AABB-ul rezultat, daca contine originea inseamna ca se intersecteaza iar distanta minima la laturi este vectorul de penetrare
CAABB AABB_GetMinkowskiDifference(CAABB &a, CAABB &b);
/*!
 *	\brief: Morphs one AABB into another AABB with linear speed
 *	Speed should include delta time
 */
void AABB_MorphInto_linear(CAABB *source, CAABB *target, float fSpeed);
/*!
*	\brief: Morphs one AABB into another AABB quadratic style
*	\param: fMinSpeed should include delta time
*  	\param: fDistMultiplier gets multiplied with actual distance between points (should include delta time)
*/
void AABB_MorphInto_quadratic(CAABB *source, CAABB *target, float fDistMultiplier, float fMinSpeed);

//returns a random point inside the box
D3DXVECTOR2 AABB_GetRandomPointInBox(CAABB &a);
FORCEINLINE float AABB_GetSurface(CAABB &a) { return a.vSize.x * a.vSize.y; }
