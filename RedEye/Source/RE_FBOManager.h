#ifndef __REFBOMANAGER__
#define __REFBOMANAGER__

#include <EASTL/vector.h>
#include <EASTL/map.h>

struct RE_FBO
{
	enum FBO_Type : char { DEFAULT, DEFERRED } type = DEFAULT;
	eastl::vector<unsigned int> texturesID;
	unsigned int
		ID = 0, width = 0, height = 0,
		depthBuffer = 0, stencilBuffer = 0,
		depthstencilBuffer = 0, depthBufferTexture = 0;
};

namespace RE_FBOManager
{
	int CreateFBO(unsigned int width, unsigned int height, unsigned int texturesSize = 1, bool depth = true, bool stencil = false);
	int CreateDeferredFBO(unsigned int width, unsigned int height);

	unsigned int GetDepthTexture(unsigned int ID);
	unsigned int GetTextureID(unsigned int ID, unsigned int texAttachment);
	unsigned int GetWidth(unsigned int ID);
	unsigned int GetHeight(unsigned int ID);

	void ChangeFBOSize(unsigned int ID, unsigned int width, unsigned int height);
	void ChangeFBOBind(unsigned int tID, unsigned int width = 0, unsigned int height = 0);

	void ClearFBOBuffers(unsigned int ID, const float color[4]);
	void ClearFBO(unsigned int ID);
	void ClearAll();

	namespace Internal
	{
		static eastl::map<unsigned int, RE_FBO> fbos;
		void LoadDeferredTextures(RE_FBO& fbo);
	}
};

#endif // !__REFBOMANAGER__
