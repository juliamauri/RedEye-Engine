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

	LightMode light;
	eastl::string name;
	math::float4 clear_color;
	math::float4 clip_distance;
	class RE_CompCamera* camera = nullptr;

	eastl::pair<uint, uint> fbos;
	const uint GetFBO() const;

	enum class Flag : ushort
	{
		FRUSTUM_CULLING = 0x1,	 // 0000 0000 0000 0001
		OVERRIDE_CULLING = 0x2,	 // 0000 0000 0000 0010
		OUTLINE_SELECTION = 0x4, // 0000 0000 0000 0100
		DEBUG_DRAW = 0x8,		 // 0000 0000 0000 1000

		SKYBOX = 0x10,			 // 0000 0000 0001 0000
		BLENDED = 0x20,			 // 0000 0000 0010 0000
		WIREFRAME = 0x40,		 // 0000 0000 0100 0000
		FACE_CULLING = 0X80,	 // 0000 0000 1000 0000

		TEXTURE_2D = 0x100,		 // 0000 0001 0000 0000
		COLOR_MATERIAL = 0x200,	 // 0000 0010 0000 0000
		DEPTH_TEST = 0x400,		 // 0000 0100 0000 0000
		CLIP_DISTANCE = 0x800	 // 0000 1000 0000 0000
	};

	ushort flags;
	static const char* flag_labels[12];

	inline void AddFlag(Flag flag) { flags |= static_cast<const ushort>(flag); }
	inline void RemoveFlag(Flag flag) { flags -= static_cast<const ushort>(flag); }
	inline const bool HasFlag(Flag flag) const;

	void DrawEditor();
	void Save(RE_Json* node) const;
	void Load(RE_Json* node);
};

#endif // !__RENDER_VIEW_H__