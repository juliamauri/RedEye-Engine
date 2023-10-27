#ifndef __RE_COMPPARTICLEEMITTER_H__
#define __RE_COMPPARTICLEEMITTER_H__

#include "RE_Component.h"
#include <EASTL/vector.h>

class RE_CompParticleEmitter : public RE_Component
{
private:

	P_UID simulation;
	const char* emitter_md5 = nullptr;

public:

	RE_CompParticleEmitter();
	~RE_CompParticleEmitter() final = default;

	void CopySetUp(GameObjectsPool* pool, RE_Component* copy, const GO_UID parent) final;

	void Update() final;

	void Draw() const final;
	void DrawProperties() final;

	bool HasBlend() const;
	bool HasLight() const;

	void UpdateEmitter(const char* emitter);
	void SetEmitter(const char* md5) { emitter_md5 = md5; }

	// Getters
	P_UID GetSimulationID() const { return simulation; }
	const char* GetEmitterResource() const { return emitter_md5; }

	// Resources
	void UseResources() final;
	void UnUseResources() final;
	eastl::vector<const char*> GetAllResources() final;

	// Serialization
	void JsonSerialize(RE_Json* node, eastl::map<const char*, int>* resources) const final;
	void JsonDeserialize(RE_Json* node, eastl::map<int, const char*>* resources) final;

	size_t GetBinarySize() const final;
	void BinarySerialize(char*& cursor, eastl::map<const char*, int>* resources) const final;
	void BinaryDeserialize(char*& cursor, eastl::map<int, const char*>* resources) final;
};

#endif // !__RE_COMPPARTICLEEMITTER_H__