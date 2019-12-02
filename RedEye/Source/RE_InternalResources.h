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
	const char* GetDefaulMaterial()const;
	unsigned int GetSkyBoxShader() const;

	unsigned int GetTextureChecker() const;

	unsigned int GetSkyBoxVAO() const;
	unsigned int GetSkyBoxTexturesID() const;

	void FindDefaultSkyBox();
private:
	bool InitShaders();
	bool InitMaterial();
	bool InitChecker();
	bool InitSkyBox();

private:
	const char* defaultShader = nullptr;
	const char* skyboxShader = nullptr;
	const char* defaultMaterial = nullptr;

	unsigned int checkerTexture = 0;

	const char* skyboxMD5 = nullptr;
};

#endif // !__INTERNALRESOURCCES_H__

