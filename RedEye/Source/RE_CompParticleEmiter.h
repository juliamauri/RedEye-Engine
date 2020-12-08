#ifndef __RE_COMPPARTICLEEMITER_H__
#define __RE_COMPPARTICLEEMITER_H__

#include "RE_Component.h"
#include "MathGeoLib/include/Math/float3.h"
#include <vector>

class Particle;
class RE_Mesh;

enum Particle_Stat
{
	PS_CameraPosition, //Watch to camera
	PS_CameraDiretion, //Watch to direction from camera
	PS_Free
};

class RE_CompParticleEmitter : public RE_Component
{
public:
	RE_CompParticleEmitter();
	~RE_CompParticleEmitter();

	void Init() override;
	void CleanUp() override;

	void PreUpdate() override;
	void Update() override;
	void PostUpdate() override;

	void OnPlay() override;
	void OnPause() override;
	void OnStop() override;

	void Draw() const override;
	void DrawProperties() override;

	bool LocalEmission() const;
	bool EmmissionFinished() const;

	RE_Mesh* GetMesh() const;

	//void Serialize(JSONNode* node, rapidjson::Value* val) override;

	void SetUp(RE_Mesh* paticle, unsigned int shader);

private:

	void ResetParticle(Particle* p);
	void UpdateParticles(int spawns_needed);

private:

	int max_particles = 0;
	float time_counter = 0.0f;
	float spawn_counter = 0.0f;

	float emissor_life = -1.0f;

	// Emissor Values
	float emissionRate = 3.0f;
	math::vec spawn_position_offset = math::vec::zero;
	math::vec gravity = math::vec::zero;
	bool local_emission = true;

	// Particle Spawned Info
	float lifetime = 1.0f;
	float initial_speed = 0.f;

	// Margins
	math::vec direction_margin = math::vec::zero;
	float speed_margin = 0.f;
	float lifetime_margin = 0.f;

	// Particle Drawing
	math::vec rgb_alpha = math::vec::zero;
	unsigned int shader = 0;

	Particle* particles = nullptr;
	RE_Mesh* mParticle = nullptr;
};

#endif // !__RE_COMPPARTICLEEMITER_H__