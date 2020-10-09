#include "dxstdafx.h"
#include "Randoms.h"

//========================================================================
//
//  "Mersenne Twister pseudorandom number generator" 
//
//========================================================================
#include <time.h>


CRandom::CRandom(void)
{
	randomCallsCount = 0;

	rseed = 1;
	// safe0 start
	rseed_sp = 0;
	mti=CMATH_N+1;
	// safe0 end
}	
	

int CRandom::GetRandomCallsCount()
{
	return randomCallsCount;
}

int CRandom::GetProbabilityFromDomain(float arrProbabilities[], int nProbabilitiesCnt)
{
	//face suma tuturor probabilitatilor
	float sum = 0.0f;
	for (int ii = 0; ii < nProbabilitiesCnt; ii++)
	{
		//daca am probabilitate negativa nu o iau in seama
		sum += max(arrProbabilities[ii], 0.0f);
	}
	if (sum <= 0.0f)
		return -1;
	//alege o valoare in aceasta suma
	float chosen = RandFloat(sum);
	//vede pe ce "segment" cade;
	for (int ii = 0; ii < nProbabilitiesCnt; ii++)
	{
		//daca am probabilitate 0 sau mai mica o ignor
		if (arrProbabilities[ii] <= 0.0f)
			continue;

		if (chosen <= arrProbabilities[ii])
		{
			return ii;
		}

		chosen -= arrProbabilities[ii];
	}
	//daca inca mai are probabilitate in el a mers ceva nasol deci da return -1 ca si cum nu ar fi dat pe nimic
	return -1;
}

unsigned int CRandom::RandInt( unsigned int n )
{
    unsigned long y;
    static unsigned long mag01[2]={0x0, CMATH_MATRIX_A};

	if (n == 0)
	{
		return(0);
	}
	randomCallsCount++;

    /* mag01[x] = x * MATRIX_A  for x=0,1 */

    if (mti >= CMATH_N) { /* generate N words at one time */
        int kk;

        if (mti == CMATH_N+1)   /* if sgenrand() has not been called, */
            SetRandomSeed(4357); /* a default initial seed is used   */

        for (kk=0;kk<CMATH_N-CMATH_M;kk++) {
            y = (mt[kk]&CMATH_UPPER_MASK)|(mt[kk+1]&CMATH_LOWER_MASK);
            mt[kk] = mt[kk+CMATH_M] ^ (y >> 1) ^ mag01[y & 0x1];
        }
        for (;kk<CMATH_N-1;kk++) {
            y = (mt[kk]&CMATH_UPPER_MASK)|(mt[kk+1]&CMATH_LOWER_MASK);
            mt[kk] = mt[kk+(CMATH_M-CMATH_N)] ^ (y >> 1) ^ mag01[y & 0x1];
        }
        y = (mt[CMATH_N-1]&CMATH_UPPER_MASK)|(mt[0]&CMATH_LOWER_MASK);
        mt[CMATH_N-1] = mt[CMATH_M-1] ^ (y >> 1) ^ mag01[y & 0x1];

        mti = 0;
    }
  
    y = mt[mti++];
    y ^= CMATH_TEMPERING_SHIFT_U(y);
    y ^= CMATH_TEMPERING_SHIFT_S(y) & CMATH_TEMPERING_MASK_B;
    y ^= CMATH_TEMPERING_SHIFT_T(y) & CMATH_TEMPERING_MASK_C;
    y ^= CMATH_TEMPERING_SHIFT_L(y);

	// ET - old engine added one to the result.
	// We almost NEVER wanted to use this function
	// like this.  So, removed the +1 to return a 
	// range from 0 to n (not including n).
	int ret = (y % n);
	//LOG_DBG_BUFF(L":%d:    randint(%d):%d", randomCallsCount, n, ret);
	return ret;
}

int CRandom::RandInt( int min, int max )
{
	randomCallsCount++;

	unsigned int n = max - min + 1;
	unsigned long y;
	static unsigned long mag01[2] = { 0x0, CMATH_MATRIX_A };

	if (n == 0)
	{
		return(0);
	}
	randomCallsCount++;

	/* mag01[x] = x * MATRIX_A  for x=0,1 */

	if (mti >= CMATH_N) { /* generate N words at one time */
		int kk;

		if (mti == CMATH_N + 1)   /* if sgenrand() has not been called, */
			SetRandomSeed(4357); /* a default initial seed is used   */

		for (kk = 0; kk < CMATH_N - CMATH_M; kk++) {
			y = (mt[kk] & CMATH_UPPER_MASK) | (mt[kk + 1] & CMATH_LOWER_MASK);
			mt[kk] = mt[kk + CMATH_M] ^ (y >> 1) ^ mag01[y & 0x1];
		}
		for (; kk < CMATH_N - 1; kk++) {
			y = (mt[kk] & CMATH_UPPER_MASK) | (mt[kk + 1] & CMATH_LOWER_MASK);
			mt[kk] = mt[kk + (CMATH_M - CMATH_N)] ^ (y >> 1) ^ mag01[y & 0x1];
		}
		y = (mt[CMATH_N - 1] & CMATH_UPPER_MASK) | (mt[0] & CMATH_LOWER_MASK);
		mt[CMATH_N - 1] = mt[CMATH_M - 1] ^ (y >> 1) ^ mag01[y & 0x1];

		mti = 0;
	}

	y = mt[mti++];
	y ^= CMATH_TEMPERING_SHIFT_U(y);
	y ^= CMATH_TEMPERING_SHIFT_S(y) & CMATH_TEMPERING_MASK_B;
	y ^= CMATH_TEMPERING_SHIFT_T(y) & CMATH_TEMPERING_MASK_C;
	y ^= CMATH_TEMPERING_SHIFT_L(y);

	// ET - old engine added one to the result.
	// We almost NEVER wanted to use this function
	// like this.  So, removed the +1 to return a 
	// range from min to max 
	int ret = (y % n) + min;
	//LOG_DBG_BUFF(L":%d:    randint(%d, %d):%d", randomCallsCount, min, max, ret);
	return ret;
}


float CRandom::RandomF()
{
	float ret = ((float)RandInt(0xffffffff) / (float)0xffffffff);
	//LOG_DBG_BUFF(L":   randomF():%.6f", ret);
	return ret;
}

float CRandom::RandFloat(float n)
{
	float ret = n * ((float)RandInt(0xffffffff) / (float)0xffffffff);
	//LOG_DBG_BUFF(L":   rndfloat(%.6f):%.6f", n, ret);
	return ret;
}

float CRandom::RandFloat(float min, float max)
{
	float ret = min + (max - min) * ((float)RandInt(0xffffffff) / (float)0xffffffff);
	//LOG_DBG_BUFF(L":   rndfloat(%.6f, %.6f):%.6f", min, max, ret);
	return ret;
}

int CRandom::RandSign()
{
	int ret = ((int)((RandInt(0xffffffff) % 2) * 2) - 1);
	// never return 0
	if (ret == 0)
		ret = 1;
	//LOG_DBG_BUFF(L":   RandSign():%d", ret);
	return ret;
}

float CRandom::RandFloatSgn(float n)
{
	float ret = 2.0f * n * ((float)RandInt(0xffffffff) / (float)0xffffffff) - n;
	if (ret == 0.0f)
		ret = n;

	//LOG_DBG_BUFF(L":   rndfloatsgn(%.6f):%.6f", n, ret);

	return ret;
}

D3DXVECTOR2 CRandom::RandD3DXVECTOR2sgn(float x, float y)
{
	//LOG_DBG_BUFF(L":   RandD3DXVECTOR2sgn calls 2 randfloatsgn:");
	return (D3DXVECTOR2(RandFloatSgn(x), RandFloatSgn(y)));
}

void CRandom::SetRandomSeed(unsigned int n)
{
	//LOG_DBG_BUFF(L": SetRandSeed(%d). Calls: %d", n, randomCallsCount);
	randomCallsCount = 0;
	/* setting initial seeds to mt[N] using         */
	/* the generator Line 25 of Table 1 in          */
	/* [KNUTH 1981, The Art of Computer Programming */
	/*    Vol. 2 (2nd Ed.), pp102]                  */
	mt[0]= n & 0xffffffff;
	for (mti=1; mti<CMATH_N; mti++)
		mt[mti] = (69069 * mt[mti-1]) & 0xffffffff;

	rseed = n;
}

unsigned int CRandom::GetRandomSeed(void)
{
	return(rseed);
}

void CRandom::Randomize(void)
{
	SetRandomSeed((unsigned int)time(NULL));
}

//--------------------------------------------------------------------------------
// End CRandom - Twister
//--------------------------------------------------------------------------------



//========================================================================
// PrimeSearch.cpp -  traverse a known set of items randomly only once
//========================================================================

int CPrimeSearch::prime_array[] =
{
	2, 3, 5, 7, 
	11, 13, 17, 19, 23, 29, 31, 37, 41, 43, 47, 
	53, 59, 61, 67, 71, 73, 79, 83, 89, 97, 
	101, 103, 107, 109, 113, 127, 131, 137, 139, 149, 
	151, 157, 163, 167, 173, 179, 181, 191, 193, 197, 199, 
	211, 223, 227, 229, 233, 239, 241,
	251, 257, 263, 269, 271, 277, 281, 283, 293, 
	307, 311, 313, 317, 331, 337, 347, 349, 
	353, 359, 367, 373, 379, 383, 389, 397, 
	401, 409, 419, 421, 431, 433, 439, 443, 449, 
	457, 461, 463, 467, 479, 487, 491, 499, 
	503, 509, 521, 523, 541, 547, 
	557, 563, 569, 571, 577, 587, 593, 599, 
	601, 607, 613, 617, 619, 631, 641, 643, 647, 
	653, 659, 661, 673, 677, 683, 691, 
	701, 709, 719, 727, 733, 739, 743, 
	751, 757, 761, 769, 773, 787, 797, 
	809, 811, 821, 823, 827, 829, 839, 
	853, 857, 859, 863, 877, 881, 883, 887, 
	907, 911, 919, 929, 937, 941, 947, 
	953, 967, 971, 977, 983, 991, 997, 

	// after 1000, for space efficiency reasons, 
	// we choose to include fewer prime numbers the bigger the number get.

	1009, 1051, 1103, 1151, 1201, 1259, 1301, 1361, 1409, 1451, 
	1511, 1553, 1601, 1657, 1709, 1753, 1801, 1861, 1901, 1951, 
	2003, 2053, 2111, 2113, 2153, 2203, 2251, 
	2309, 2351, 2411, 2459, 2503, 2551, 2557, 
	2609, 2657, 2707, 2753, 2767, 2801, 2851, 2903, 2953,
	3001, 3061, 3109, 3163, 3203, 3251, 3301, 3359, 3407, 3457, 
	3511, 3557, 3607, 3659, 3701, 3761, 3803, 3851, 3907, 3967, 
	4001, 4051, 4111, 4153, 4201, 4253, 4327, 4357, 4409, 4451, 
	4507, 4561, 4603, 4651, 4703, 4751, 4801, 4861, 4903, 4951, 

	// begin to skip even more primes

	5003, 5101, 5209, 5303, 5407, 5501, 5623, 5701, 5801, 5903, 
	6007, 6101, 6211, 6301, 6421, 6521, 6607, 6701, 6803, 6907, 
	7001, 7103, 7207, 7307, 7411, 7507, 7603, 7703, 7817, 7901, 
	8009, 8101, 8209, 8311, 8419, 8501, 8609, 8707, 8803, 8923, 
	9001, 9103, 9203, 9311, 9403, 9511, 9601, 9719, 9803, 9901, 

	// and even more
	10007, 10501, 11003, 11503, 12007, 12503, 13001, 13513, 14009, 14503, 
	15013, 15511, 16033, 16519, 17011, 17509, 18013, 18503, 19001, 19501, 
	20011, 20507, 21001, 21503, 22003, 22501, 23003, 23509, 24001, 24509

	// if you need more - go get them yourself!!!!
	// Create a bigger array of prime numbers by using this web site: 
	// http://www.rsok.com/~jrm/printprimes.html

};


CPrimeSearch::CPrimeSearch(int elements)
{
	assert(elements>0 && "Can't do a PrimeSearch if you have 0 elements to search through!");

	maxElements = elements;

	int a = (rand()%13)+1;
	int b = (rand()%7)+1;
	int c = (rand()%5)+1;

	skip = (a * maxElements * maxElements) + (b * maxElements) + c;
	skip &= ~0xc0000000;		// this keeps skip from becoming too large....

	Restart();

	currentPrime = prime_array;
	int s = sizeof(prime_array)/sizeof(prime_array[0]);

	// if this assert gets hit you didn't have enough prime numbers to deal with this number of 
	// elements. Go back to the web site.
	assert(prime_array[s-1]>maxElements);

	while (*currentPrime < maxElements)
	{
		currentPrime++;
	}

	int test = skip % *currentPrime;
	if (!test)
		skip++;
}

int CPrimeSearch::GetNext(bool restart)
{
	if (restart)
		Restart();

	if (Done())
		return -1;

	bool done = false;

	int nextMember = currentPosition;

	while (!done)
	{
		nextMember = nextMember + skip;
		nextMember %= *currentPrime;
		searches++;

		if (nextMember < maxElements)
		{
			currentPosition = nextMember;
			done = true;
		}
	}

	return currentPosition;
}

//--------------------------------------------------------------------------------------
// PERLIN
//TODO: De facut o clasa buna pentru perlin noise
//--------------------------------------------------------------------------------------
float CosineInterpolate(float a, float b, float t)
{
	float ft = t * 3.1415927f;
	float f = (1 - cos(ft)) * 0.5f;

	return  a * (1.0f - f) + b * f;
}

inline float Interpolate(float a, float b, float t)
{
	return a * (1.0f - t) + b * t;
}

float Noise1D(int x)
{
	int nx = ((x << 13) ^ x);
	return (1.0f - ((nx * (nx * nx * 15731 + 789221) + 1376312589) & 0x7fffffff) / 2147483648.0f);
	//max float este 2147483647.0f
}

float SmoothedNoise1D(int x)
{
	return Noise1D(x) / 2 + Noise1D(x - 1) / 4 + Noise1D(x + 1) / 4;
}

float InterpolatedNoise1D(float x)
{
	int integer_X = floor(x);
	float fractional_X = x - integer_X;

	float v1 = SmoothedNoise1D(integer_X);
	float v2 = SmoothedNoise1D(integer_X + 1.0f);

	return (v1 * (1.0f - fractional_X) + v2 * fractional_X);
}

float InterpolatedNoise1D_cos(float x)
{
	int integer_X = floor(x);
	float fractional_X = x - integer_X;

	float v1 = SmoothedNoise1D(integer_X);
	float v2 = SmoothedNoise1D(integer_X + 1.0f);

	return CosineInterpolate(v1, v2, fractional_X);
}


float PerlinNoise1D(float x, float frequency, float freqMultiplier, float amplitude, float amplitudeMultiplier, int octaves)
{
	float total = 0.0f;
	float inFreq = frequency;
	float inAmp = amplitude;

	for (int n = 0; n<octaves; n++)
	{
		total += InterpolatedNoise1D(x * inFreq) * inAmp;

		inFreq *= freqMultiplier;
		inAmp *= amplitudeMultiplier;
	}
	return total;
}

float PerlinNoise1D_cos(float x, float frequency, float freqMultiplier, float amplitude, float amplitudeMultiplier, int octaves)
{
	float total = 0.0f;
	float inFreq = frequency;
	float inAmp = amplitude;

	for (int n = 0; n<octaves; n++)
	{
		total += InterpolatedNoise1D_cos(x * inFreq) * inAmp;

		inFreq *= freqMultiplier;
		inAmp *= amplitudeMultiplier;
	}
	return total;
}


//----- perlin 2D ------

float Noise2D(int x, int y)
{
	int nx = x + y * 59;	 //era 57 aici
	return (1.0f - ((nx * (nx * nx * 15731 + 789221) + 1376312589) & 0x7fffffff) / 2147483648.0f);
}

float SmoothedNoise2D(float x, float y)
{
	float corners = (Noise2D(x - 1, y - 1) + Noise2D(x + 1, y - 1) + Noise2D(x - 1, y + 1) + Noise2D(x + 1, y + 1)) / 16;
	float sides = (Noise2D(x - 1, y) + Noise2D(x + 1, y) + Noise2D(x, y - 1) + Noise2D(x, y + 1)) / 8;
	float center = Noise2D(x, y) / 4;

	//return corners + sides + center;
	//pentru ca face media, cam dispar extremele intre 0.0->0.2f si intre 0.8f->1.0f
	return (((corners + sides + center) / 0.2) - 1.0f) / 3.0f;
}

float InterpolatedNoise2D(float x, float y)
{
	int integer_X = floor(x);
	float fractional_X = x - integer_X;

	int integer_Y = floor(y);
	float fractional_Y = y - integer_Y;

	float v1 = SmoothedNoise2D(integer_X, integer_Y);
	float v2 = SmoothedNoise2D(integer_X + 1, integer_Y);
	float v3 = SmoothedNoise2D(integer_X, integer_Y + 1);
	float v4 = SmoothedNoise2D(integer_X + 1, integer_Y + 1);

	float i1 = Interpolate(v1, v2, fractional_X);
	float i2 = Interpolate(v3, v4, fractional_X);

	return Interpolate(i1, i2, fractional_Y);
}

float PerlinNoise2D(float x, float y, float frequency, float freqMultiplier, float amplitude, float amplitudeMultiplier, int octaves)
{
	float total = 0.0f;
	float inFreq = frequency;
	float inAmp = amplitude;

	for (int i = 0; i<octaves; i++)
	{
		total += InterpolatedNoise2D(x * inFreq, y * inFreq) * inAmp;

		inFreq *= freqMultiplier;
		inAmp *= amplitudeMultiplier;
	}
	return total;
}

//----- perlin 3D -----

float Noise3D(int x, int y, int z)
{
	int nx = x + y * 59 + z * 67;
	return (1.0f - ((nx * (nx * nx * 15731 + 789221) + 1376312589) & 0x7fffffff) / 2147483648.0f);
}

float SmoothedNoise3D(float x, float y, float z)
{
	float corners = (Noise3D(x - 1, y - 1, z - 1) + Noise3D(x + 1, y - 1, z - 1) + Noise3D(x - 1, y + 1, z - 1) + Noise3D(x + 1, y + 1, z - 1) +
		Noise3D(x - 1, y - 1, z + 1) + Noise3D(x + 1, y - 1, z + 1) + Noise3D(x - 1, y + 1, z + 1) + Noise3D(x + 1, y + 1, z + 1)) / 32
		;
	float sides = (Noise3D(x - 1, y, z) + Noise3D(x + 1, y, z) + Noise3D(x, y - 1, z) + Noise3D(x, y + 1, z) + Noise3D(x, y, z - 1) + Noise3D(x, y, z + 1)) / 12;
	float center = Noise3D(x, y, z) / 4;
	return corners + sides + center;
}

float InterpolatedNoise3D(float x, float y, float z)
{
	int integer_X = floor(x);
	float fractional_X = x - integer_X;

	int integer_Y = floor(y);
	float fractional_Y = y - integer_Y;

	int integer_Z = floor(z);
	float fractional_Z = z - integer_Z;

	float v1 = SmoothedNoise3D(integer_X, integer_Y, integer_Z);
	float v2 = SmoothedNoise3D(integer_X + 1, integer_Y, integer_Z);
	float v3 = SmoothedNoise3D(integer_X, integer_Y + 1, integer_Z);
	float v4 = SmoothedNoise3D(integer_X + 1, integer_Y + 1, integer_Z);

	float i1 = Interpolate(v1, v2, fractional_X);
	float i2 = Interpolate(v3, v4, fractional_X);

	float v5 = SmoothedNoise3D(integer_X, integer_Y, integer_Z + 1);
	float v6 = SmoothedNoise3D(integer_X + 1, integer_Y, integer_Z + 1);
	float v7 = SmoothedNoise3D(integer_X, integer_Y + 1, integer_Z + 1);
	float v8 = SmoothedNoise3D(integer_X + 1, integer_Y + 1, integer_Z + 1);

	float i3 = Interpolate(v5, v6, fractional_X);
	float i4 = Interpolate(v7, v8, fractional_X);

	float j1 = Interpolate(i1, i2, fractional_Y);
	float j2 = Interpolate(i3, i4, fractional_Y);

	return Interpolate(j1, j2, fractional_Z);
}

float PerlinNoise3D(float x, float y, float z, float frequency, float freqMultiplier, float amplitude, float amplitudeMultiplier, int octaves)
{
	float zreal = z * frequency;
	int integer_Z = floor(zreal);
	float fractional_Z = zreal - integer_Z;

	float p1 = PerlinNoise2D(x, y + integer_Z * 53, frequency, freqMultiplier, amplitude, amplitudeMultiplier, octaves);
	float p2 = PerlinNoise2D(x, y + (integer_Z + 1) * 53, frequency, freqMultiplier, amplitude, amplitudeMultiplier, octaves);

	return Interpolate(p1, p2, fractional_Z * frequency);

	/*
	//asa era inainte dar filtrul din smoothednoise3d nu arata la fel ca cel 2D
	float total = 0.0f;
	float inFreq = frequency;
	float inAmp = amplitude;

	for(int i=0; i<octaves; i++)
	{
	total += InterpolatedNoise3D(x * inFreq, y * inFreq, z * inFreq) * inAmp;

	inFreq *= freqMultiplier;
	inAmp *= amplitudeMultiplier;
	}
	return total;
	*/
}

float PerlinNoise3D(float x, float y, float z, float frequency, float amplitude)
{
	float zreal = z * frequency;
	int integer_Z = floor(zreal);
	float fractional_Z = zreal - integer_Z;

	float p1 = InterpolatedNoise2D(x * frequency, (y + (integer_Z * 53)) * frequency) * amplitude;
	float p2 = InterpolatedNoise2D(x * frequency, (y + ((integer_Z + 1) * 53)) * frequency) * amplitude;

	return Interpolate(p1, p2, fractional_Z);

}


int Random_GetProbabilityFromDomain(float arrProbabilities[], int nProbabilitiesCnt)
{
	//face suma tuturor probabilitatilor
	float sum = 0.0f;
	for (int ii = 0; ii < nProbabilitiesCnt; ii++)
	{
		//daca am probabilitate negativa nu o iau in seama
		sum += max(arrProbabilities[ii], 0.0f);
	}
	if (sum <= 0.0f)
		return -1;
	//alege o valoare in aceasta suma
	float chosen = randfloat(sum);
	//vede pe ce "segment" cade;
	for (int ii = 0; ii < nProbabilitiesCnt; ii++)
	{
		//daca am probabilitate 0 sau mai mica o ignor
		if (arrProbabilities[ii] <= 0.0f)
			continue;

		if (chosen <= arrProbabilities[ii])
		{
			return ii;
		}

		chosen -= arrProbabilities[ii];
	}
	//daca inca mai are probabilitate in el a mers ceva nasol deci da return -1 ca si cum nu ar fi dat pe nimic
	return -1;
}
