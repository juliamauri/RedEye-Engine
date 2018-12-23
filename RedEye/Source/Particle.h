#ifndef __PARTICLE_H__
#define __PARTICLE_H__

#include "RE_CompTransform.h"
class RE_CompParticleEmitter;
class RE_Mesh;

class Particle {
public:
	void Update();

	bool isEmitted();
	void Emit(math::vec spawn, math::vec s, math::vec g, float lt);

	void Draw(unsigned int shader);

	void SetUp(RE_CompParticleEmitter* pe, RE_Mesh* mesh);

private:
	RE_CompParticleEmitter* parent_emiter = nullptr;
	//RE_CompTransform transform;

	RE_Mesh* mesh = nullptr;

	bool isEmitted_flag = false;
	bool position_is_local = false;

	// life
	float current_lifetime = 0.0f;
	float max_lifetime = 0.0f;

	math::vec position = math::vec::zero;

	math::vec right = math::vec::zero;
	math::vec up = math::vec::zero;
	math::vec front = math::vec::zero;

	float speed = 0.f;
	float lifetime = 1.0f;
};

#endif // !__PARTICLE_H__