#ifndef __TEXTURE2DMANAGER_H__
#define __TEXTURE2DMANAGER_H__

#include "Resource.h"

#include <map>
#include <vector>
#include <list>

enum ImageExtensionType
{
	PNG,
	JPG,
	DDS
};

struct Texture2D : ResourceContainer
{
private:
	unsigned int ID = 0;
	int width, height;

public:
	Texture2D(unsigned int ID, int widht, int height);
	~Texture2D();

	void use();
	void GetWithHeight(int* w,int* h);
	void DrawTextureImGui();
};

class Texture2DManager 
{
public:
	Texture2DManager(const char* folderPath);
	~Texture2DManager();

	bool Init();

	//Load texture
	const char* LoadTexture2D(const char* name, ImageExtensionType extension);
	const char* LoadTexture2D(const char* path, const char* file_name, bool droped = false);
	const char* LoadTexture2D(const char* path, bool from_Library = false, const char* assets_file = nullptr);
	void use(const char* TextureID);
	void use(Texture2D* TextureID);
	void drawTexture(const char* TextureID);
	void drawTexture(Texture2D* TextureID);
	void GetWithHeight(const char* TextureID, int* w, int* h);

	void DeleteTexture2D(const char* TextureID);

private:

	Texture2D* ProcessTexture(const char* path, int extension, const char* name, bool  droped = false, bool from_Library = false);

	const char* folderPath;
	bool texturesmodified = false;

	const char* GetExtensionStr(ImageExtensionType imageType);
	int GetExtensionIL(const char* ext);

	std::string md5_genereted;
	std::string exists_md5;
};

#endif // !__TEXTURE2DMANAGER_H__