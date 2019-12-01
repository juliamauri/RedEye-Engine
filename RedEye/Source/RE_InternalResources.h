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

	unsigned int GetDefaultShader() const;
	unsigned int GetSkyBoxShader() const;
	
	unsigned int GetTextureChecker() const;

	unsigned int GetSkyBoxVAO() const;
	unsigned int GetSkyBoxTexturesID() const;

	void FindDefaultSkyBox();
private:
	bool InitShaders();
	bool InitChecker();
	bool InitSkyBox();

private:
	RE_Shader* defaultShader = nullptr;
	RE_Shader* skyboxShader = nullptr;

	unsigned int checkerTexture = 0;

	const char* skyboxMD5 = nullptr;
};

#endif // !__INTERNALRESOURCCES_H__

