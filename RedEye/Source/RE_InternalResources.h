#ifndef __INTERNAL_RESOURCCES_H__
#define __INTERNAL_RESOURCCES_H__

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

	unsigned int GetTextureChecker();
	unsigned int GetTextureWaterFoam();
	eastl::vector<RE_Shader_Cvar> GetWaterUniforms();

	namespace Internal
	{
		void InitChecker();
		bool InitShaders();
		bool InitMaterial();
		bool InitSkyBox();

		void InitWaterResources();

		static unsigned int checkerTexture = 0u;
		static unsigned int water_foam_texture = 0;

		static const char* defaultMaterial = nullptr;
		static const char* defaultSkybox = nullptr;

		// Default shaders
		static const char* defaultShader = nullptr;
		static const char* defaultScaleShader = nullptr;
		static const char* skyboxShader = nullptr;

		// Deferred shaders
		static const char* defGeoShader = nullptr;
		static const char* defLightShader = nullptr;

		// Water
		static eastl::vector<RE_Shader_Cvar> waterUniforms;
		static const char* waterShader = nullptr;
		static const char* waterDefShader = nullptr;

		static const char* WATER_FOAM_TEX_PATH = "Settings/DefaultAssets/water_foam.png";
	}
}

#endif // !__INTERNAL_RESOURCCES_H__

