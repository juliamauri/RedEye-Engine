#ifndef __RE_MATH_H__
#define __RE_MATH_H__

#include "MathGeoLib/include/MathGeoLib.h"
#include "Globals.h"

class RE_Math
{
public:

	RE_Math();
	~RE_Math();

	void Init();

	static int Cap(const int val = 0, const int min = 0, const int max = 1);
	static float Cap(const float val = 0.f, const float min = 0.f, const float max = 1.f);

	// Geometry
	static math::float4x4 Rotate(const math::float3 axis, const float radians);
	static math::float4x4 Rotate(const math::Quat quat);

	// Random seed
	static void	SetRNGSeed(unsigned int seed);

	// Random number generation
	static float	RandomF();
	static int		RandomInt();
	static float	RandomF(float min, float max);
	static int		RandomInt(int min, int max);
	static UID		RandomUID();

private:

	static math::LCG lcg;
};

#endif // !__RE_MATH_H__