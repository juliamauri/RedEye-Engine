#ifndef __RENDER_VIEW_H__
#define __RENDER_VIEW_H__

#include "RenderSettings.h"
#include <MGL/MathGeoLib.h>
#include <EASTL/array.h>

class RE_Json;

class RenderView
{
public:

	RenderSettings settings;
	uint fbos[2];
	math::float4 clip_distance = math::float4::zero;
	math::float4 clear_color = { 0.0f, 0.0f, 0.0f, 1.0f };

public:

	RenderView(
		uint default_fbo = 0U,
		uint deferred_fbo = 0U,
		RenderSettings settings = RenderSettings(),
		math::float4 clipDistance = math::float4::zero);

	~RenderView() = default;

	void DrawEditor(const char* id);

	uint GetFBO() const;

	// Serialization
	void Load(RE_Json* node);
	void Save(RE_Json* node) const;
};

#endif // !__RENDER_VIEW_H__