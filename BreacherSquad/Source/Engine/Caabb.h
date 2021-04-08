#pragma once

///------------------------------------------------------------------------------------------------------------------------------------------
///  2D AABB
///------------------------------------------------------------------------------------------------------------------------------------------
class CAABB
{
public:
	Vec2 vHalfSize;			//don't set manually!
	Vec2 vCenter;			//don't set manually!
	Vec2 vMin, vMax;		//don't set manually!
	Vec2 vSize;				//don't set manually!
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

	void				Set(float xmin, float ymin, float xmax, float ymax);
	void				Set(Vec2 min, Vec2 max);
	void				Set(RECTXYWH_F rect);
	void				Set(RECTLTRB_F rect);
	void				Set(CAABB* sourceAABB, Vec2 vOffset);
	// Correctly sets min/max by sorting the input points
	void				Set_Corrected(Vec2 pt1, Vec2 pt2);		

	bool				PointIn(Vec2 pt1);
	bool				PointIn(float x, float y);
	bool				Intersects(CAABB *dest);
	bool				IntersectsCircle(Vec2 center, float radius);

	//Moves AABB with delta vector
	void				Move(Vec2 delta);
	//Inflates AABB with delta on each axis
	void				Inflate(Vec2 delta);
	void				Inflate(float dX, float dY);
	float				GetArea();
	//scales CAABB by percent
	void				Scale(float fScalePercent);
	//flips AABB around origin
	void				Flip(bool flipX, bool flipY);
	//returns AABB as RECTXYWH
	RECTXYWH			to_RECTXYWH();
	//returns AABB as RECTXYWH_F
	RECTXYWH_F			to_RECTXYWH_F();
};

namespace AABB {
	CAABB				Lerp(CAABB &a, CAABB &b, float fFactor);
	bool				Intersection(CAABB &a, CAABB &b, CAABB &retVal);
	CAABB				Union(CAABB &a, CAABB &b);
	// Finds AABB that boxes all points
	CAABB				FromPoints(Vec2 * vecArr, int vecCnt);
	// Finda aabb that boxes all points, ignoring Z!
	CAABB				FromPoints(Vec3 * vecArr, int vecCnt);
	/*
	 * Keeps source box inside destination box without scaling and only if possible
	 * /returns FALSE if can't fit source in destination
	*/
	bool				KeepInside(CAABB & boxSource, CAABB & boxDest);
	/*!
	 * \brief Calculeaza intersectia unui segment start-end cu toate aabb-urile din lista si intoarce pe cel cu care s-a intersectat (cel mai apropiat).
	 * Atentie! Daca segmentul pleaca din bbox punctul de coliziune va fi in spatele primului punct (ca sa nu poti trage prin pereti)
	 */
	CAABB*				Segment_Intersection_Arr(Vec2 & start, Vec2 & end, CAABB * arrBoxes[], int nBoxesCnt, Vec2 * retCollisionPoint = NULL, Vec2 * retNormal = NULL);
	/*!
	 * \brief AABB segment start-end intersection.
	 * \warning Daca segmentul pleaca din bbox punctul de coliziune va fi in spatele primului punct (ca sa nu poti trage prin pereti)
	 */
	bool				Segment_Intersection(Vec2 & start, Vec2 & end, CAABB & box, Vec2 * retCollisionPoint = NULL);
	/*!
	* \brief AABB segment start-end intersection.
	* fRetT is the intersection factor between 0 and 1 where 1 is segment len
	*/
	bool				Segment_IntersectionEx(Vec2 & start, Vec2 & end, CAABB & box, Vec2 * retCollisionPoint, float &fRetT);
	/*!
	* \brief AABB segment start-end intersection without checking the heads of the segment.
	*/
	bool				Segment_Intersection_NoHeads(Vec2 & start, Vec2 & end, CAABB & box, Vec2 * retCollisionPoint);

	// Diferenta minkowski a doua AABB-uri (folosita pt coliziune, foarte rapida)
	// \returns: AABB-ul rezultat, daca contine originea inseamna ca se intersecteaza iar distanta minima la laturi este vectorul de penetrare
	CAABB				GetMinkowskiDifference(CAABB &a, CAABB &b);
	/*!
	 *	\brief: Morphs one AABB into another AABB with linear speed
	 *	Speed should include delta time
	 */
	void				MorphInto_Linear(CAABB *source, CAABB *target, float fSpeed);
	/*!
	*	\brief: Morphs one AABB into another AABB quadratic style
	*	\param: fMinSpeed should include delta time
	*  	\param: fDistMultiplier gets multiplied with actual distance between points (should include delta time)
	*/
	void				MorphInto_Quadratic(CAABB *source, CAABB *target, float fDistMultiplier, float fMinSpeed);

	// Returns a random point inside the box
	Vec2				GetRandomPointInBox(CAABB &a);
	// Returns surface of aabb
	FORCEINLINE float	GetSurface(CAABB &a) { return a.vSize.x * a.vSize.y; }
}