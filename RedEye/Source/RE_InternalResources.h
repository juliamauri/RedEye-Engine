#ifndef __INTERNALRESOURCCES_H__
#define __INTERNALRESOURCCES_H__

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

	unsigned int skyBoxVAO = 0;
	unsigned int skyBoxVBO = 0;
	unsigned int skyBoxTexturesID = 0;
};

#endif // !__INTERNALRESOURCCES_H__

