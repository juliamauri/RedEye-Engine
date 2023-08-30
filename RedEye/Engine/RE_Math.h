#ifndef __RE_MATH_H__
#define __RE_MATH_H__

#include "RE_Assert.h"
#include <MGL/MathGeoLib.h>

namespace RE_Math
{
	// Significant Values
	constexpr float pi = 3.1415926f;
	constexpr float pi_x2 = 6.2831853f;
	constexpr float pi_div2 = 1.5707963f;
	constexpr float deg_to_rad = 0.01745329f;
	constexpr float rad_to_deg = 57.2957795f;

	// Number valuations
	template<typename T>
	inline T Cap(const T val, const T min, const T max)
	{
		RE_ASSERT(min <= max);
		T res[4] = { val, min, max, 0 };
		return res[(val < min) + (2 * (val > max))];
	}

	template<typename T>
	inline T Min(const T a, const T b)
	{
		T res[2] = { a, b };
		return res[b < a];
	}

	template<typename T>
	inline T Max(const T a, const T b)
	{
		T res[2] = { a, b };
		return res[b > a];
	}

	inline math::vec MinVecValues(const math::vec a, const math::vec b)
	{
		float res[6] = { a.x, b.x, a.y, b.y, a.z, b.z };
		return math::vec(res[b.x < a.x], res[2 + (b.y < a.y)], res[4 + (b.z < a.z)]);
	}

	inline math::vec MaxVecValues(const math::vec a, const math::vec b)
	{
		float res[6] = { a.x, b.x, a.y, b.y, a.z, b.z };
		return math::vec(res[b.x > a.x], res[2 + (b.y > a.y)], res[4 + (b.z > a.z)]);
	}

	// Geometry
	inline math::float4x4 Rotate(const math::float3 axis, const float radians)
	{
		return math::float4x4(math::Quat::identity * math::Quat::RotateAxisAngle(axis.Normalized(), radians));
	}
	inline math::float4x4 Rotate(const math::Quat quat)
	{
		return math::float4x4(math::Quat::identity * quat);
	}
};

#endif // !__RE_MATH_H__