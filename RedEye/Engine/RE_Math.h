#ifndef __RE_MATH_H__
#define __RE_MATH_H__

#include "RE_Assert.h"
#include <MGL/MathGeoLib.h>

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
	template<typename T>
	static inline T Cap(const T val, const T min, const T max)
	{
		RE_ASSERT(min <= max);
		T res[4] = { val, min, max, 0 };
		return res[(val < min) + (2 * (val > max))];
	}

	template<typename T>
	static inline T Min(const T a, const T b)
	{
		T res[2] = { a, b };
		return res[b < a];
	}

	template<typename T>
	static inline T Max(const T a, const T b)
	{
		T res[2] = { a, b };
		return res[b > a];
	}

	static inline math::vec MinVecValues(const math::vec a, const math::vec b)
	{
		float res[6] = { a.x, b.x, a.y, b.y, a.z, b.z };
		return math::vec(res[b.x < a.x], res[2 + (b.y < a.y)], res[4 + (b.z < a.z)]);
	}

	static inline math::vec MaxVecValues(const math::vec a, const math::vec b)
	{
		float res[6] = { a.x, b.x, a.y, b.y, a.z, b.z };
		return math::vec(res[b.x > a.x], res[2 + (b.y > a.y)], res[4 + (b.z > a.z)]);
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

#define RANDOM_UID RE_MATH->RandomUID()

#endif // !__RE_MATH_H__