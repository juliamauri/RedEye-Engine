#include "Globals.h"

#include "OutputLog.h"

#include "Glew/include/glew.h"
#include <gl/GL.h>
#include <EASTL/string.h>

void _CheckGLError(const char* file, int line)
{
	GLenum err(glGetError());

	while (err != GL_NO_ERROR)
	{
		eastl::string error;
		switch (err)
		{
		case GL_INVALID_OPERATION:  error = "INVALID_OPERATION";      break;
		case GL_INVALID_ENUM:       error = "INVALID_ENUM";           break;
		case GL_INVALID_VALUE:      error = "INVALID_VALUE";          break;
		case GL_OUT_OF_MEMORY:      error = "OUT_OF_MEMORY";          break;
		case GL_INVALID_FRAMEBUFFER_OPERATION:  error = "INVALID_FRAMEBUFFER_OPERATION";  break;
		case GL_STACK_OVERFLOW:  error = "STACK_OVERFLOW";  break;
		default: error = "error not tracked"; break;
		}
		RE_LOG_ERROR("GL_%s - %s:%i", error.c_str(), file, line);
		err = glGetError();
	}

	return;
}