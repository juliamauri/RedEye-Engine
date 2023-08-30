#include "RE_Random.h"

// Random Seed LCG

math::LCG lcg;

void RE_Random::SetRandomRNGSeed()
{
	lcg.Seed(math::Clock::TickU32());
}

math::LCG& RE_Random::GetRNGSeed() { return lcg; }

void RE_Random::SetRNGSeed(unsigned int seed) { lcg.Seed(seed); }

// Random number generation

// range [ 0, 1 )
float RE_Random::RandomF() { return lcg.Float(); }

// range ( -1, 1 )
float RE_Random::RandomFN() { return (lcg.Float() * 2.f) - 1.f; }

// range [ 0, 2147483647 ]
int RE_Random::RandomInt() { return lcg.Int(); }

// @param a Lower bound, inclusive.
// @param b Upper bound, inclusive.
float RE_Random::RandomF(float min, float max) { return lcg.Float(min, max); }

// @param a Lower bound, inclusive.
// @param b Upper bound, exclusive.
int RE_Random::RandomInt(int min, int max) { return lcg.Int(min, max); }

ulonglong RE_Random::RandomUID()
{
	ulonglong ret = static_cast<ulonglong>(lcg.Int());
	ret = ret << 32;
	ret += static_cast<ulonglong>(lcg.Int()) + 1ULL;

	return ret;
}

math::vec RE_Random::RandomVec()
{
	return math::vec(lcg.Float(), lcg.Float(), lcg.Float());
}

math::vec RE_Random::RandomNVec()
{
	return math::vec(lcg.Float() - 0.5f, lcg.Float() - 0.5f, lcg.Float() - 0.5f).Normalized();
}