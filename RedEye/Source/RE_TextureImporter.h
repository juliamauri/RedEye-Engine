#ifndef __RETEXTUREIMPORTER_H__
#define __RETEXTUREIMPORTER_H__

#include "Resource.h"

class RE_FileIO;
class RE_Texture;
struct RE_TextureSettings;
enum TextureType;
struct RE_SkyBoxSettings;

class RE_TextureImporter
{
public:
	RE_TextureImporter(const char* folderPath = "Images/");
	~RE_TextureImporter();

	bool Init();

	const char* AddNewTextureOnResources(const char* assetsPath);

	eastl::string TransformToDDS(const char* assetBuffer, unsigned int assetSize, TextureType assetType, unsigned int* newSize);

	void LoadTextureInMemory(const char* buffer, unsigned int size, TextureType type, unsigned int* ID, int* width, int* height, RE_TextureSettings settings);
	void SaveOwnFormat(const char* assetBuffer, unsigned int assetSize, TextureType assetType, RE_FileIO* toSave);

	void LoadSkyBoxInMemory(RE_SkyBoxSettings& settings, unsigned int* ID, bool isDDS = false);

private:
	const char* folderPath = nullptr;
};

#endif // !__RETEXTUREIMPORTER_H__