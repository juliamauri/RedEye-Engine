#ifndef __RE_MATH_H__
#define __RE_MATH_H__

#include "Globals.h"
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

class RE_Math
{
public:
	RE_Math();
	~RE_Math() {}

	// Random number generation
	float				RandomF();
	float				RandomFN();
	int					RandomInt();
	float				RandomF(float min, float max);
	int					RandomInt(int min, int max);
	unsigned long long	RandomUID();

	math::vec RandomVec();
	math::vec RandomNVec();

	// Random Seed LCG
	math::LCG& GetRNGSeed();
	void SetRNGSeed(unsigned int seed);

	// Number valuations
	static int CapI(const int val, const int min, const int max)
	{
		RE_ASSERT(min <= max);
		int res[3] = { val, min, max };
		return res[(val < min) + (2 * (val > max))];
	}
	static unsigned int CapUI(const unsigned int val, const unsigned int min, const unsigned int max)
	{
		RE_ASSERT(min <= max);
		unsigned int res[3] = { val, min, max };
		return res[(val < min) + (2 * (val > max))];
	}
	static float CapF(const float val, const float min, const float max)
	{
		RE_ASSERT(min <= max);
		float res[3] = { val, min, max };
		return res[(val < min) + (2 * (val > max))];
	}
	static int MinI(const int a, const int b)
	{
		int res[2] = { a, b };
		return res[b < a];
	}
	static unsigned int MinUI(const unsigned int a, const unsigned int b)
	{
		unsigned int res[2] = { a, b };
		return res[b < a];
	}
	static float MinF(const float a, const float b)
	{
		float res[2] = { a, b };
		return res[b < a];
	}
	static int MaxI(const int a, const int b)
	{
		int res[2] = { a, b };
		return res[b > a];
	}
	static unsigned int MaxUI(const unsigned int a, const unsigned int b)
	{
		unsigned int res[2] = { a, b };
		return res[b > a];
	}
	static float MaxF(const float a, const float b)
	{
		float res[2] = { a, b };
		return res[b > a];
	}

	// Significant Values
	static const float pi;
	static const float pi_x2;
	static const float pi_div2;
	static const float deg_to_rad;
	static const float rad_to_deg;

	// Geometry
	static math::float4x4 Rotate(const math::float3 axis, const float radians)
	{
		return math::float4x4(math::Quat::identity * math::Quat::RotateAxisAngle(axis.Normalized(), radians));
	}
	static math::float4x4 Rotate(const math::Quat quat)
	{
		return math::float4x4(math::Quat::identity * quat);
	}

private:

	math::LCG lcg;
};

#endif // !__RE_MATH_H__