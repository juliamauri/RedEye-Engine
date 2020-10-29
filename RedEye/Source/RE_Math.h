#ifndef __RE_MATH_H__
#define __RE_MATH_H__

#include "MathGeoLib/include/MathBuildConfig.h"
#include "MathGeoLib/include/MathGeoLib.h"

class RE_Math
{
public:

	RE_Math();

	void Init();

	// Random number generation
	static float	RandomF();
	static int		RandomInt();
	static float	RandomF(float min, float max);
	static int		RandomInt(int min, int max);

	// Random seed
	static void	SetRNGSeed(unsigned int seed);

	// Geometry
	static math::float4x4 Rotate(math::float3 axis, float radians);
	static math::float4x4 Rotate(math::Quat quat);

private:

	static math::LCG lcg;
};

#endif // !__RE_MATH_H__