#include "RE_EmissionData.h"

#include "Application.h"
#include "RE_Math.h"
#include "RE_Particle.h"

math::vec RE_EmissionShape::GetPosition() const
{
	switch (shape)
	{
	case RE_EmissionShape::POINT: return geo.point;
	case RE_EmissionShape::CIRCLE: return geo.circle.GetPoint(RE_MATH->RandomF() * 6.283185307f);
	case RE_EmissionShape::BOX:
		return { geo.box[0].x + (RE_MATH->RandomF() * (geo.box[1].x - geo.box[0].x)),
			geo.box[0].y + (RE_MATH->RandomF() * (geo.box[1].y - geo.box[0].y)),
			geo.box[0].z + (RE_MATH->RandomF() * (geo.box[1].z - geo.box[0].z)) };
	case RE_EmissionShape::SPHERE: return geo.sphere.pos + ((RE_MATH->RandomF() * geo.sphere.r) * RE_MATH->RandomNDir());
	case RE_EmissionShape::HOLLOW_SPHERE:
		float rand_radius = geo.hollow_sphere.second + (RE_MATH->RandomF() * (geo.hollow_sphere.first.r - geo.hollow_sphere.second));
		return geo.hollow_sphere.first.pos + (rand_radius * RE_MATH->RandomNDir());
	}
}

math::vec RE_EmissionVector::GetSpeed() const
{
	switch (type) {
	case NONE: return math::vec::zero;
	case VALUE: return min;
	case RANGEX: return { min.x + (RE_MATH->RandomF() * (max.x - min.x)), min.y, min.z};
	case RANGEY: return { min.x, min.y + (RE_MATH->RandomF() * (max.y - min.y)), min.z };
	case RANGEZ: return { min.x, min.y, min.z + (RE_MATH->RandomF() * (max.z - min.z)) };
	case RANGEXY: return { min.x + (RE_MATH->RandomF() * (max.x - min.x)), min.y + (RE_MATH->RandomF() * (max.y - min.y)), min.z };
	case RANGEXZ: return { min.x + (RE_MATH->RandomF() * (max.x - min.x)), min.y, min.z + (RE_MATH->RandomF() * (max.z - min.z)) };
	case RANGEYZ: return { min.x, min.y + (RE_MATH->RandomF() * (max.y - min.y)), min.z + (RE_MATH->RandomF() * (max.z - min.z)) };
	case RANGEXYZ: return { min.x + (RE_MATH->RandomF() * (max.x - min.x)), min.y + (RE_MATH->RandomF() * (max.y - min.y)), min.z + (RE_MATH->RandomF() * (max.z - min.z)) };}
}

float RE_EmissionSingleValue::GetValue() const
{
	switch (type) {
	case NONE: return 0.f;
	case VALUE: return min;
	case RANGE: return min + (RE_MATH->RandomF() * (max - min)); }
}

void RE_EmissionBoundary::ParticleCollision(RE_Particle& p) const
{
	bool ret = false;
	// Check boundary collisions
	switch (type)
	{
	case Type::NONE: break;
	case Type::GROUND:
	{
		// Check if particle intersects or has passed plane
		float dist_to_plane = data.plane.SignedDistance(p.position);
		if (dist_to_plane < p.col_radius)
		{
			// Resolve intersection
			const math::vec norm_speed = p.velocity.Normalized();
			float dist_to_col = 0.f;
			if (math::Plane::IntersectLinePlane(
				data.plane.normal,
				data.plane.d + p.col_radius,
				p.position,
				norm_speed,
				dist_to_col))
			{
				p.position += norm_speed * dist_to_col;

				// Resolve impulse if particle not already moving away from plane
				float dot = p.velocity.Dot(data.plane.normal);
				const math::vec speeds[2] = {
					p.velocity,
					p.velocity - (p.col_restitution + restitution) * dot * data.plane.normal };

				p.velocity = speeds[dot < 0.f];
			}
		}

		break;
	}
	case Type::CEILING: break;
	case Type::BOX: break;
	case Type::SPHERE: break;
	}
}

math::vec RE_EmissionExternalForces::GetAcceleration() const
{
	switch (type) {
	case RE_EmissionExternalForces::NONE: return math::vec::zero; break;
	case RE_EmissionExternalForces::GRAVITY: return math::vec(0.f, gravity, 0.f);
	case RE_EmissionExternalForces::WIND: return wind;
	case RE_EmissionExternalForces::WIND_GRAVITY: return math::vec(wind.x, wind.y + gravity, wind.z); }
}
