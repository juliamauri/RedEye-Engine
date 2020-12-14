#include "Globals.h"

#include "Application.h"
#include <gl/GL.h>
#include <EASTL/string.h>

void GLAPIENTRY MessageCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam)
{
	if (severity == GL_DEBUG_SEVERITY_NOTIFICATION || type == GL_DEBUG_TYPE_OTHER) return;

	eastl::string severityStr = "Severiry ";
	switch (severity) {
	case GL_DEBUG_SEVERITY_LOW: severityStr += "low"; break;
	case GL_DEBUG_SEVERITY_MEDIUM: severityStr += "medium"; break;
	case GL_DEBUG_SEVERITY_HIGH: severityStr += "high"; break;
	default: severityStr += "not specified."; break; }

	eastl::string typeStr = "Type ";
	switch (type) {
	case GL_DEBUG_TYPE_ERROR: typeStr += "GL ERROR"; break;
	case GL_DEBUG_TYPE_PORTABILITY: typeStr += "GL PORTABILITY"; break;
	case GL_DEBUG_TYPE_PERFORMANCE: typeStr += "GL PERFORMANCE"; break;
	case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: typeStr += "GL DEPRECATED"; break;
	default: typeStr += "GL UNDEFINED"; break;
	}

	if (type == GL_DEBUG_TYPE_ERROR) RE_LOG_ERROR("%s, %s, message = %s\n",
		(typeStr.c_str()), severityStr.c_str(), message);
	else RE_LOG_WARNING("%s, %s, message = %s\n",
		(typeStr.c_str()), severityStr.c_str(), message);
}
