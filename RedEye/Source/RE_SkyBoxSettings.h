#ifndef __RE_SKYBOX_SETTINGS_H__
#define __RE_SKYBOX_SETTINGS_H__

#include "RE_TextureSettings.h"

enum RE_TextureFace {
	RE_NOFACE = -1,
	RE_RIGHT,
	RE_LEFT,
	RE_TOP,
	RE_BOTTOM,
	RE_FRONT,
	RE_BACK
};

struct SkyBoxTexture
{
	SkyBoxTexture(RE_TextureFace f) : face(f) {}

	RE_TextureFace face = RE_NOFACE;
	std::string path;
	const char* textureMD5 = nullptr;

	inline bool operator==(const SkyBoxTexture& b) {
		return (textureMD5 == b.textureMD5 && face == b.face); }
	inline bool operator!=(const SkyBoxTexture& b) {
		return (textureMD5 != b.textureMD5 || face != b.face); }
};

struct RE_SkyBoxSettings
{
	float skyBoxSize = 2000.0f;
	RE_TextureFilters min_filter = RE_LINEAR,  mag_filter = RE_LINEAR;
	RE_TextureWrap wrap_s = RE_CLAMP_TO_EDGE, wrap_t = RE_CLAMP_TO_EDGE, wrap_r = RE_CLAMP_TO_EDGE;

	SkyBoxTexture textures[6] = { SkyBoxTexture(RE_RIGHT), RE_LEFT, RE_TOP, RE_BOTTOM, RE_FRONT, RE_BACK };

	inline bool operator==(const RE_SkyBoxSettings& b) {
		return (min_filter == b.min_filter && mag_filter == b.mag_filter && wrap_s == b.wrap_s && wrap_t == b.wrap_t && wrap_r == b.wrap_r && skyBoxSize == skyBoxSize
			&& textures[0] == b.textures[0] && textures[1] == b.textures[1] && textures[2] == b.textures[2] && textures[3] == b.textures[3] && textures[4] == b.textures[4] && textures[5] == b.textures[5]);
	}

	inline bool operator!=(const RE_SkyBoxSettings& b) {
		return (min_filter != b.min_filter || mag_filter != b.mag_filter || wrap_s != b.wrap_s || wrap_t != b.wrap_t || wrap_r != b.wrap_r || skyBoxSize != skyBoxSize
			|| textures[0] != b.textures[0] || textures[1] != b.textures[1] || textures[2] != b.textures[2] || textures[3] != b.textures[3] || textures[4] != b.textures[4] || textures[5] != b.textures[5]);
	}

	inline bool texturesChanged(RE_SkyBoxSettings& b) {
		return (textures[0] == b.textures[0] && textures[1] == b.textures[1] && textures[2] == b.textures[2] && textures[3] == b.textures[3] && textures[4] == b.textures[4] && textures[5] == b.textures[5]);
	}
};

#endif // !__RE_SKYBOX_SETTINGS_H__