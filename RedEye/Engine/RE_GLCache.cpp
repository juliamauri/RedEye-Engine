#include "RE_GLCache.h"

#include "RE_ShaderImporter.h"
#include <GL/glew.h>

void RE_GLCache::ChangeShader(uint ID)
{
	if (currentShaderID != ID)
		RE_ShaderImporter::use((currentShaderID = ID));
}

void RE_GLCache::ChangeVAO(uint VAO)
{
	if (currenVAO != VAO)
		glBindVertexArray((currenVAO = VAO));
}

void RE_GLCache::ChangeTextureBind(uint tID)
{
	if (currenTexID != tID)
		glBindTexture(GL_TEXTURE_2D, static_cast<GLuint>(currenTexID = tID));
}
