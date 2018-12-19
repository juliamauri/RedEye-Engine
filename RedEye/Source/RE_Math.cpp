#include "RE_Math.h"

#include "Application.h"
#include "OutputLog.h"

#ifdef _DEBUG
#pragma comment(lib, "MathGeoLib/lib/MathGeoLib_debug.lib")
#else
#pragma comment(lib, "MathGeoLib/lib/MathGeoLib_release.lib")
#endif

RE_Math::RE_Math()
{}

void RE_Math::Init()
{
	LOG("Initializing Math");
	App->ReportSoftware("MathGeoLib", nullptr, "https://github.com/juj/MathGeoLib");
	lcg.Seed(Clock::TickU32());
}

float RE_Math::RandomF()
{
	// range [ 0, 1 )
	return lcg.Float();
}

int RE_Math::RandomInt()
{
	// range [ 0, 2147483647 ]
	return lcg.Int();
}

float RE_Math::RandomF(float min, float max)
{
	// @param a Lower bound, inclusive.
	// @param b Upper bound, inclusive.
	return lcg.Float(min, max);
}

int RE_Math::RandomInt(int min, int max)
{
	// @param a Lower bound, inclusive.
	// @param b Upper bound, exclusive.
	return lcg.Int(min, max);
}

void RE_Math::SetRNGSeed(unsigned int seed)
{
	lcg.Seed(seed);
}

math::float4x4 RE_Math::Rotate(math::float3 axis, float radians)
{
	axis.Normalize();
	return math::float4x4(math::Quat::identity * math::Quat::RotateAxisAngle(axis, radians));
}

math::float4x4 RE_Math::Rotate(math::Quat quat)
{
	return math::float4x4(math::Quat::identity * quat);
}
