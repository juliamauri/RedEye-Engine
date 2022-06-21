#ifndef __RE_PARTICLE_RENDER__
#define __RE_PARTICLE_RENDER__

#include "RE_ParticleEmitter.h"

class RE_ParticleRender : public ResourceContainer
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

	void FillEmitter(class RE_ParticleEmitter* to_fill);
	void FillResouce(class RE_ParticleEmitter* from);

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
	RE_PR_Light light = {};

	const char* meshMD5 = nullptr;
	ComponentType primType = C_POINT;
	RE_CompPrimitive* primCmp = nullptr;

	math::float3 scale = { 0.5f,0.5f,0.1f };

	RE_ParticleEmitter::ParticleDir particleDir = RE_ParticleEmitter::ParticleDir::Billboard;
	math::float3 direction = { -1.0f,1.0f,0.5f };
};

#endif // !__RE_PARTICLE_RENDER__