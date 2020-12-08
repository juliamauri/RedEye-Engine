#ifndef __RE_MATH_H__
#define __RE_MATH_H__

#include "MathGeoLib/include/MathGeoLib.h"

/*#define DEGTORAD 0.0174532925199432957f
#define RADTODEG 57.295779513082320876f
#define PI 3.14159265358979323846f
#define TWO_PI 6.28318530717958647692f
#define HALF_PI 1.57079632679489661923f
#define QUARTER_PI 0.78539816339744830961f
#define INV_PI 0.31830988618379067154f
#define INV_TWO_PI 0.15915494309189533576f
#define HAVE_M_PI*/

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
	float					RandomF();
	int						RandomInt();
	float					RandomF(float min, float max);
	int						RandomInt(int min, int max);
	unsigned long long		RandomUID();

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