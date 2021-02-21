#pragma once

/* Period parameters */  
#define CMATH_N 624
#define CMATH_M 397
#define CMATH_MATRIX_A 0x9908b0df   /* constant vector a */
#define CMATH_UPPER_MASK 0x80000000 /* most significant w-r bits */
#define CMATH_LOWER_MASK 0x7fffffff /* least significant r bits */

/* Tempering parameters */   
#define CMATH_TEMPERING_MASK_B 0x9d2c5680
#define CMATH_TEMPERING_MASK_C 0xefc60000
#define CMATH_TEMPERING_SHIFT_U(y)  (y >> 11)
#define CMATH_TEMPERING_SHIFT_S(y)  (y << 7)
#define CMATH_TEMPERING_SHIFT_T(y)  (y << 15)
#define CMATH_TEMPERING_SHIFT_L(y)  (y >> 18)

class CRandom
{
private:
	int					randomCallsCount; //random calls since SetRandomSeed

	// DATA
	unsigned int		rseed;
	unsigned int		rseed_sp;
	unsigned long		mt[CMATH_N]; /* the array for the state vector  */
	int					mti; /* mti==N+1 means mt[N] is not initialized */

	// FUNCTIONS
public:
	CRandom(void);	

	
	int				GetRandomCallsCount();			// Returns the number of random calls since last SetRandomSeed 
	unsigned int	RandInt( unsigned int n );
	int				RandInt( int min, int max );	// min <= rnd <= max
	
	float			RandomF();						// Returns a random float between 0.0f-1.0f
	float			RandFloat( float n );
	float			RandFloat( float min, float max );

	int				RandSign();
	float			RandFloatSgn( float n );		//-n <= rnd <= n
	Vec2			RandVec2Sgn(float x, float y);

	void			SetRandSeed(unsigned int n);
	unsigned int	GetRandSeed(void);
	void			SetRandSeedTime(void);

	//Returns the index of the array item with the selected probability
	int				GetProbabilityFromDomain(float arrProbabilities[], int nProbabilitiesCnt);
	
	// Shuffles an array
	template <class anyType>
	void ShuffleArray(anyType *arrayArg, int arrayElementsCnt, int shuffles);
};

//========================================================================
// PrimeSearch.h -  traverse a known set of items randomly only once
/******************************************************************
This class enables you to visit each and every member of an array
exactly once in an apparently random order.

void FadeToBlack(Screen *screen)
{
	int w = screen.GetWidth();
	int h = screen.GetHeight();
	int pixels = w * h;

	PrimeSearch search(pixels);
	int p;
	while((p=search.GetNext())!=-1)
	{
		int x = p % w;
		int y = h / p;

		screen.SetPixel(x, y, BLACK);

		// of course, you wouldn't blit every pixel change.
		screen.Blit();
	}
}
NOTE: If you want the search to start over at the beginning again - 
you must call the Restart() method, OR call GetNext(true).
********************************************************************/

class CPrimeSearch
{
	static int prime_array[];

	int skip;
	int currentPosition;
	int maxElements;
	int *currentPrime;
	int searches;
	
public:
	CPrimeSearch(int elements);
	int GetNext(bool restart=false);
	bool Done() { return (searches==*currentPrime); }
	void Restart() { currentPosition=0; searches=0; }
};


//--------------------------------------------------------------------------------------
// PERLIN
//--------------------------------------------------------------------------------------

namespace UTPerlin
{
	float Noise1D(int x);
	float SmoothedNoise1D(int x);
	// fast noise fn without octaves
	float InterpolatedNoise1D_cos(float x);
	float InterpolatedNoise1D(float x);
	//functie de cos cu octave pt variatie (variatia se aduna, deci va depasi amplitude
	float PerlinNoise1D(float x, float frequency, float freqMultiplier, float amplitude, float amplitudeMultiplier, int octaves);
	float PerlinNoise1D_cos(float x, float frequency, float freqMultiplier, float amplitude, float amplitudeMultiplier, int octaves);
	
	//2D noise
	float Noise2D(int x, int y);
	float SmoothedNoise2D(float x, float y);
	float InterpolatedNoise2D(float x, float y);
	float PerlinNoise2D(float x, float y, float frequency, float freqMultiplier, float amplitude, float amplitudeMultiplier, int octaves);

	//3D noise
	float Noise3D(int x, int y, int z);
	float SmoothedNoise3D(float x, float y, float z);
	float InterpolatedNoise3D(float x, float y, float z);
	float PerlinNoise3D(float x, float y, float z, float frequency, float freqMultiplier, float amplitude, float amplitudeMultiplier, int octaves);
	// no octaves version
	float PerlinNoise3D(float x, float y, float z, float frequency, float amplitude);
}

//--------------------------------------------------------------------------------------
// UTILITY
//--------------------------------------------------------------------------------------

//comment the next line in order to use the system random generator functions
//#define K_RANDOM_USE_CUSTOM

#if defined(K_RANDOM_USE_CUSTOM)


///**************************************************************************************
/// Sigleton de acces functie random
///**************************************************************************************
CRandom& UTGetRandom()
{
	static CRandom g_Random;
	return g_Random;
}

#define  randseed(a)			UTGetRandom().SetRandomSeed(a)
#define  randfloatsgn(a)		UTGetRandom().RandFloatSgn(a)
#define  randfloat(a)			UTGetRandom().RandFloat(a)
#define  randint(a)				UTGetRandom().RandInt(a)
#define  randint_range(a, b)	UTGetRandom().RandInt(a, b)
#define  randsign()				((int)((UTGetRandom().RandInt(0xffffffff) % 2) * 2) - 1)
#define  randVec2sgn(x,y) (Vec2(UTGetRandom().RandFloatSgn(x), UTGetRandom().RandFloatSgn(y)))

#else

#define  randseed(a)			srand(a)
#define  randfloatsgn(a)		(float((float)rand() / ((float)RAND_MAX / ((a) * 2.0f))) - (a))
#define  randfloat(a)			float((float)rand() / ((float)RAND_MAX / (a)))
#define  randint(a)				((unsigned int)((unsigned int)rand() % (unsigned int)(a)))
#define  randint_range(a, b)	(a + ((unsigned int)((unsigned int)rand() % (unsigned int)(b - a + 1))))
#define  randsign()				((int)(( ((unsigned int)rand() % (unsigned int)(2)) ) * 2 - 1))
#define  randVec2sgn(x,y)		(Vec2((float((float)rand() / ((float)RAND_MAX / ((x) * 2.0f))) - (x)), (float((float)rand() / ((float)RAND_MAX / ((y) * 2.0f))) - (y))))
#define  randompercent(fProbabilityPercent) ((randfloat(100.0f) <= fProbabilityPercent) ? true : false)

#endif

/*
// Receives array of probabilities and returns index based on those probabilities
int Random_GetProbabilityFromDomain(float arrProbabilities[], int nProbabilitiesCnt);

template <class anyType>
void Random_ShuffleArray(anyType *arrayArg, int arrayElementsCnt, int shuffles)
{
	anyType pivot;
	for (int kk = 0; kk < shuffles; kk++)
	{
		int pos1 = randint(arrayElementsCnt);
		int pos2 = randint(arrayElementsCnt);
		pivot = arrayArg[pos1];
		arrayArg[pos1] = arrayArg[pos2];
		arrayArg[pos2] = pivot;
	}
}
*/
