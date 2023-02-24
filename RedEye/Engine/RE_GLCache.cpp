#include "RE_GLCache.h"

#include "RE_ShaderImporter.h"
#include <GL/glew.h>

void RE_GLCache::ChangeShader(unsigned int ID)
{
	static unsigned int currentShaderID = 0u;
	if (currentShaderID != ID) RE_ShaderImporter::use(currentShaderID = ID);
}

void RE_GLCache::ChangeVAO(unsigned int VAO)
{
	static unsigned int currenVAO = 0u;
	if (currenVAO != VAO) glBindVertexArray(currenVAO = VAO);
}

void RE_GLCache::ChangeTextureBind(unsigned int tID)
{
	static unsigned int currenTexID = 0u;
	if (currenTexID != tID) glBindTexture(GL_TEXTURE_2D, currenTexID = tID);
}
