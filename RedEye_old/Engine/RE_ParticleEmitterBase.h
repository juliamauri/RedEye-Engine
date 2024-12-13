#ifndef __RE_PARTICLEEMITTERBASE_H__
#define __RE_PARTICLEEMITTERBASE_H__

class RE_ParticleEmitter;

class RE_ParticleEmitterBase : public ResourceContainer
{
public:
	RE_ParticleEmitterBase() = default;
	RE_ParticleEmitterBase(const char* metapath) : ResourceContainer(metapath) {}
	~RE_ParticleEmitterBase() final = default;

	void LoadInMemory() final;
	void UnloadMemory() final;

	void SomeResourceChanged(const char* resMD5);

	RE_ParticleEmitter* GetNewEmitter();
	void FillEmitter(RE_ParticleEmitter* emitter_to_fill);
	void FillAndSave(RE_ParticleEmitter* for_fill);
	void GenerateSubResourcesAndReference(const char* emission_name, const char* renderer_name);

	void ChangeEmissor(RE_ParticleEmitter* sim, const char* emissor);
	void ChangeRenderer(RE_ParticleEmitter* sim, const char* renderer);

	bool HasEmissor() const;
	bool HasRenderer() const;

	bool Contains(const char* res) const;

private:

	void Draw() final;

	void SaveResourceMeta(RE_Json* metaNode) const final;
	void LoadResourceMeta(RE_Json* metaNode) final;

private:
	const char* resource_emission = nullptr;
	const char* resource_renderer = nullptr;
};


#endif //!__RE_PARTICLEEMITTERBASE_H__