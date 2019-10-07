#ifndef __RETEXTUREIMPORTER_H__
#define __RETEXTUREIMPORTER_H__

#include "Resource.h"

#include <map> 

class RE_FileIO;

struct Texture2D : ResourceContainer
{
private:
	unsigned int ID = 0;
	int width, height;

public:
	Texture2D(unsigned int ID, int widht, int height);
	~Texture2D();

	void use();
	void GetWithHeight(int* w, int* h);
	void DrawTextureImGui();
};

class RE_TextureImporter
{
public:
	RE_TextureImporter(const char* folderPath);
	~RE_TextureImporter();

	bool Init();

	const char* LoadTextureAssets(const char* assetsPath);
	const char* LoadTextureLibrary(const char* libraryPath, const char* assetsPath);

private:
	Texture2D* ProcessTexture(RE_FileIO* fileTexture, int ILextension, bool generateOwnFormat = false, const char* md5Generated = nullptr);

	int GetExtensionIL(const char* extension);

private:
	const char* folderPath = nullptr;
};

#endif // !__RETEXTUREIMPORTER_H__