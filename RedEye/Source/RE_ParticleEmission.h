#ifndef __RE_PARTICLEEMISSION_H__
#define __RE_PARTICLEEMISSION_H__

#include "Resource.h"
#include "RE_EmissionData.h"

class RE_ParticleEmission : public ResourceContainer
{
public:
	RE_ParticleEmission() {}
	RE_ParticleEmission(const char* metapath) : ResourceContainer(metapath) {}
	~RE_ParticleEmission() {}

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
	void BinarySerialize() const;
	unsigned int GetBinarySize() const;

private:

	// Playback
	bool loop = true;
	float max_time = 5.f;
	float start_delay = 0.0f;
	float time_muliplier = 1.f;

	// Spawning
	unsigned int max_particles = 1000u;
	RE_EmissionInterval spawn_interval = {};
	RE_EmissionSpawn spawn_mode = {};

	// Instantiation
	RE_EmissionSingleValue initial_lifetime = {};
	RE_EmissionShape initial_pos = {};
	RE_EmissionVector initial_speed = {};

	// Physics
	RE_EmissionExternalForces external_acc = {};
	RE_EmissionBoundary boundary = {};
	RE_EmissionCollider collider = {};
};

#endif //!__RE_PARTICLEEMISSION_H__
