#ifndef __RE_TEXTURE_H__
#define __RE_TEXTURE_H__

#include "Resource.h"

#include "MathGeoLib/include/Math/float4.h"

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

//Same as GL values
enum RE_TextureFilters {
	RE_NEAREST = 0x2600,
	RE_LINEAR = 0x2601,
	RE_NEAREST_MIPMAP_NEAREST = 0x2700,
	RE_LINEAR_MIPMAP_NEAREST = 0x2701,
	RE_NEAREST_MIPMAP_LINEAR = 0x2702,
	RE_LINEAR_MIPMAP_LINEAR = 0x2703,
};
enum RE_TextureWrap {
	RE_REPEAT = 0x2901,
	RE_CLAMP_TO_BORDER = 0x812D,
	RE_CLAMP_TO_EDGE = 0x812F,
	RE_MIRRORED_REPEAT = 0x8370
};

struct RE_TextureSettings {
	RE_TextureFilters min_filter = RE_NEAREST;
	RE_TextureFilters mag_filter = RE_NEAREST;
	RE_TextureWrap wrap_s = RE_REPEAT;
	RE_TextureWrap wrap_t = RE_REPEAT;
	math::float4 borderColor = { 0.0f, 0.0f, 0.0f, 0.0f };

	inline bool operator==(const RE_TextureSettings &b) {
		return (min_filter == b.min_filter && mag_filter == b.mag_filter && wrap_s == b.wrap_s && wrap_t == b.wrap_t && borderColor.Equals(b.borderColor));
	}


	inline bool operator!=(const RE_TextureSettings &b) {
		return (min_filter != b.min_filter || mag_filter != b.mag_filter || wrap_s != b.wrap_s || wrap_t != b.wrap_t || !borderColor.Equals(b.borderColor));
	}
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

	static int GetComboFilter(RE_TextureFilters filter);
	static RE_TextureFilters GetFilterCombo(int combo);

	static int GetComboWrap(RE_TextureWrap wrap);
	static RE_TextureWrap GetWrapCombo(int combo);

private:
	void Draw() override;
	void SaveResourceMeta(JSONNode* metaNode) override; 
	void LoadResourceMeta(JSONNode* metaNode) override; 

	void AssetLoad();
	void LibraryLoad();
	void LibrarySave();

	void TexParameteri(unsigned int pname, int param);
	void TexParameterfv(unsigned int pname, float* param);
											  
private:
	unsigned int ID = 0;
	int width, height;
	TextureType texType = RE_TEXTURE_UNKNOWN;
	RE_TextureSettings texSettings;

	bool applySave = false;
	RE_TextureSettings restoreSettings;
};

#endif // !__RE_TEXTURE_H__