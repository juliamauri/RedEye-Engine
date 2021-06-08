#include "RE_ParticleRender.h"

unsigned int RE_ParticleRender::GetBinarySize() const
{
	unsigned int size = 0;// sizeof(RE_ParticleEmitter::ColorState);
	size += sizeof(float) * 3;
	size += sizeof(float) * 6;
	size += sizeof(bool) * 2;
	size += sizeof(bool) * 2;
	size += sizeof(float);
	size += sizeof(bool);
	size += sizeof(float) * 3;
	size += sizeof(bool);
	size += sizeof(float) * 3;
	size += sizeof(bool) * 2;
	size += sizeof(float) * 3;
	size += sizeof(bool);
	size += sizeof(float) * 3;


	const char* meshMD5 = nullptr;

	RE_CompPrimitive* primCmp = nullptr;

	math::float3 scale = { 0.5f,0.5f,0.1f };

	RE_ParticleEmitter::Particle_Dir particleDir = RE_ParticleEmitter::Particle_Dir::PS_Billboard;
	math::float3 direction = { -1.0f,1.0f,0.5f };

	//Curves
	bool useCurve = false;
	bool smoothCurve = false;
	eastl::vector< ImVec2> curve;
	int total_points = 10;
	return 0;
}
