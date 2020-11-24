#ifndef __INTERNALRESOURCCES_H__
#define __INTERNALRESOURCCES_H__

#include "Cvar.h"

#include <EASTL/vector.h>
class RE_SkyBox;
class RE_Shader;

class RE_InternalResources
{
public:

	RE_InternalResources();
	~RE_InternalResources();

	void Init();
	
	const char* GetDefaultShader()const;
	const char* GetDefaultWaterShader()const;
	const char* GetDefaultSkyBoxShader()const;
	const char* GetDefaultScaleShader()const;
	const char* GetDefaulMaterial()const;
	const char* GetDefaultSkyBox()const;
	const char* GetLightPassShader()const;

	unsigned int GetTextureChecker() const;
	unsigned int GetTextureWaterFoam() const;
	eastl::vector<ShaderCvar> GetWaterUniforms() const;

private:

	void InitChecker();
	bool InitShaders();
	bool InitMaterial();
	bool InitSkyBox();

	void InitWaterResources();

private:

	const char* defaultShader = nullptr;
	const char* defaultScaleShader = nullptr;
	const char* skyboxShader = nullptr;

	const char* defGeoShader = nullptr;
	const char* defLightShader = nullptr;


	const char* waterShader = nullptr;
	const char* waterDefShader = nullptr;
	eastl::vector<ShaderCvar> waterUniforms;
	unsigned int water_foam_texture = 0;

	const char* defaultMaterial = nullptr;
	const char* defaultSkybox = nullptr;

	unsigned int checkerTexture = 0u; 
};

#endif // !__INTERNALRESOURCCES_H__

