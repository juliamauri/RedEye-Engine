#ifndef __PARTICLE_H__
#define __PARTICLE_H__

#include "RE_CompTransform.h"
class RE_CompParticleEmitter;
class RE_Mesh;

class Particle {
public:
	void Update();
	bool Alive();
	void Emit();

	void Draw(unsigned int shader);

	void SetUp(RE_CompParticleEmitter* pe);

public:
	RE_CompParticleEmitter* parent_emiter = nullptr;

	float current_lifetime = 0.0f;
	float max_lifetime = 0.0f;

	math::vec position = math::vec::zero;

	math::vec right = math::vec::zero;
	math::vec up = math::vec::zero;
	math::vec front = math::vec::zero;

	float speed = 0.f;
	float lifetime = 100.0f;
};

#endif // !__PARTICLE_H__