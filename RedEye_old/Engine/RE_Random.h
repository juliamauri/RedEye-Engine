#ifndef __RE_RANDOM_H__
#define __RE_RANDOM_H__

#include "RE_DataTypes.h"
#include <MGL/MathGeoLib.h>

namespace RE_Random
{
	// Random Seed LCG
	math::LCG& GetRNGSeed();
	void SetRandomRNGSeed();
	void SetRNGSeed(unsigned int seed);

	// Random number generation
	float				RandomF();
	float				RandomFN();
	int					RandomInt();
	float				RandomF(float min, float max);
	int					RandomInt(int min, int max);
	ulonglong			RandomUID();

	math::vec RandomVec();
	math::vec RandomNVec();
}

#define RANDOM_UID RE_Random::RandomUID()

#endif // !__RE_RANDOM_H__