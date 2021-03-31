#ifndef __RE_COMPPARTICLEEMITER_H__
#define __RE_COMPPARTICLEEMITER_H__

#include "RE_Component.h"
#include "MathGeoLib/include/Math/float3.h"
#include <vector>

class Particle;
class RE_Mesh;
struct RE_ParticleEmitter;
class RE_CompPrimitive;

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

	void UseResources();
	void UnUseResources();

	bool isLighting() const;
	void CallLightShaderUniforms(unsigned int shader, const char* array_unif_name, unsigned int& count, unsigned int maxLights, bool sharedLight) const;

private:

	RE_ParticleEmitter* simulation = nullptr;

	bool draw = false;

	bool emitlight = false;
	math::vec lightColor = math::vec::one;
	float specular = 0.2f;
	bool particleLColor = false;

	// Attenuattion
	float intensity = 1.0f;
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

	// Triangle "Point"
	unsigned int VAO, VBO, EBO;
};

#endif // !__RE_COMPPARTICLEEMITER_H__