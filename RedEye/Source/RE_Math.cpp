#include "RE_Math.h"

#include "Application.h"
#include "RE_LogManager.h"

#ifdef _DEBUG
#pragma comment(lib, "MathGeoLib/lib/MathGeoLib_debug.lib")
#else
#pragma comment(lib, "MathGeoLib/lib/MathGeoLib_release.lib")
#endif

math::LCG RE_Math::lcg;

RE_Math::RE_Math() {}
RE_Math::~RE_Math() {}

void RE_Math::Init()
{
	RE_LOG("Initializing Math");
	RE_SOFT_NS("MathGeoLib", "https://github.com/juj/MathGeoLib");
	lcg.Seed(math::Clock::TickU32());
}

int RE_Math::Cap(const int val, const int min, const int max)
{
	RE_ASSERT(min <= max);
	int res[3] = { val, min, max };
	return res[(2 * (val > max)) + (val < min)];
}

float RE_Math::Cap(const float val, const float min, const float max)
{
	return val > max ? max : (val < min ? min : val);
}

int RE_Math::Min(const int a, const int b)
{
	int res[2] = { a, b };
	return res[b < a];
}

float RE_Math::Max(const float a, const float b)
{
	float res[2] = { a, b };
	return res[b < a];
}

math::float4x4 RE_Math::Rotate(const math::float3 axis, const float radians)
{
	return math::float4x4(math::Quat::identity * math::Quat::RotateAxisAngle(axis.Normalized(), radians));
}

math::float4x4 RE_Math::Rotate(const math::Quat quat)
{
	return math::float4x4(math::Quat::identity * quat);
}

void RE_Math::SetRNGSeed(unsigned int seed) { lcg.Seed(seed); }

// range [ 0, 1 )
float RE_Math::RandomF() { return lcg.Float(); }

// range [ 0, 2147483647 ]
int RE_Math::RandomInt() { return lcg.Int(); }

// @param a Lower bound, inclusive.
// @param b Upper bound, inclusive.
float RE_Math::RandomF(float min, float max) { return lcg.Float(min, max); }

// @param a Lower bound, inclusive.
// @param b Upper bound, exclusive.
int RE_Math::RandomInt(int min, int max) { return lcg.Int(min, max); }

UID RE_Math::RandomUID()
{
	UID ret = static_cast<UID>(lcg.Int());
	ret = ret << 32;
	ret += static_cast<UID>(lcg.Int()) + 1ull;

	return ret;
}
