#pragma once

///------------------------------------------------------------------------------------------------------------------------------------------
///  Aici vor fi toate functiile si structurile de date care tin de intersectii, dreptunghiuri gen AABB samd
///------------------------------------------------------------------------------------------------------------------------------------------

///------------------------------------------------------------------------------------------------------------------------------------------
///  Generic math equations
///------------------------------------------------------------------------------------------------------------------------------------------

//Gaseste distanta la un segment. Cand este in afara segmentului intoarce distanta la capete (un fel de glow)
float MATH_LineDist(D3DXVECTOR3 p, D3DXVECTOR3 v1, D3DXVECTOR3 v2, D3DXVECTOR3 *n);
//Gaseste intersectia intre 2 linii (p1-p2 si p3-p4) si intoarce punctul de intersectie
bool MATH_LineLineIntersection(D3DXVECTOR2 p1, D3DXVECTOR2 p2, D3DXVECTOR2 p3, D3DXVECTOR2 p4, D3DXVECTOR2 *outPt);

/*!
*	\brief transforms "current" into "target" using quadratic easing
*	\param: fMinSpeed should include delta time
*  	\param: fDistMultiplier gets multiplied with actual distance between points (should include delta time)
*/
void MATH_EaseTo_quadratic(float * current, float target, float fDistMultiplier, float fMinSpeed);
/*!
*	\brief transforms "current" into "target" using linear easing
*	fSpeed should include delta time
*/
void MATH_EaseTo_linear(float * current, float target, float fSpeed);
/*!
 *	\brief Intoarce alpha intre 0 si 1 cand fCursor este la capetele domeniului. Alpha este 0 la inceputul domeniului si la finalul acestuia si variaza liniar.
 * De exemplu pe un domeniu de lungime 10, cursor la 0.5 pe regionsize de 1.0f va intoarce 0.5 alpha
 */
float MATH_GetAlphaOnDomainEnds(float fCursor, float fDomainLength, float fAlphaRegionSize);

/*!
 * Tells if specified value is power of two
 */
bool MATH_IsPowerOfTwo(unsigned int nVal);

/* Get angle of vector. Returns -PI..PI */
float Math_GetVectorAngle(D3DXVECTOR2 const &dir);
/* Get angle of vector. Returns -PI..PI */
float Math_GetVectorAngle(D3DXVECTOR2 start, D3DXVECTOR2 end);

/* RETURNS: angle in rad between 2 vectors */
float Math_GetAngleBetweenVectors(D3DXVECTOR2 vec1, D3DXVECTOR2 vec2);

///------------------------------------------------------------------------------------------------------------------------------------------
///  2D AABB
///------------------------------------------------------------------------------------------------------------------------------------------
class CAABB
{
public:
	D3DXVECTOR2 vHalfSize;   //don't set manually!
	D3DXVECTOR2 vCenter;	 //don't set manually!
	D3DXVECTOR2 vMin, vMax;	 //don't set manually!
	D3DXVECTOR2 vSize;		 //don't set manually!
public:
	CAABB() :vHalfSize(0.0f, 0.0f), vCenter(0.0f, 0.0f), vMin(0.0f, 0.0f), vSize(0.0f, 0.0f), vMax(0.0f, 0.0f)
	{}
	
	CAABB(RECTXYWH_F &srcRectF)
	{
		Set(srcRectF);
	}

	CAABB(D3DXVECTOR2 min, D3DXVECTOR2 max);
	CAABB(const CAABB& src) :
		vMin(src.vMin), vMax(src.vMax), vSize(src.vSize), vCenter(src.vCenter), vHalfSize(src.vHalfSize)
	{}
	CAABB(float vminx, float vminy, float vmaxx, float vmaxy);

	void Set(float xmin, float ymin, float xmax, float ymax);
	void Set(D3DXVECTOR2 min, D3DXVECTOR2 max);
	void Set(RECTXYWH_F rect);
	void Set(RECTLTRB_F rect);
	void Set(CAABB* sourceAABB, D3DXVECTOR2 vOffset);
	//seteaza corect bbox indiferent de pozitia punctelor
	void Set_Corrected(D3DXVECTOR2 pt1, D3DXVECTOR2 pt2);

	bool PointIn(D3DXVECTOR2 pt1);
	bool PointIn(float x, float y);
	bool Intersects(CAABB *dest);
	bool IntersectsCircle(D3DXVECTOR2 center, float radius);

	//Moves AABB with delta vector
	void Move(D3DXVECTOR2 delta);
	//Inflates AABB with delta on each axis
	void Inflate(D3DXVECTOR2 delta);
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


inline int Log2i(float val)
{
	return ((*(int *)(&val) >> 23) & 0xFF) - 127;
}
inline int Log2i(int val) 
{ 
	return Log2i((float)val); 
}
inline int MATH_GetBitsNeededForValue(int val) 
{ 
	return Log2i(val) + 1; 
}

//returns number of bits set in dwValue
int MATH_CountBits(UINT32 dwValue);