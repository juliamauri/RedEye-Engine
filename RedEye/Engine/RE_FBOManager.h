#ifndef __REFBOMANAGER__
#define __REFBOMANAGER__

#include "RE_DataTypes.h"

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

	void LoadDeferredTextures();
};

namespace RE_FBOManager
{
	int CreateFBO(
		uint width,
		uint height,
		uint texturesSize = 1,
		bool depth = true,
		bool stencil = false);
	
	int CreateDeferredFBO(uint width, uint height);

	uint GetDepthTexture(uint ID);
	uint32_t GetTextureID(uint ID, uint texAttachment);
	uint GetWidth(uint ID);
	uint GetHeight(uint ID);
	void GetWidthAndHeight(uint ID, uint32_t& width, uint32_t& height);

	void ChangeFBOSize(uint ID, uint width, uint height);
	void ChangeFBOBind(uint tID, uint width = 0, uint height = 0);

	void ClearFBOBuffers(uint ID, const float color[4]);
	void ClearFBO(uint ID);
	void ClearAll();
};

#endif // !__REFBOMANAGER__
