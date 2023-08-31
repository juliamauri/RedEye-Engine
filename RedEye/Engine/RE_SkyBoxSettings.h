#ifndef __RE_SKYBOX_SETTINGS_H__
#define __RE_SKYBOX_SETTINGS_H__

#include "RE_DataTypes.h"
#include "RE_TextureSettings.h"

struct SkyBoxTexture
{
	enum class Face : short
	{
		NOFACE = -1,
		RIGHT = 0,
		LEFT = 1,
		TOP = 2,
		BOTTOM = 3,
		FRONT = 4,
		BACK = 5
	};

	SkyBoxTexture(Face f) : face(f) {}

	Face face = Face::NOFACE;
	std::string path;
	const char* textureMD5 = nullptr;

	inline bool operator==(const SkyBoxTexture& b)
	{
		return (textureMD5 == b.textureMD5 && face == b.face);
	}
	inline bool operator!=(const SkyBoxTexture& b)
	{
		return (textureMD5 != b.textureMD5 || face != b.face);
	}
};

struct RE_SkyBoxSettings
{
	float skyBoxSize = 2000.0f;

	RE_TextureSettings::Filter min_filter = RE_TextureSettings::Filter::LINEAR;
	RE_TextureSettings::Filter mag_filter = RE_TextureSettings::Filter::LINEAR;

	RE_TextureSettings::Wrap wrap_s = RE_TextureSettings::Wrap::CLAMP_TO_EDGE;
	RE_TextureSettings::Wrap wrap_t = RE_TextureSettings::Wrap::CLAMP_TO_EDGE;
	RE_TextureSettings::Wrap wrap_r = RE_TextureSettings::Wrap::CLAMP_TO_EDGE;

	SkyBoxTexture textures[6] =
	{
		SkyBoxTexture(SkyBoxTexture::Face::RIGHT),
		SkyBoxTexture(SkyBoxTexture::Face::LEFT),
		SkyBoxTexture(SkyBoxTexture::Face::TOP),
		SkyBoxTexture(SkyBoxTexture::Face::BOTTOM),
		SkyBoxTexture(SkyBoxTexture::Face::FRONT),
		SkyBoxTexture(SkyBoxTexture::Face::BACK)
	};

	inline bool operator==(const RE_SkyBoxSettings& b)
	{
		return (min_filter == b.min_filter && mag_filter == b.mag_filter && wrap_s == b.wrap_s && wrap_t == b.wrap_t && wrap_r == b.wrap_r && skyBoxSize == skyBoxSize
			&& textures[0] == b.textures[0] && textures[1] == b.textures[1] && textures[2] == b.textures[2] && textures[3] == b.textures[3] && textures[4] == b.textures[4] && textures[5] == b.textures[5]);
	}

	inline bool operator!=(const RE_SkyBoxSettings& b)
	{
		return (min_filter != b.min_filter || mag_filter != b.mag_filter || wrap_s != b.wrap_s || wrap_t != b.wrap_t || wrap_r != b.wrap_r || skyBoxSize != skyBoxSize
			|| textures[0] != b.textures[0] || textures[1] != b.textures[1] || textures[2] != b.textures[2] || textures[3] != b.textures[3] || textures[4] != b.textures[4] || textures[5] != b.textures[5]);
	}

	inline bool texturesChanged(RE_SkyBoxSettings& b)
	{
		return (textures[0] == b.textures[0] && textures[1] == b.textures[1] && textures[2] == b.textures[2] && textures[3] == b.textures[3] && textures[4] == b.textures[4] && textures[5] == b.textures[5]);
	}
};

#endif // !__RE_SKYBOX_SETTINGS_H__