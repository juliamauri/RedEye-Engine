#ifndef __RENDER_VIEW_H__
#define __RENDER_VIEW_H__

#include "RE_DataTypes.h"
#include <MGL/MathGeoLib.h>
#include <EASTL/string.h>
#include <EASTL/array.h>

class RE_Json;

class RenderView
{
public:

	enum class LightMode : ushort
	{
		DISABLED = 0,
		GL,
		DIRECT,
		DEFERRED,
	};

	enum Flag
	{
		// GL Specs
		WIREFRAME = 1 << 0,			// 0000 0000 0000 0001
		FACE_CULLING = 1 << 1,		// 0000 0000 0000 0010
		TEXTURE_2D = 1 << 2,		// 0000 0000 0000 0100
		COLOR_MATERIAL = 1 << 3,	// 0000 0000 0000 1000
		DEPTH_TEST = 1 << 4,		// 0000 0000 0001 0000
		CLIP_DISTANCE = 1 << 5,		// 0000 0000 0010 0000
		GL_LIGHT = 1 << 6,			// 0000 0000 0100 0000

		// RE Utility
		FRUSTUM_CULLING = 1 << 7,	// 0000 0000 1000 0000
		OVERRIDE_CULLING = 1 << 8,	// 0000 0001 0000 0000
		OUTLINE_SELECTION = 1 << 9,	// 0000 0010 0000 0000
		DEBUG_DRAW = 1 << 10,		// 0000 0100 0000 0000
		SKYBOX = 1 << 11,			// 0000 1000 0000 0000
		BLENDED = 1 << 12,			// 0001 0000 0000 0000

		// Renderer
		VSYNC = 1 << 13,			// 0010 0000 0000 0000
		SHARE_LIGHT_PASS = 1 << 14	// 0100 0000 0000 0000
	};

public:

	ushort flags = 0;
	LightMode light_mode;
	eastl::pair<uint, uint> fbos;
	math::float4 clip_distance = math::float4::zero;
	math::float4 clear_color = { 0.0f, 0.0f, 0.0f, 1.0f };

public:

	RenderView(
		eastl::pair<uint, uint> fbos = { 0, 0 },
		ushort flags = 0,
		LightMode light = LightMode::GL,
		math::float4 clipDistance = math::float4::zero) :
		fbos(fbos), flags(flags), light_mode(light), clip_distance(clipDistance) {}

	~RenderView() = default;

	void DrawEditor(const char* id);

	uint GetFBO() const { return light_mode != LightMode::DEFERRED ? fbos.first : fbos.second; }

	// Flags
	inline void AddFlag(Flag flag) { flags |= static_cast<ushort>(flag); }
	inline void RemoveFlag(Flag flag) { flags -= static_cast<ushort>(flag); }
	inline const bool HasFlag(Flag flag) const { return flags & static_cast<ushort>(flag); }
	void CheckboxFlag(const char* label, Flag flag);

	// Serialization
	void Load(RE_Json* node);
	void Save(RE_Json* node) const;
};

#endif // !__RENDER_VIEW_H__