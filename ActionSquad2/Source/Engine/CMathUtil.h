#pragma once

//#TODO: convert to namespace UTMath

///------------------------------------------------------------------------------------------------------------------------------------------
///  Generic math utils
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

int Log2i(float val);
int Log2i(int val);
int MATH_GetBitsNeededForValue(int val);

//returns number of bits set in dwValue
int MATH_CountBits(UINT32 dwValue);