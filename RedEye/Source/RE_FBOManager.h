#ifndef __REFBOMANAGER__
#define __REFBOMANAGER__

#include "EventListener.h"

#include <EASTL/map.h>
#include <EASTL/vector.h>

struct RE_FBO {
	unsigned int ID = 0;
	unsigned int width = 0, height = 0;
	eastl::vector<unsigned int> texturesID;
	unsigned int depthBuffer = 0;
	unsigned int stencilBuffer = 0;
	unsigned int depthstencilBuffer = 0;
	unsigned int depthBufferTexture = 0;
	enum FBO_Type : char
	{
		DEFAULT = 0,
		DEFERRED
	} type = DEFAULT;
};

class RE_FBOManager :
	public EventListener
{
public:
	RE_FBOManager();
	~RE_FBOManager();

	static int CreateFBO(unsigned int width, unsigned int height, unsigned int texturesSize = 1, bool depth = true, bool stencil = false);
	static int CreateDeferredFBO(unsigned int width, unsigned int height);

	static void ChangeFBOSize(unsigned int ID, unsigned int width, unsigned int height);

	static void ClearFBO(unsigned int ID);

	static unsigned int GetDepthTexture(unsigned int ID);
	static unsigned int GetTextureID(unsigned int ID, unsigned int texAttachment);
	static unsigned int GetWidth(unsigned int ID);
	static unsigned int GetHeight(unsigned int ID);

	static void ChangeFBOBind(unsigned int tID, unsigned int width = 0, unsigned int height = 0);
	static void ClearFBOBuffers(unsigned int ID, const float color[4]);

private:

	static void LoadDeferredTextures(RE_FBO &fbo);

private:

	static eastl::map<unsigned int, RE_FBO> fbos;
};

#endif // !__REFBOMANAGER__
