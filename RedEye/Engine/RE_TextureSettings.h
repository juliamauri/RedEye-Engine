#ifndef __RE_TEXTURE_SETTINGS_H__
#define __RE_TEXTURE_SETTINGS_H__

#include "RE_DataTypes.h"
#include <MGL/Math/float4.h>

struct RE_TextureSettings
{
	enum class Type : uint //Same as image types defines on  il.h DevIL
	{
		TEXTURE_UNKNOWN = 0x0000,
		BMP = 0x0420,
		JPG = 0x0425,
		PNG = 0x042A,
		TGA = 0x042D,
		TIFF = 0x042E,
		DDS = 0x0437
	};

	enum Filter : int //Same as GL values
	{
		NEAREST = 0x2600,
		LINEAR = 0x2601,
		NEAREST_MIPMAP_NEAREST = 0x2700,
		LINEAR_MIPMAP_NEAREST = 0x2701,
		NEAREST_MIPMAP_LINEAR = 0x2702,
		LINEAR_MIPMAP_LINEAR = 0x2703
	};

	Filter min_filter = NEAREST;
	Filter mag_filter = NEAREST;

	enum Wrap : int //Same as GL values
	{
		REPEAT = 0x2901,
		CLAMP_TO_BORDER = 0x812D,
		CLAMP_TO_EDGE = 0x812F,
		MIRRORED_REPEAT = 0x8370
	};

	Wrap wrap_s = REPEAT;
	Wrap wrap_t = REPEAT;

	math::float4 borderColor = { 0.0f, 0.0f, 0.0f, 0.0f };

	inline bool operator==(const RE_TextureSettings& b)
	{
		return
			min_filter == b.min_filter && mag_filter == b.mag_filter && // filter
			wrap_s == b.wrap_s && wrap_t == b.wrap_t && // wrap
			borderColor.Equals(b.borderColor); // color
	}

	inline bool operator!=(const RE_TextureSettings& b)
	{
		return
			min_filter != b.min_filter || mag_filter != b.mag_filter || // filter
			wrap_s != b.wrap_s || wrap_t != b.wrap_t || // wrap
			!borderColor.Equals(b.borderColor); // color
	}
};

#endif // !__RE_TEXTURE_SETTINGS_H__