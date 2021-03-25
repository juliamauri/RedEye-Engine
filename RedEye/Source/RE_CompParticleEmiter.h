#ifndef __RE_COMPPARTICLEEMITER_H__
#define __RE_COMPPARTICLEEMITER_H__

#include "RE_Component.h"
#include "MathGeoLib/include/Math/float3.h"
#include <vector>

class Particle;
class RE_Mesh;
struct RE_ParticleEmitter;

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

	void CopySetUp(GameObjectsPool* pool, RE_Component* copy, const UID parent) override;

	void Update() override;

	void Draw() const override;
	void DrawProperties() override;

	void SerializeJson(RE_Json* node, eastl::map<const char*, int>* resources) const override;
	void DeserializeJson(RE_Json* node, eastl::map<int, const char*>* resources) override;

	unsigned int GetBinarySize() const override;
	void SerializeBinary(char*& cursor, eastl::map<const char*, int>* resources) const override;
	void DeserializeBinary(char*& cursor, eastl::map<int, const char*>* resources) override;

	bool isLighting() const;
	void CallLightShaderUniforms(unsigned int shader, const char* array_unif_name, unsigned int& count, unsigned int maxLights) const;

private:

	RE_ParticleEmitter* simulation = nullptr;

	bool emitlight = false;
	math::vec lightColor = math::vec::one;

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

	unsigned int VAO, VBO, EBO;
	bool draw = false;
};

#endif // !__RE_COMPPARTICLEEMITER_H__