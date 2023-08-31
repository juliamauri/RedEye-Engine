#ifndef __RE_TEXTURE_H__
#define __RE_TEXTURE_H__

#include "Resource.h"
#include "RE_TextureSettings.h"

class RE_Texture : public ResourceContainer
{
public:
	RE_Texture() = default;
	RE_Texture(const char* metaPath) : ResourceContainer(metaPath) {}
	~RE_Texture() final = default;

	const char* GenerateMD5();
	RE_TextureSettings::Type DetectExtension();
	RE_TextureSettings::Type GetTextureType() const;

	void LoadInMemory() override;
	void UnloadMemory() override;

	void Import(bool keepInMemory = true) override;

	void use();
	void GetWithHeight(int* w, int* h);
	void DrawTextureImGui();
	uint GetID()const { return ID; }

	static int GetComboFilter(RE_TextureSettings::Filter filter);
	static RE_TextureSettings::Filter GetFilterCombo(int combo);

	static int GetComboWrap(RE_TextureSettings::Wrap wrap);
	static RE_TextureSettings::Wrap GetWrapCombo(int combo);

	void ReImport() override;

private:

	void Draw() override;
	void SaveResourceMeta(RE_Json* metaNode) override; 
	void LoadResourceMeta(RE_Json* metaNode) override; 

	void AssetLoad();
	void LibraryLoad();
	void LibrarySave();

	void TexParameteri(uint pname, int param);
	void TexParameterfv(uint pname, float* param);
											  
private:

	uint ID = 0;

	int width = -1;
	int height = -1;

	RE_TextureSettings::Type texType = RE_TextureSettings::Type::TEXTURE_UNKNOWN;
	RE_TextureSettings texSettings;

	bool applySave = false;
	RE_TextureSettings restoreSettings;
};

#endif // !__RE_TEXTURE_H__