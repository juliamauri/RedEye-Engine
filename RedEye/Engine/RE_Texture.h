#ifndef __RE_TEXTURE_H__
#define __RE_TEXTURE_H__

#include "Resource.h"
#include "RE_TextureSettings.h"

class RE_Texture : public ResourceContainer
{
public:
	RE_Texture() {}
	RE_Texture(const char* metaPath) : ResourceContainer(metaPath) {}
	~RE_Texture() {}

	const char* GenerateMD5();
	TextureType DetectExtension();
	TextureType GetTextureType() const;

	void LoadInMemory() override;
	void UnloadMemory() override;

	void Import(bool keepInMemory = true) override;

	void use();
	void GetWithHeight(int* w, int* h);
	void DrawTextureImGui();
	unsigned int GetID()const { return ID; }

	static int GetComboFilter(RE_TextureFilters filter);
	static RE_TextureFilters GetFilterCombo(int combo);

	static int GetComboWrap(RE_TextureWrap wrap);
	static RE_TextureWrap GetWrapCombo(int combo);

	void ReImport() override;

private:

	void Draw() override;
	void SaveResourceMeta(RE_Json* metaNode) override; 
	void LoadResourceMeta(RE_Json* metaNode) override; 

	void AssetLoad();
	void LibraryLoad();
	void LibrarySave();

	void TexParameteri(unsigned int pname, int param);
	void TexParameterfv(unsigned int pname, float* param);
											  
private:

	unsigned int ID = 0;
	int width = -1, height = -1;
	TextureType texType = RE_TEXTURE_UNKNOWN;
	RE_TextureSettings texSettings;

	bool applySave = false;
	RE_TextureSettings restoreSettings;
};

#endif // !__RE_TEXTURE_H__