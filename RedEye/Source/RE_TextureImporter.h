#ifndef __RETEXTUREIMPORTER_H__
#define __RETEXTUREIMPORTER_H__

#include "Resource.h"

#include <map> 

class RE_FileIO;
class RE_Texture;
struct RE_TextureSettings;
enum TextureType;

class RE_TextureImporter
{
public:
	RE_TextureImporter(const char* folderPath);
	~RE_TextureImporter();

	bool Init();

	const char* AddNewTextureOnResources(const char* assetsPath);

	void LoadTextureInMemory(const char* buffer, unsigned int size, TextureType type, unsigned int* ID, int* width, int* height, RE_TextureSettings settings);
	void SaveOwnFormat(const char* assetBuffer, unsigned int assetSize, TextureType assetType, RE_FileIO* toSave);

	unsigned int LoadSkyBoxTextures(const char* texturesPath, const char* extension);
	unsigned int LoadSkyBoxTextures(const char* texturesPath[6]);

private:

	int GetExtensionIL(const char* extension);

private:
	const char* folderPath = nullptr;
};

#endif // !__RETEXTUREIMPORTER_H__