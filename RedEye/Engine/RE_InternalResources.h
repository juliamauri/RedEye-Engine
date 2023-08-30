#ifndef __INTERNALRESOURCCES_H__
#define __INTERNALRESOURCCES_H__

#include "RE_Cvar.h"
#include <EASTL/vector.h>

namespace RE_InternalResources
{
	void Init();
	void Clear();
	
	const char* GetDefaultShader();
	const char* GetDefaultWaterShader();
	const char* GetDefaultSkyBoxShader();
	const char* GetDefaultScaleShader();
	const char* GetDefaulMaterial();
	const char* GetDefaultSkyBox();
	const char* GetLightPassShader();
	const char* GetParticleLightPassShader();
	const char* GetParticleShader();

	unsigned int GetTextureChecker();
	unsigned int GetTextureWaterFoam();
	eastl::vector<RE_Shader_Cvar> GetWaterUniforms();
};

#endif // !__INTERNALRESOURCCES_H__

