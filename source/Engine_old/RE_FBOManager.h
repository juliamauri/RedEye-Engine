#ifndef __REFBOMANAGER__
#define __REFBOMANAGER__

struct RE_FBO
{
	enum class FBO_Type : char { DEFAULT, DEFERRED } type = FBO_Type::DEFAULT;
	eastl::vector<uint32_t> texturesID;
	uint32_t
		ID = 0, width = 0, height = 0,
		depthBuffer = 0, stencilBuffer = 0,
		depthstencilBuffer = 0, depthBufferTexture = 0;
};

class RE_FBOManager
{
public:
	RE_FBOManager() {}
	~RE_FBOManager() {}

	int CreateFBO(unsigned int width, unsigned int height, unsigned int texturesSize = 1, bool depth = true, bool stencil = false);
	int CreateDeferredFBO(unsigned int width, unsigned int height);

	unsigned int GetDepthTexture(unsigned int ID);
	uint32_t  GetTextureID(unsigned int ID, unsigned int texAttachment);
	unsigned int GetWidth(unsigned int ID);
	unsigned int GetHeight(unsigned int ID);

	void ChangeFBOSize(unsigned int ID, unsigned int width, unsigned int height);
	void ChangeFBOBind(unsigned int tID, unsigned int width = 0, unsigned int height = 0);

	void ClearFBOBuffers(unsigned int ID, const float color[4]);
	void ClearFBO(unsigned int ID);
	void ClearAll();

private:

	void LoadDeferredTextures(RE_FBO& fbo);

private:

	eastl::map<unsigned int, RE_FBO> fbos;
};

#endif // !__REFBOMANAGER__
