#ifndef __RE_PARTICLEEMITTERBASE_H__
#define __RE_PARTICLEEMITTERBASE_H__

#include "Resource.h"

class RE_ParticleEmitterBase : public ResourceContainer
{
public:
	RE_ParticleEmitterBase() {}
	RE_ParticleEmitterBase(const char* metapath) : ResourceContainer(metapath) {}
	~RE_ParticleEmitterBase() {}

	void LoadInMemory() override;
	void UnloadMemory() override;

	void SomeResourceChanged(const char* resMD5);

	class RE_ParticleEmitter* GetNewEmitter();

private:

	void Draw() override;

	void SaveResourceMeta(RE_Json* metaNode) override;
	void LoadResourceMeta(RE_Json* metaNode) override;

private:
	const char* resource_emission = nullptr;
	const char* resource_renderer = nullptr;
};


#endif //!__RE_PARTICLEEMITTERBASE_H__