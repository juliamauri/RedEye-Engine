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

class RE_Math
{
public:

	RE_Math();
	~RE_Math();

	void Init();

	static int Cap(const int val = 0, const int min = 0, const int max = 1);
	static float Cap(const float val = 0.f, const float min = 0.f, const float max = 1.f);

	static int Min(const int a, const int b);
	static float Max(const float a, const float b);

	// Geometry
	static math::float4x4 Rotate(const math::float3 axis, const float radians);
	static math::float4x4 Rotate(const math::Quat quat);

	// Random seed
	static void	SetRNGSeed(unsigned int seed);

	// Random number generation
	static float					RandomF();
	static int						RandomInt();
	static float					RandomF(float min, float max);
	static int						RandomInt(int min, int max);
	static unsigned long long		RandomUID();

private:

	static math::LCG lcg;
};

#endif // !__RE_MATH_H__