#ifndef __RE_TEXTURE_SETTINGS_H__
#define __RE_TEXTURE_SETTINGS_H__

#include <MGL/Math/float4.h>

enum TextureType { //Same as image types defines on  il.h DevIL
	RE_TEXTURE_UNKNOWN = 0x0000,
	RE_BMP = 0x0420,
	RE_JPG = 0x0425,
	RE_PNG = 0x042A,
	RE_TGA = 0x042D,
	RE_TIFF = 0x042E,
	RE_DDS = 0x0437
};

enum RE_TextureFilters { //Same as GL values
	RE_NEAREST = 0x2600,
	RE_LINEAR = 0x2601,
	RE_NEAREST_MIPMAP_NEAREST = 0x2700,
	RE_LINEAR_MIPMAP_NEAREST = 0x2701,
	RE_NEAREST_MIPMAP_LINEAR = 0x2702,
	RE_LINEAR_MIPMAP_LINEAR = 0x2703
};

enum RE_TextureWrap { //Same as GL values
	RE_REPEAT = 0x2901,
	RE_CLAMP_TO_BORDER = 0x812D,
	RE_CLAMP_TO_EDGE = 0x812F,
	RE_MIRRORED_REPEAT = 0x8370
};

struct RE_TextureSettings
{
	RE_TextureFilters min_filter = RE_NEAREST, mag_filter = RE_NEAREST;
	RE_TextureWrap wrap_s = RE_REPEAT, wrap_t = RE_REPEAT;
	math::float4 borderColor = { 0.0f, 0.0f, 0.0f, 0.0f };

	inline bool operator==(const RE_TextureSettings& b) {
		return (
			min_filter == b.min_filter && mag_filter == b.mag_filter && // filter
			wrap_s == b.wrap_s && wrap_t == b.wrap_t && // wrap
			borderColor.Equals(b.borderColor));
	} // color

	inline bool operator!=(const RE_TextureSettings& b) {
		return (
			min_filter != b.min_filter || mag_filter != b.mag_filter || // filter
			wrap_s != b.wrap_s || wrap_t != b.wrap_t || // wrap
			!borderColor.Equals(b.borderColor));
	} // color
};

#endif // !__RE_TEXTURE_SETTINGS_H__