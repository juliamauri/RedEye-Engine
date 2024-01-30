#ifndef __RE_GLCACHE_H__
#define __RE_GLCACHE_H__

#include "RE_DataTypes.h"

namespace RE_GLCache
{
	namespace
	{
		uint currentShaderID = 0u;
		uint currenVAO = 0u;
		uint currenTexID = 0u;
	}

	void ChangeShader(uint ID);
	void ChangeVAO(uint VAO);
	void ChangeTextureBind(uint tID);
};

#endif // !__RE_GLCACHE_H__
