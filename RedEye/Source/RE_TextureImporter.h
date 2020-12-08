#ifndef __RE_TEXTURE_IMPORTER_H__
#define __RE_TEXTURE_IMPORTER_H__

#include "RE_Texture.h"
#include "RE_SkyBox.h"

class RE_FileBuffer;

class RE_TextureImporter
{
public:
	RE_TextureImporter(const char* folderPath = "Images/") : folderPath(folderPath) {}
	~RE_TextureImporter() {}

	bool Init();

	const char* AddNewTextureOnResources(const char* assetsPath);

	eastl::string TransformToDDS(const char* assetBuffer, unsigned int assetSize, TextureType assetType, unsigned int* newSize);

	void LoadTextureInMemory(const char* buffer, unsigned int size, TextureType type, unsigned int* ID, int* width, int* height, RE_TextureSettings settings);
	void SaveOwnFormat(const char* assetBuffer, unsigned int assetSize, TextureType assetType, RE_FileBuffer* toSave);

	void LoadSkyBoxInMemory(RE_SkyBoxSettings& settings, unsigned int* ID, bool isDDS = false);

private:
	const char* folderPath = nullptr;
};

#endif // !__RE_TEXTURE_IMPORTER_H__