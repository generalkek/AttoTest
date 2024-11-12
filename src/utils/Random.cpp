
#include "Random.h"

#include <time.h>
#include <stdint.h>
#include <cstring>

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

namespace math {

	class GCCRandom
	{
	private:
		// DATA
		unsigned int		rseed;
		unsigned int		rseed_sp;
		unsigned long mt[CMATH_N]; /* the array for the state vector  */
		int mti; /* mti==N+1 means mt[N] is not initialized */

		// FUNCTIONS
	public:
		GCCRandom(void);

		unsigned int	Random(unsigned int n);
		float			Random();
		void			SetRandomSeed(unsigned int n);
		unsigned int	GetRandomSeed(void);
		void			Randomize(void);
	};

	GCCRandom g_random{};

	/*unsigned int PCGHash(unsigned int seed)
	{
		unsigned int state = seed * 747796405u + 2891336453u;
		unsigned int word = ((state >> ((state >> 28u) + 4u)) ^ state) * 277803737u;
		return (word >> 22u) ^ word;
	}*/

	unsigned int Random(unsigned int x)
	{
		return g_random.Random(x);
	}

	unsigned int Random(unsigned int min, unsigned int max)
	{
		const unsigned int res = g_random.Random(max);
		const unsigned int minRes = min + res;
		return minRes > max ? max : minRes;
	}

	float Random()
	{
		return g_random.Random();
	}

	float RandomF(float x)
	{
		return static_cast<float>(g_random.Random(static_cast<unsigned int>(x)));
	}

	float RandomF(float min, float max)
	{
		const float res = g_random.Random();
		return min + (max - min) * res;
	}

	GCCRandom::GCCRandom(void)
	{
		rseed = 1;
		// safe0 start
		rseed_sp = 0;
		mti = CMATH_N + 1;
		memset(mt, 0, sizeof(mt));
		// safe0 end
		Randomize();
	}

	unsigned int GCCRandom::Random(unsigned int n)
	{
		unsigned long y;
		static unsigned long mag01[2] = { 0x0, CMATH_MATRIX_A };

		if (n == 0)
			return(0);

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
		// range from 0 to n (not including n).
		return (y % n);
	}


	float GCCRandom::Random()
	{
		float r = (float)Random(INT32_MAX);
		float divisor = (float)INT32_MAX;
		return (r / divisor);
	}

	void GCCRandom::SetRandomSeed(unsigned int n)
	{
		/* setting initial seeds to mt[N] using         */
		/* the generator Line 25 of Table 1 in          */
		/* [KNUTH 1981, The Art of Computer Programming */
		/*    Vol. 2 (2nd Ed.), pp102]                  */
		mt[0] = n & 0xffffffff;
		for (mti = 1; mti < CMATH_N; mti++)
			mt[mti] = (69069 * mt[mti - 1]) & 0xffffffff;

		rseed = n;
	}

	unsigned int GCCRandom::GetRandomSeed(void)
	{
		return(rseed);
	}

	void GCCRandom::Randomize(void)
	{
		SetRandomSeed((unsigned int)time(NULL));
	}
}