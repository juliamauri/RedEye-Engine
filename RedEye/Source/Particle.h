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
	RE_CompTransform transform;
	bool isunlinked_from_parent = false;

	RE_Mesh* mesh = nullptr;

	bool isEmitted_flag = false;

	float lifetime = 0.0f;
	math::vec position = math::vec::zero;
	math::vec speed = math::vec::zero;
	math::vec gravity = math::vec::zero;
};

#endif // !__PARTICLE_H__