#pragma once
#include "Resource.h"
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


};

