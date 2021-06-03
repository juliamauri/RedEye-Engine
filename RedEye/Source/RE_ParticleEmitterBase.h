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
	void Import(bool keepInMemory = true) override;

	void Save();
	void ProcessMD5();

	class RE_ParticleEmitter* GetNewEmitter();

private:

	void Draw() override;

	void SaveResourceMeta(RE_Json* metaNode) override;
	void LoadResourceMeta(RE_Json* metaNode) override;

	void JsonDeserialize(bool generateLibraryPath = false);
	void JsonSerialize(bool onlyMD5 = false); //We need to call ProcessMD5() before SaveMeta

	void BinaryDeserialize();
	void BinarySerialize();
	unsigned int GetBinarySize() const;

private:
	const char* resource_emission;
	const char* resource_renderer;
};


#endif //!__RE_PARTICLEEMITTERBASE_H__