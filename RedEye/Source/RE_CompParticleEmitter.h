#ifndef __RE_COMPPARTICLEEMITTER_H__
#define __RE_COMPPARTICLEEMITTER_H__

#include "RE_Component.h"
#include "MathGeoLib/include/Math/float3.h"
#include <vector>

class RE_ParticleEmitter;

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

	eastl::vector<const char*> GetAllResources() override;
	void UseResources();
	void UnUseResources();

	bool isLighting() const;
	void CallLightShaderUniforms(unsigned int shader, const char* array_unif_name, unsigned int& count, unsigned int maxLights, bool sharedLight) const;

	bool isBlend() const;

	RE_ParticleEmitter* GetSimulation() const;
	const char* GetEmitterResource()const;
	void UpdateEmitter(const char* emitter);

	void SetEmitter(const char* md5);

private:

	RE_ParticleEmitter* simulation = nullptr;
	const char* emitter_md5 = nullptr;
};

#endif // !__RE_COMPPARTICLEEMITTER_H__