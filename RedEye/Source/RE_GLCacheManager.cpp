#include "RE_GLCacheManager.h"

#include "RE_ShaderImporter.h"

#include "Glew/include/glew.h"

void RE_GLCacheManager::ChangeShader(unsigned int ID)
{
	static unsigned int currentShaderID = 0;
	if (currentShaderID != ID) RE_ShaderImporter::use(currentShaderID = ID);
}

void RE_GLCacheManager::ChangeVAO(unsigned int VAO)
{
	static unsigned int currenVAO = 0;
	if (currenVAO != VAO) glBindVertexArray(currenVAO = VAO);
}

void RE_GLCacheManager::ChangeTextureBind(unsigned int tID)
{
	static unsigned int currenTexID = 0;
	if (currenTexID != tID) glBindTexture(GL_TEXTURE_2D, currenTexID = tID);
}