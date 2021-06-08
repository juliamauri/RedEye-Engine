#pragma once
#include "Resource.h"

#include "RE_ParticleEmitter.h"
#include "RE_Component.h"

class RE_ParticleRender :
    public ResourceContainer
{
public:
	RE_ParticleRender() {}
	RE_ParticleRender(const char* metapath) : ResourceContainer(metapath) {}
	~RE_ParticleRender() {}

	void LoadInMemory() override;
	void UnloadMemory() override;
	void Import(bool keepInMemory = true) override;

	void Save();
	void ProcessMD5();

private:

	//void Draw() override;

	void SaveResourceMeta(RE_Json* metaNode) override;
	void LoadResourceMeta(RE_Json* metaNode) override;

	void JsonDeserialize(bool generateLibraryPath = false);
	void JsonSerialize(bool onlyMD5 = false); //We need to call ProcessMD5() before SaveMeta

	void BinaryDeserialize();
	void BinarySerialize();
	unsigned int GetBinarySize() const;

private:
	RE_PR_Color color = {};
	RE_PR_Opacity opacity = {};

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

	const char* meshMD5 = nullptr;
	ComponentType primCmp = C_POINT;

	math::float3 scale = { 0.5f,0.5f,0.1f };

	RE_ParticleEmitter::Particle_Dir particleDir = RE_ParticleEmitter::Particle_Dir::PS_Billboard;
	math::float3 direction = { -1.0f,1.0f,0.5f };
};