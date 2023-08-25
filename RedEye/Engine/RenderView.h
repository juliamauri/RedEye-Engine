#ifndef __RENDER_VIEW_H__
#define __RENDER_VIEW_H__

#include <EASTL/string.h>
#include <MGL/MathGeoLib.h>

struct RenderView
{
	enum class Type : short
	{
		EDITOR,
		GAME,
		PARTICLE,
		OTHER
	};

	enum class LightMode : int
	{
		DISABLED = 0,
		GL,
		DIRECT,
		DEFERRED,
	};

	RenderView(eastl::string name = "",
		eastl::pair<unsigned int, unsigned int> fbos = { 0, 0 },
		short flags = 0, LightMode light = LightMode::GL, math::float4 clipDistance = math::float4::zero);

	LightMode light;
	eastl::string name;
	math::float4 clear_color;
	math::float4 clip_distance;
	class RE_CompCamera* camera = nullptr;

	eastl::pair<unsigned int, unsigned int> fbos;
	const unsigned int GetFBO() const;

	enum class Flag : short
	{
		FRUSTUM_CULLING = 0x1,	 // 0000 0000 0001
		OVERRIDE_CULLING = 0x2,	 // 0000 0000 0010
		OUTLINE_SELECTION = 0x4, // 0000 0000 0100
		DEBUG_DRAW = 0x8,		 // 0000 0000 1000

		SKYBOX = 0x10,			 // 0000 0001 0000
		BLENDED = 0x20,			 // 0000 0010 0000
		WIREFRAME = 0x40,		 // 0000 0100 0000
		FACE_CULLING = 0X80,	 // 0000 1000 0000

		TEXTURE_2D = 0x100,		 // 0001 0000 0000
		COLOR_MATERIAL = 0x200,	 // 0010 0000 0000
		DEPTH_TEST = 0x400,		 // 0100 0000 0000
		CLIP_DISTANCE = 0x800	 // 1000 0000 0000
	};

	short flags;
	static const char* flag_labels[12];

	inline void AddFlag(Flag flag) { flags |= static_cast<const short>(flag); }
	inline void RemoveFlag(Flag flag) { flags -= static_cast<const short>(flag); }
	inline const bool HasFlag(Flag flag) const { return flags & static_cast<const short>(flag); }
};

#endif // !__RENDER_VIEW_H__