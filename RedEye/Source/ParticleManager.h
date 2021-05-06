#ifndef __RE_PARTICLEMANAGER_H__
#define __RE_PARTICLEMANAGER_H__

#include "MathGeoLib/include/Math/float3.h"
#include "EA/EASTL/utility.h"
#include "EA/EASTL/list.h"

class RE_CompPrimitive;

struct RE_Particle
{
	bool Update(float dt);

	math::vec position = math::vec::zero;
	math::vec speed = math::vec::zero;
	math::vec lightColor = math::vec::one;
	float intensity = 1.0f;
	float specular = .2f;

	float lifetime = 0.0f;
	float max_lifetime = 15.0f;
};

struct RE_ParticleEmitter
{
	int GetNewSpawns(float dt);

	void SetUpParticle(RE_Particle* particle);

	unsigned int id = 0u;

	// Simulation parameters
	enum PlaybackState { STOP = 0, PLAY, PAUSE } state = STOP;
	bool loop = true;
	float spaw_frequency = 3.f;
	float lifetime = 1.5f;

	// Control values
	float spawn_offset = 0.f;
	float speed_muliplier = 2.f;
	
	float maxLifeTime = 1.5f;
	float maxSpeed = 2.f;
	float maxDistance = 2.f * 1.5f; //lifetime * speed

	// Render values
	enum ColorState { SINGLE = 0, OVERLIFETIME, OVERDISTANCE, OVERSPEED };
	ColorState cState = SINGLE;
	math::vec particleColor = math::vec::one;
	math::vec gradient[2] = { math::vec::zero, math::vec::one };

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

	math::float3 scale = { 0.1f,0.1f,0.1f };

	enum Particle_Dir : int
	{
		PS_FromPS,
		PS_Billboard,
		PS_Custom
	};
	Particle_Dir particleDir = PS_Billboard;
	math::float3 direction = { -1.0f,1.0f,0.5f };

};

class ParticleManager
{
public:
	ParticleManager();
	~ParticleManager();

	unsigned int Allocate(RE_ParticleEmitter* emitter);
	bool Deallocate(unsigned int index);


	bool SetEmitterState(unsigned int index, RE_ParticleEmitter::PlaybackState state);

public:

	eastl::list<eastl::pair<RE_ParticleEmitter*, eastl::list<RE_Particle*>*>*> simulations;

private:

	static unsigned int emitter_count;
};

#endif //!__RE_PARTICLEMANAGER_H__