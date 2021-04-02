#pragma once

#define			QUARTER_PI 0.7853981633f
#define			HALF_PI 1.57079632f
#define			DOUBLE_PI 6.283185307f
#define			PI 3.1415926536f
//float	minimum
#define			EPS 0.00001f

// Is any of the flags set?
#define			IS_FLAG_ANY(x, flag)		( ((x) & (flag)) != 0 )
// Are ALL the flags set?
#define			IS_FLAG_ALL(x, flag)		( ((x) & (flag)) == (flag) )
// (NOT IS) make sure no flag is set
#define			NIS_FLAG_ANY(x, flag)		( ((x) & (flag)) == 0 )
// clears by mask
#define			FLAGOP_CLEAR(x, flagsMask)  (x &= ~flagsMask)

// changes a var between 2 limits
#define			inc_limit(var, step, limit) {if(var < (limit)) {var += step; if((var) > (limit)) var = limit;}}
#define			dec_limit(var, step, limit) {if(var > (limit)) {var -= step; if((var) < (limit)) var = limit;}}
#define			SIGN(var) (((var) < 0) ? -1:1)
// sign or zero
#define			SIGNZ(var) ((var == 0)?0: (((var) < 0) ? -1 : 1))
// invert a float that'sbetween 0  and 1
#define			INV_UNIT(var) (1.0f - var)
// checks if floats are almost equal
#define			FLOATS_EQUAL(a, b, threshold) ((fabs((a) - (b)) < threshold)?true:false)
// finds fractional part of float
#define			FLOAT_FRAC(a) (a - floor(a))
// Acts like modulo but on float
#define			FLOAT_MOD(a, nModuloValue) ( (((int)floor(a)) % (int)(nModuloValue)) + (a - floor(a)) )
// rounds a float to closest int value (kind of like casting to int)
#define			ROUND_FLOAT(x) (floor((x) + 0.5f))
// clamps float to 1 decimal places
#define			FLOAT_1DP(x) (floor(x * 10.0f) / 10.0f)
#define			FLOAT_2DP(x) (floor(x * 100.0f) / 100.0f)
#define			FLOAT_3DP(x) (floor(x * 1000.0f) / 1000.0f)

// atan2 approximation const values
#define			ATAN2_PI_FLOAT		3.14159265f
#define			ATAN2_PIBY2_FLOAT	1.5707963f
#define			ONEQTR_PI			(M_PI / 4.0f);
#define			THRQTR_PI			(3.0f * M_PI / 4.0f);

namespace UTMath
{
	// distance to segment. Returns capsule shaped distance field.
	float			LineDist(Vec3 p, Vec3 v1, Vec3 v2, Vec3 *n);

	// finds intersection between p1-p2 and p3-p4
	bool			LineLineIntersection(Vec2 p1, Vec2 p2, Vec2 p3, Vec2 p4, Vec2 *outPt);

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

	/* Get angle of vector. Returns -PI..PI. ang:0.0 for vec(1.0, 0.0f) */
	float			GetVectorAngle(Vec2 const &dir);

	/* Get angle of vector. Returns -PI..PI. ang:0.0 for vec(1.0, 0.0f) */
	float			GetVectorAngle(Vec2 start, Vec2 end);

	/* RETURNS: angle in rad between 2 vectors */
	float			GetAngleBetweenVectors(Vec2 vec1, Vec2 vec2);

	int				Log2i(float val);
	int				Log2i(int val);
	int				GetBitsNeededForValue(int val);

	// Returns number of bits set in dwValue
	int				CountBits(UINT32 dwValue);

	// faster atan2 approx
	float			atan2_approximation1(float y, float x);
	// faster atan2 approx
	float			atan2_approximation2(float y, float x);
	// Cosine interpolation
	float			Interpolate_cos(float a, float b, float t);
	// Linear interpolation
	float			Interpolate_lin(float a, float b, float t);

}
