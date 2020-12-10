#ifndef __RE_TEXTURE_IMPORTER_H__
#define __RE_TEXTURE_IMPORTER_H__

#include "RE_TextureSettings.h"

class RE_FileBuffer;

namespace RE_TextureImporter
{
	void Init();

	const char* AddNewTextureOnResources(const char* assetsPath);
	const char* TransformToDDS(const char* assetBuffer, unsigned int assetSize, TextureType assetType, unsigned int* newSize);
	void LoadTextureInMemory(const char* buffer, unsigned int size, TextureType type, unsigned int* ID, int* width, int* height, RE_TextureSettings settings);
	void SaveOwnFormat(const char* assetBuffer, unsigned int assetSize, TextureType assetType, RE_FileBuffer* toSave);
};

#endif // !__RE_TEXTURE_IMPORTER_H__