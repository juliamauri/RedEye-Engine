#ifndef __RE_MATH_H__
#define __RE_MATH_H__

#include "MathGeoLib/include/MathGeoLib.h"

enum Dir
{
	FORWARD,
	BACKWARD,
	LEFT,
	RIGHT,
	UP,
	DOWN
};

namespace RE_Math
{
	static math::LCG lcg(math::Clock::TickU32());

	// Random seed
	void	SetRNGSeed(unsigned int seed);

	// Random number generation
	float				RandomF();
	int					RandomInt();
	float				RandomF(float min, float max);
	int					RandomInt(int min, int max);
	unsigned long long	RandomUID();

	// if-less number evaluation
	int Cap(const int val = 0, const int min = 0, const int max = 1);
	float Cap(const float val = 0.f, const float min = 0.f, const float max = 1.f);

	int Min(const int a, const int b);
	float Max(const float a, const float b);

	// Geometry
	math::float4x4 Rotate(const math::float3 axis, const float radians);
	math::float4x4 Rotate(const math::Quat quat);
};

#endif // !__RE_MATH_H__