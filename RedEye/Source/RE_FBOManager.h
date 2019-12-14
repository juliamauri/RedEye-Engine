#ifndef __REFBOMANAGER__
#define __REFBOMANAGER__

#include "EventListener.h"

#include <map>

struct RE_FBO {
	unsigned int ID = 0;
	unsigned int widht = 0, height = 0;
	std::vector<unsigned int> texturesID;
	unsigned int depthBuffer = 0;
	unsigned int stencilBuffer = 0;
	unsigned int depthstencilBuffer = 0;
};

class RE_FBOManager :
	public EventListener
{
public:
	RE_FBOManager();
	~RE_FBOManager();

	int CreateFBO(unsigned int width, unsigned int height, unsigned int texturesSize = 1, bool depth = true, bool stencil = false);

	void ChangeFBOSize(unsigned int ID, unsigned int width, unsigned int height);

	void ClearFBO(unsigned int ID);

	unsigned int GetTextureID(unsigned int ID, unsigned int texAttachment)const;
	unsigned int GetWidth(unsigned int ID)const;
	unsigned int GetHeight(unsigned int ID)const;

	static void ChangeFBOBind(unsigned int tID, unsigned int width = 0, unsigned int height = 0);

private:
	std::map<unsigned int, RE_FBO> fbos;
};

#endif // !__REFBOMANAGER__
