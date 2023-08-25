
#include "RenderView.h"

RenderView::RenderView(
	eastl::string name,
	eastl::pair<unsigned int, unsigned int> fbos,
	short flags,
	LightMode light,
	math::float4 clipDistance) :

	name(name), fbos(fbos), flags(flags), light(light), clip_distance(clipDistance),
	clear_color({ 0.0f, 0.0f, 0.0f, 1.0f })
{}

const unsigned int RenderView::GetFBO() const
{
	return light != LightMode::DEFERRED ? fbos.first : fbos.second;
}