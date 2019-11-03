#ifndef __RE_TEXTURE_H__
#define __RE_TEXTURE_H__

#include "Resource.h"

//Same as image types defines on  il.h DevIL
enum TextureType {
	RE_TEXTURE_UNKNOWN = 0x0000,
	RE_BMP = 0x0420,
	RE_JPG = 0x0425,
	RE_PNG = 0x042A,
	RE_TGA = 0x042D,
	RE_TIFF = 0x042E,
	RE_DDS = 0x0437
};

//TODO
struct RE_TextureSettings {

};


class RE_Texture :
	public ResourceContainer
{
public:
	RE_Texture();
	RE_Texture(const char* metaPath);
	~RE_Texture();

	const char* GenerateMD5();
	TextureType DetectExtension();

	void LoadInMemory() override;
	void UnloadMemory() override;

	void use();
	void GetWithHeight(int* w, int* h);
	void DrawTextureImGui();
	unsigned int GetID()const { return ID; }

private:
	void Draw() override;
	void SaveResourceMeta(JSONNode* metaNode) override; 
	void LoadResourceMeta(JSONNode* metaNode) override; 

	void AssetLoad();
	void LibraryLoad();
	void LibrarySave();
											  
private:
	unsigned int ID = 0;
	int width, height;
	TextureType texType = RE_TEXTURE_UNKNOWN;
	RE_TextureSettings texSettings;
};

#endif // !__RE_TEXTURE_H__