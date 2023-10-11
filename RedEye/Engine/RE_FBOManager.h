#ifndef __REFBOMANAGER__
#define __REFBOMANAGER__

struct RE_FBO
{
	enum class Type : char { DEFAULT, DEFERRED };
	Type type = Type::DEFAULT;

	eastl::vector<uint32_t> texturesID;
	uint32_t ID = 0;
	uint32_t width = 0;
	uint32_t height = 0;
	uint32_t depthBuffer = 0;
	uint32_t stencilBuffer = 0;
	uint32_t depthstencilBuffer = 0;
	uint32_t depthBufferTexture = 0;
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
