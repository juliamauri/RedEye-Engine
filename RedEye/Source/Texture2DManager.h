#ifndef __TEXTURE2DMANAGER_H__
#define __TEXTURE2DMANAGER_H__

#include <map>
#include <list>

enum ImageExtensionType
{
	PNG,
	JPG
};

struct Texture2D
{
private:
	unsigned int ID = 0;
	int width, height;

public:
	Texture2D(const char* path, int extension, bool droped = false);
	~Texture2D();

	void use();
	void GetWithHeight(int* w,int* h);
};

class Texture2DManager 
{
public:
	~Texture2DManager();

	bool Init(const char* folderPath);

	//Load texture
	unsigned int LoadTexture2D(const char* name, ImageExtensionType extension);
	unsigned int LoadTexture2D(const char* path, const char* file_name, bool droped = false);

	void use(unsigned int TextureID);
	void GetWithHeight(unsigned int TextureID, int* w, int* h);

	void DeleteTexture2D(unsigned int TextureID);

private:
	const char* folderPath;
	unsigned int ID_count = 0;

	std::list<unsigned int> textureIDContainer;
	std::map<unsigned int, Texture2D*> textures2D;

	const char* GetExtensionStr(ImageExtensionType imageType);
	int GetExtensionIL(ImageExtensionType imageType);
};




#endif // !__TEXTURE2DMANAGER_H__