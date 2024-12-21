#ifndef __RE_COMPPARTICLEEMITTER_H__
#define __RE_COMPPARTICLEEMITTER_H__

class RE_ParticleEmitter;

class RE_CompParticleEmitter : public RE_Component
{
public:

	RE_CompParticleEmitter::RE_CompParticleEmitter() : RE_Component(RE_Component::Type::PARTICLEEMITER) {}
	~RE_CompParticleEmitter() final = default;

	void CopySetUp(GameObjectsPool* pool, RE_Component* copy, const GO_UID parent) final;

	void Update() final;

	void Draw() const final;
	void DrawProperties() final;

	void SerializeJson(RE_Json* node, eastl::map<const char*, int>* resources) const final;
	void DeserializeJson(RE_Json* node, eastl::map<int, const char*>* resources) final;

	size_t GetBinarySize() const final;
	void SerializeBinary(char*& cursor, eastl::map<const char*, int>* resources) const final;
	void DeserializeBinary(char*& cursor, eastl::map<int, const char*>* resources) final;

	eastl::vector<const char*> GetAllResources() final;
	void UseResources();
	void UnUseResources();

	bool HasLight() const;
	void CallLightShaderUniforms(unsigned int shader, const char* array_unif_name, unsigned int& count, unsigned int maxLights, bool sharedLight) const;

	bool isBlend() const;

	RE_ParticleEmitter* GetSimulation() const;
	const char* GetEmitterResource() const;
	void UpdateEmitter(const char* emitter);

	void SetEmitter(const char* md5);

private:

	RE_ParticleEmitter* simulation = nullptr;
	const char* emitter_md5 = nullptr;
};

#endif // !__RE_COMPPARTICLEEMITTER_H__