#ifndef __RE_PARTICLE_H__
#define __RE_PARTICLE_H__

#include "MathGeoLib/include/Math/float3.h"

struct RE_Particle
{
	// Base attributes
	float lifetime = 0.0f;
	float dt_offset = 0.f;

	math::vec position = math::vec::zero;
	math::vec velocity = math::vec::zero;

	// Physic properties
	float mass = 0.1f;
	float col_radius = 0.2f;

	// Lighting parameters
	math::vec lightColor = math::vec::one;
	float intensity = 1.0f;
	float specular = .2f;
};

#endif //!__RE_PARTICLE_H__