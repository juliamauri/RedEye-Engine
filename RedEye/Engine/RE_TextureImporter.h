#ifndef __RE_TEXTURE_IMPORTER_H__
#define __RE_TEXTURE_IMPORTER_H__

#include "RE_TextureSettings.h"
#include <IL/il.h>

class RE_FileBuffer;

namespace RE_TextureImporter
{
	bool Init();

	const char* AddNewTextureOnResources(const char* assetsPath);
	const char* TransformToDDS(const void* assetBuffer, ILuint assetSize, TextureType assetType, ILuint* newSize);
	void LoadTextureInMemory(const void* buffer, ILuint size, TextureType type, ILuint* ID, ILint* width, ILint* height, RE_TextureSettings settings);
	void SaveOwnFormat(const void* assetBuffer, ILuint assetSize, TextureType assetType, RE_FileBuffer* toSave);
};

#endif // !__RE_TEXTURE_IMPORTER_H__