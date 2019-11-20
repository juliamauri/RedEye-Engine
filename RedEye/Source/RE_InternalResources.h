#ifndef __INTERNALRESOURCCES_H__
#define __INTERNALRESOURCCES_H__

class RE_SkyBox;

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

private:
	bool InitShaders();
	bool InitChecker();
	bool InitSkyBox();

private:
	unsigned int defaultShader = 0;
	unsigned int skyboxShader = 0;

	unsigned int checkerTexture = 0;

	RE_SkyBox* defaultSkybox = nullptr;
};

#endif // !__INTERNALRESOURCCES_H__

