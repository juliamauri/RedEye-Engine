#ifndef __PARTICLE_H__
#define __PARTICLE_H__

#include "RE_CompTransform.h"
class RE_CompParticleEmitter;
class RE_Mesh;

class Particle {
private:
	RE_CompParticleEmitter* parent_emiter = nullptr;
	RE_CompTransform transform;
	bool isunlinked_from_parent = false;

	RE_Mesh* mesh = nullptr;
};

#endif // !__PARTICLE_H__