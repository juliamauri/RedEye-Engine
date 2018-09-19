#ifndef __RE_MATH_H__
#define __RE_MATH_H__

#include "MathGeoLib/include/MathBuildConfig.h"
#include "MathGeoLib/include/MathGeoLib.h"

class RE_Math
{
public:

	RE_Math();
	RE_Math(unsigned int seed);

	float	RandomF();
	int		RandomInt();

	float	RandomF(float min, float max);
	int		RandomInt(int min, int max);

	void	SetRNGSeed(unsigned int seed);

	math::float4x4 Rotate(math::float3 axis, float radians);

private:

	math::LCG lcg;
};

#endif // !__RE_MATH_H__