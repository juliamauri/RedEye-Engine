#pragma once

#include "RE_DataTypes.h"

class RE_Json;

struct RenderSettings
{
	enum Flag : ushort
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
	};

	enum class LightMode : ushort
	{
		DISABLED = 0,
		GL,
		DIRECT,
		DEFERRED,
	};

	ushort flags = 0;
	LightMode light;

	RenderSettings(
		ushort flags = 0,
		LightMode light = LightMode::GL) :
		flags(flags), light(light) {}
	~RenderSettings() = default;

	void DrawEditor(const char* id = "none");

	// Flags
	inline void AddFlag(Flag flag) { flags |= flag; }
	inline void RemoveFlag(Flag flag) { flags -= flag; }
	inline const bool HasFlag(Flag flag) const { return flags & flag; }
	void CheckboxFlag(const char* label, Flag flag);

	// Serialization
	void Load(RE_Json* node);
	void Save(RE_Json* node) const;
};