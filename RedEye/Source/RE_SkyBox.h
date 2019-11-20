#ifndef __RE_SKYBOX_H__
#define __RE_SKYBOX_H__

#include "RE_Texture.h"

#include <string>

#define MAXSKYBOXTEXTURES  6

enum RE_TextureFace {
	RE_NOFACE = -1,
	RE_RIGHT,
	RE_LEFT,
	RE_TOP,
	RE_BOTTOM,
	RE_FRONT,
	RE_BACK
};

struct TexSkyBox {
	TexSkyBox(RE_TextureFace f) { face = f; }

	std::string assetPath;
	int width, height;
	TextureType texType = RE_TEXTURE_UNKNOWN;
	RE_TextureFace face = RE_NOFACE;

	inline bool operator==(const TexSkyBox& b) {
		return (width == b.width && height == b.height && texType == b.texType && assetPath == b.assetPath && face == b.face);
	}


	inline bool operator!=(const TexSkyBox& b) {
		return (width != b.width || height != b.height || texType != b.texType || assetPath != b.assetPath || face != b.face);
	}
};

struct RE_SkyBoxSettings {
	RE_TextureFilters min_filter = RE_LINEAR;
	RE_TextureFilters mag_filter = RE_LINEAR;
	RE_TextureWrap wrap_s = RE_CLAMP_TO_EDGE;
	RE_TextureWrap wrap_t = RE_CLAMP_TO_EDGE;
	RE_TextureWrap wrap_r = RE_CLAMP_TO_EDGE;

	TexSkyBox textures[6] = { TexSkyBox(RE_RIGHT), TexSkyBox(RE_LEFT), TexSkyBox(RE_TOP), TexSkyBox(RE_BOTTOM), TexSkyBox(RE_FRONT), TexSkyBox(RE_BACK) };
	float skyBoxSize = 0.0f;

	inline bool operator==(const RE_SkyBoxSettings& b) {
		return (min_filter == b.min_filter && mag_filter == b.mag_filter && wrap_s == b.wrap_s && wrap_t == b.wrap_t && wrap_r == b.wrap_r && skyBoxSize == skyBoxSize
			&& textures[0] == b.textures[0] && textures[1] == b.textures[1] && textures[2] == b.textures[2] && textures[3] == b.textures[3] && textures[4] == b.textures[4] && textures[5] == b.textures[5]);
	}


	inline bool operator!=(const RE_SkyBoxSettings& b) {
		return (min_filter != b.min_filter || mag_filter != b.mag_filter || wrap_s != b.wrap_s || wrap_t != b.wrap_t || wrap_r != b.wrap_r  || skyBoxSize != skyBoxSize
			|| textures[0] != b.textures[0] || textures[1] != b.textures[1] || textures[2] != b.textures[2] || textures[3] != b.textures[3] || textures[4] != b.textures[4] || textures[5] != b.textures[5]);
	}

	inline bool texturesChanged(RE_SkyBoxSettings& b) {
		return (textures[0] == b.textures[0] && textures[1] == b.textures[1] && textures[2] == b.textures[2] && textures[3] == b.textures[3] && textures[4] == b.textures[4] && textures[5] == b.textures[5]);
	}
};

class RE_SkyBox :
	public ResourceContainer
{
public:
	RE_SkyBox();
	RE_SkyBox(const char* metaPath);
	~RE_SkyBox();

	void LoadInMemory() override;
	void UnloadMemory() override;

	void use();
	unsigned int GetID()const { return ID; }
	unsigned int GetVAO()const { return VAO; }

	void AddTexture(RE_TextureFace face, const char* assetPath);
	void AssetSave();

private:
	void Draw() override;
	void SaveResourceMeta(JSONNode* metaNode) override;
	void LoadResourceMeta(JSONNode* metaNode) override;

	void AssetLoad();
	void LibraryLoad();
	void LibrarySave();

	void TexParameteri(unsigned int pname, int param);

	void LoadSkyBoxCube();

private:
	unsigned int ID = 0;
	unsigned int VAO = 0;
	unsigned int VBO = 0;

	RE_SkyBoxSettings skyBoxSettings;

	bool applySize = false;
	bool applyTextures = false;

	bool applySave = false;
	RE_SkyBoxSettings restoreSettings;

	const char* texturesname[6] = { "Right", "Left", "Top", "Bottom", "Front", "Back" };
};

#endif // !__RE_SKYBOX_H__