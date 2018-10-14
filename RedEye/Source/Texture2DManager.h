#ifndef __TEXTURE2DMANAGER_H__
#define __TEXTURE2DMANAGER_H__

#include <map>
#include <vector>
#include <list>

enum ImageExtensionType
{
	PNG,
	JPG,
	DDS
};

struct Texture2D
{
private:
	unsigned int ID = 0;
	int width, height;
	std::string name;

public:
	Texture2D(const char* path, int extension,const char* name, bool droped = false);
	~Texture2D();

	void use();
	void GetWithHeight(int* w,int* h);
	void DrawTextureImGui();
	const char* GetName();
	const unsigned int GetID();
};

class Texture2DManager 
{
public:
	Texture2DManager(const char* folderPath);
	~Texture2DManager();

	bool Init();

	//Load texture
	unsigned int LoadTexture2D(const char* name, ImageExtensionType extension);
	unsigned int LoadTexture2D(const char* path, const char* file_name, bool droped = false);

	void use(unsigned int TextureID);
	void drawTexture(unsigned int TextureID);
	void GetWithHeight(unsigned int TextureID, int* w, int* h);

	void DeleteTexture2D(unsigned int TextureID);

	std::vector<Texture2D*>* GetTextures();

private:
	const char* folderPath;
	unsigned int ID_count = 0;
	bool texturesmodified = false;

	std::list<unsigned int> textureIDContainer;
	std::map<unsigned int, Texture2D*> textures2D;
	std::vector<Texture2D*> actualTextures;

	const char* GetExtensionStr(ImageExtensionType imageType);
	int GetExtensionIL(const char* ext);
};




#endif // !__TEXTURE2DMANAGER_H__