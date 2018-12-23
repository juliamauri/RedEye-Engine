#ifndef __RE_COMPPARTICLEEMITER_H__
#define __RE_COMPPARTICLEEMITER_H__

#define MAX_PARTICLES 200

#include <vector>

#include "RE_Component.h"
#include "Particle.h"

enum Particle_Stat
{
	PS_CameraPosition, //Watch to camera
	PS_CameraDiretion, //Watch to direction from camera
	PS_Free
};

class RE_CompParticleEmitter : public RE_Component
{
public:
	RE_CompParticleEmitter(RE_GameObject* go = nullptr);
	~RE_CompParticleEmitter();

	void Init();
	void CleanUp();

	void PreUpdate();
	void Update();
	void PostUpdate();

	void Draw();
	void DrawProperties();

	void Serialize(JSONNode* node, rapidjson::Value* val);

private:
	Particle* particles = nullptr;

	math::vec point_particle_spawn = math::vec::zero;
	math::vec direction_particle_spawn = math::vec::zero;
	math::vec gravity_particle = math::vec::zero;

	float angle_margin = 0.0f;

	float duration = -1.0f;
	float lifetime = 1.0f;
	float lifetime_margin = 0.0f;
	int emisionRate = 50;
	math::vec speed_particle = math::vec::zero;
	math::vec speed_margin = math::vec::zero;

	math::vec rgb_alpha = math::vec::zero;

	RE_Mesh* mParticle = nullptr;
};

#endif // !__RE_COMPPARTICLEEMITER_H__