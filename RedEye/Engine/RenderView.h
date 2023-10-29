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
	eastl::pair<uint, uint> fbos;
	math::float4 clip_distance = math::float4::zero;
	math::float4 clear_color = { 0.0f, 0.0f, 0.0f, 1.0f };

public:

	RenderView(
		eastl::pair<uint, uint> fbos = { 0, 0 },
		RenderSettings settings = RenderSettings(),
		math::float4 clipDistance = math::float4::zero) :
		fbos(fbos), settings(settings), clip_distance(clipDistance) {}

	~RenderView() = default;

	void DrawEditor(const char* id);

	uint GetFBO() const { return settings.light != RenderSettings::LightMode::DEFERRED ? fbos.first : fbos.second; }

	// Serialization
	void Load(RE_Json* node);
	void Save(RE_Json* node) const;
};

#endif // !__RENDER_VIEW_H__