#pragma once

// Is any of the flags set?
#define		IS_FLAG_ANY(x, flag)		( ((x) & (flag)) != 0 )
// Are ALL the flags set?
#define		IS_FLAG_ALL(x, flag)		( ((x) & (flag)) == (flag) )
// (NOT IS) make sure no flag is set
#define		NIS_FLAG_ANY(x, flag)		( ((x) & (flag)) == 0 )

// atan2 approximation const values
#define ATAN2_PI_FLOAT		3.14159265f
#define ATAN2_PIBY2_FLOAT	1.5707963f
#define ONEQTR_PI			(M_PI / 4.0f);
#define THRQTR_PI			(3.0f * M_PI / 4.0f);

namespace UTMath
{
	// distance to segment. Returns capsule shaped distance field.
	float			LineDist(D3DXVECTOR3 p, D3DXVECTOR3 v1, D3DXVECTOR3 v2, D3DXVECTOR3 *n);

	// finds intersection between p1-p2 and p3-p4
	bool			LineLineIntersection(D3DXVECTOR2 p1, D3DXVECTOR2 p2, D3DXVECTOR2 p3, D3DXVECTOR2 p4, D3DXVECTOR2 *outPt);

	// tells if lines intersect and returns r and s denominators (percentages between segments defining the lines)
	// a must be different from b. c and d can be the same.
	bool			LineLineIntersects_denom(Vec2 a, Vec2 b, Vec2 c, Vec2 d, float & r, float & s);

	// finds intersection between ray starting at a going through b and segment c-d
	// a and b must not be the same
	bool			RaySegmentIntersection(Vec2 a, Vec2 b, Vec2 c, Vec2 d, Vec2 *outPt = nullptr);
	// finds intersection between ray starting at a going through b and segment c-d
	// returns denominators retR for the ray and retS for the segment
	bool			RaySegmentIntersection_denom(Vec2 a, Vec2 b, Vec2 c, Vec2 d, float &retR, float &retS, Vec2 *outPt = nullptr);

	/*!
	*	\brief transforms "current" into "target" using quadratic easing
	*	\param: fMinSpeed should include delta time
	*  	\param: fDistMultiplier gets multiplied with actual distance between points (should include delta time)
	*/
	void			EaseTo_quadratic(float * current, float target, float fDistMultiplier, float fMinSpeed);
	/*!
	*	\brief transforms "current" into "target" using linear easing
	*	fSpeed should include delta time
	*/
	void			EaseTo_linear(float * current, float target, float fSpeed);
	/*!
	 *	\brief Intoarce alpha intre 0 si 1 cand fCursor este la capetele domeniului. Alpha este 0 la inceputul domeniului si la finalul acestuia si variaza liniar.
	 * De exemplu pe un domeniu de lungime 10, cursor la 0.5 pe regionsize de 1.0f va intoarce 0.5 alpha
	 */
	float			GetAlphaOnDomainEnds(float fCursor, float fDomainLength, float fAlphaRegionSize);

	/*!
	 * Tells if specified value is power of two
	 */
	bool			IsPowerOfTwo(unsigned int nVal);

	/* Get angle of vector. Returns -PI..PI */
	float			GetVectorAngle(D3DXVECTOR2 const &dir);

	/* Get angle of vector. Returns -PI..PI */
	float			GetVectorAngle(D3DXVECTOR2 start, D3DXVECTOR2 end);

	/* RETURNS: angle in rad between 2 vectors */
	float			GetAngleBetweenVectors(D3DXVECTOR2 vec1, D3DXVECTOR2 vec2);

	int				Log2i(float val);
	int				Log2i(int val);
	int				GetBitsNeededForValue(int val);

	// Returns number of bits set in dwValue
	int				CountBits(UINT32 dwValue);

	// faster atan2 approx
	float atan2_approximation1(float y, float x);
	// faster atan2 approx
	float atan2_approximation2(float y, float x);
}
