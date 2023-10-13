#ifndef __RE_PARTICLEEMISSION_H__
#define __RE_PARTICLEEMISSION_H__

#include "RE_EmissionInterval.h"
#include "RE_EmissionSpawn.h"
#include "RE_EmissionSingleValue.h"
#include "RE_EmissionShape.h"
#include "RE_EmissionVector.h"
#include "RE_EmissionExternalForces.h"
#include "RE_EmissionBoundary.h"
#include "RE_EmissionCollider.h"

class RE_ParticleEmitter;

class RE_ParticleEmission : public ResourceContainer
{
private:

	// Playback
	bool loop = true;
	float max_time = 5.f;
	float start_delay = 0.f;
	float time_muliplier = 1.f;

	// Spawning
	uint max_particles = 1000;
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

public:

	RE_ParticleEmission() = default;
	RE_ParticleEmission(const char* metapath) : ResourceContainer(metapath) {}
	~RE_ParticleEmission() = default;

	void LoadInMemory() override;
	void UnloadMemory() override;
	void Import(bool keepInMemory = true) override;

	void Save();
	void ProcessMD5();

	void FillEmitter(RE_ParticleEmitter* to_fill);
	void FillResouce(RE_ParticleEmitter* from);

private:

	void JsonSerialize(bool onlyMD5 = false); //We need to call ProcessMD5() before SaveMeta
	void JsonDeserialize(bool generateLibraryPath = false);

	size_t GetBinarySize() const;
	void BinarySerialize() const;
	void BinaryDeserialize();
};

#endif //!__RE_PARTICLEEMISSION_H__
