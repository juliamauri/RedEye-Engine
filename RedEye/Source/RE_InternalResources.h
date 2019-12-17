#ifndef __INTERNALRESOURCCES_H__
#define __INTERNALRESOURCCES_H__

class RE_SkyBox;
class RE_Shader;

class RE_InternalResources
{
public:
	RE_InternalResources();
	~RE_InternalResources();

	bool Init();
	
	const char* GetDefaultShader()const;
	const char* GetDefaultSkyBoxShader()const;
	const char* GetDefaultScaleShader()const;
	const char* GetDefaulMaterial()const;
	const char* GetDefaultSkyBox()const;

	unsigned int GetTextureChecker() const;

private:
	bool InitShaders();
	bool InitMaterial();
	bool InitChecker();
	bool InitSkyBox();

private:
	const char* defaultShader = nullptr;
	const char* defaultScaleShader = nullptr;
	const char* skyboxShader = nullptr;
	const char* defaultMaterial = nullptr;
	const char* defaultSkybox = nullptr;

	unsigned int checkerTexture = 0;
};

#endif // !__INTERNALRESOURCCES_H__

