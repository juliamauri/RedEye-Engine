#ifndef __RE_PARTICLE_H__
#define __RE_PARTICLE_H__

#include <MGL/Math/float3.h>

struct RE_Particle
{
	// Lifetime
	float lifetime = 0.f;
	float max_lifetime = 0.f;

	// Base attributes
	math::vec position = math::vec::zero;
	math::vec velocity = math::vec::zero;

	// Collider properties
	float mass = 1.f;
	float col_radius = 1.f;
	float col_restitution = 0.9f;

	// Lighting parameters
	math::vec lightColor = math::vec::one;
	float intensity = 1.0f;
	float specular = .2f;

	RE_Particle(
		float _max_lifetime = 0.f,
		math::vec pos = math::vec::zero,
		math::vec vel = math::vec::zero,
		float _mass = 1.f,
		float _col_radius = 1.f,
		float _col_restitution = 0.9f,
		math::vec _light_color = math::vec::one,
		float _light_intensity = 1.f,
		float _light_specular = .2f) :

		lifetime(0.f), max_lifetime(_max_lifetime),
		position(pos), velocity(vel),
		mass(_mass), col_radius(_col_radius), col_restitution(_col_restitution),
		lightColor(_light_color), intensity(_light_intensity), specular(_light_specular)
	{}
};

#endif //!__RE_PARTICLE_H__