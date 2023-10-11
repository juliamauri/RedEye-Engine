#ifndef __RENDER_VIEW_H__
#define __RENDER_VIEW_H__

#include "RE_DataTypes.h"
#include <MGL/MathGeoLib.h>
#include <EASTL/string.h>
#include <EASTL/array.h>

class RE_Json;

struct RenderView
{
	enum class Type : ushort
	{
		EDITOR,
		GAME,
		PARTICLE,
		OTHER
	};

	enum class LightMode : ushort
	{
		DISABLED = 0,
		GL,
		DIRECT,
		DEFERRED,
	};

	RenderView(
		eastl::string name = "",
		eastl::pair<uint, uint> fbos = { 0, 0 },
		short flags = 0,
		LightMode light = LightMode::GL,
		math::float4 clipDistance = math::float4::zero);

	~RenderView();

	// Properties
	LightMode light;
	eastl::string name;
	math::float4 clear_color;
	math::float4 clip_distance;

	// Camera
	class RE_Camera* camera = nullptr;
	const math::Frustum* GetFrustum() const;

	// FBO
	eastl::pair<uint, uint> fbos;
	const uint GetFBO() const;

	// Flags
	enum class Flag : ushort
	{
		EMPTY = 0,

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

	ushort flags = 0;
	static const char* flag_labels[12];

	inline void AddFlag(Flag flag) { flags |= static_cast<const ushort>(flag); }
	inline void RemoveFlag(Flag flag) { flags -= static_cast<const ushort>(flag); }
	inline const bool HasFlag(Flag flag) const;

	// Draw
	void DrawEditor();

	// Serialization
	void Save(RE_Json* node) const;
	void Load(RE_Json* node);
};

#endif // !__RENDER_VIEW_H__