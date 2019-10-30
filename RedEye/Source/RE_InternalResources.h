#ifndef __INTERNALRESOURCCES_H__
#define __INTERNALRESOURCCES_H__

class RE_InternalResources
{
public:
	RE_InternalResources();
	~RE_InternalResources();

	bool Init();

	unsigned int GetDefaultShader() const;
	unsigned int GetPrimitiveShader() const;
	
	unsigned int GetTextureChecker() const;

private:
	bool InitShaders();
	bool InitChecker();

private:
	unsigned int defaultShader = 0;
	unsigned int primitiveShader = 0;

	unsigned int checkerTexture = 0;
};

#endif // !__INTERNALRESOURCCES_H__

