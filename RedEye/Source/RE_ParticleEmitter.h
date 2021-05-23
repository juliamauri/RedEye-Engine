#ifndef __RE_PARTICLEEMITTER_H__
#define __RE_PARTICLEEMITTER_H__

#include "MathGeoLib/include/Math/float3.h"
#include "MathGeoLib/include/Geometry/Plane.h"
#include <EASTL/vector.h>

#include "ImGui/imgui.h"

class RE_CompPrimitive;

struct RE_ParticleEmitter
{
	unsigned int id = 0u;

	// Playback parameters
	enum PlaybackState { STOP = 0, PLAY, PAUSE } state = STOP;
	bool loop = true; // float start_delay = 0.0f;

	// Instantiation parameters
	float lifetime = 14.f;
	float speed_muliplier = 1.f;
	float spawn_frequency = 51.f;
	float spawn_offset = 0.f;

	inline int GetNewSpawns(float dt)
	{
		int units = static_cast<int>((spawn_offset += dt) * spawn_frequency);
		spawn_offset -= static_cast<float>(units) / spawn_frequency;
		return units;
	}

	// Control values
	float maxLifeTime = 15.5f;
	float maxSpeed = 20.f;
	float maxDistance = 2.f * 1.5f; //lifetime * speed

	// Physic properties ---------------------------------------------------------

	// Collider
	float restitution = 0.2f; // elastic vs inelastic

	// External Forces
	float gravity = -9.81f;
	math::vec wind = math::vec::zero;

	// Boundaries
	enum BoundaryType
	{
		NONE,
		GROUND,
		CEILING,
		BOX,
		SPHERE
	} bound_type = GROUND;

	enum BoundaryEffect
	{
		CONTAIN,
		KILL,
		CLAMP
	} effect;

	union Boundary
	{
		Boundary() {}
		float radius = 0.f; // height
		math::Plane plane;
	} boundary;

	// Render properties ---------------------------------------------------------

	enum ColorState { SINGLE = 0, OVERLIFETIME, OVERDISTANCE, OVERSPEED };
	ColorState cState = SINGLE;
	math::vec particleColor = math::vec::one;
	math::vec gradient[2] = { math::vec::zero, math::vec::one };
	bool useOpacity = false, opacityWithCurve = false;  float opacity = 1.0f;

	bool emitlight = false;
	math::vec lightColor = math::vec::one;
	bool randomLightColor = false;
	float specular = 0.2f;
	float sClamp[2] = { 0.f, 1.f };
	bool randomSpecular = false;
	bool particleLColor = false;

	// Attenuattion
	float iClamp[2] = { 0.0f, 50.0f };
	float intensity = 1.0f;
	bool randomIntensity = false;
	float constant = 1.0f;
	float linear = 0.091f;
	float quadratic = 0.011f;

	const char* materialMD5 = nullptr;
	bool useTextures = false;
	const char* meshMD5 = nullptr;
	RE_CompPrimitive* primCmp = nullptr;

	math::float3 scale = { 0.5f,0.5f,0.1f };

	enum Particle_Dir : int
	{
		PS_FromPS,
		PS_Billboard,
		PS_Custom
	} particleDir = PS_Billboard;
	math::float3 direction = { -1.0f,1.0f,0.5f };

	//Curves
	bool useCurve = false;
	bool smoothCurve = false;
	eastl::vector< ImVec2> curve;
	int total_points = 10;
};
/*
struct Sphere
{
	Vector m_position;  
	Vector m_velocity;  
	float  m_radius;
	float  m_mass; 
	bool collide(Sphere& sphere);  
	bool resolveIntersection(Sphere& sphere, const Vector& mtd); 
	bool resolveCollision(Sphere& sphere, const Vector& Normal); 
	bool colliding(const Sphere& sphere) const;
}; 

bool Sphere::colliding(const Sphere& sphere, Vector& mtd) const 
{    // delta vector  
	Vector delta = (m_position - sphere.m_position);
	float r = m_radius + sphere.m_radius;
	// square distance
	vector dist2 = delta.dotProduct(delta);
	// square distance > radius squared. no collision
	if(dist2 > r*r) return false;
	// find the mtd (or the amount of intersection along the delta vector).
	float dist = sqrt(dist2);
	mtd = delta * (r - dist) / dist;
	return true;
}

bool Sphere::resolveIntersection(Sphere& sphere, const Vector& mtd)
{    // inverse mass quantities
	float im1 = 1 / m_mass;
	float im2 = 1 / sphere.m_mass;
	// resolve intersection
	m_position += mtd * (im1 / (im1 + im2));
	sphere.m_position -= mtd * (im2 / (im1 + im2));
	return true;
}

bool Sphere::resolveCollision(Sphere& sphere, const Vector& n){
	const float cor = 0.7f;
	// inverse mass quantities 
	float im1 = 1 / m_mass;
	float im2 = 1 / sphere.m_mass;
	// impact speed
	Vector v = (m_velocity - sphere.m_velocity);
	float vn = v.dotProduct(n);
	// sphere intersecting but moving away from each other already
	if(vn > 0.0f) return false;
	// collision impulse 
	float i = (-(1.0f + cor) * vn) / (im1 + im2);
	Vector impulse = n * i;
	// change in momentum
	m_velocity += impulse * im1;
	sphere.m_velocity -= impulse * im2;
	return true;
}

bool Sphere::collide(Sphere& sphere)
{    
	Vector mtd;
	Vector normal;
	if(!colliding(sphere, mtd))
		return false;

	normal = (m_position - sphere.m_position);
	normal.normalise();
	resolveIntersection(sphere, mtd);
	resolveCollision(sphere, normal);
	return true;
}*/


#endif //!__RE_PARTICLEEMITTER_H__