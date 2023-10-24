#include "RE_ShaderImporter.h"

#include "RE_Memory.h"
#include "Application.h"
#include "RE_Profiler.h"
#include "RE_FileBuffer.h"
#include "ModuleEditor.h"
#include "RE_ResourceManager.h"

#include <SDL2/SDL.h>
#include <GL/glew.h>
#include <EASTL/string.h>

eastl::string shader_last_error;
int* binaryFormats = nullptr;

void RE_ShaderImporter::Init()
{
	RE_PROFILE(RE_ProfiledFunc::Init, RE_ProfiledClass::ShaderImporter)

	bool ret = true;
	RE_LOG("Initializing Shader Manager");
	RE_SOFT_NVS("GLSLang", reinterpret_cast<const char*>(glGetString(GL_SHADING_LANGUAGE_VERSION)), "https://www.opengl.org/sdk/docs/tutorials/ClockworkCoders/glsl_overview.php");
	
	GLint formats = 0;
	glGetIntegerv(GL_NUM_PROGRAM_BINARY_FORMATS, &formats);
	binaryFormats = new int[formats];
	glGetIntegerv(GL_PROGRAM_BINARY_FORMATS, binaryFormats);
}

void RE_ShaderImporter::Clear()
{
	RE_PROFILE(RE_ProfiledFunc::Clear, RE_ProfiledClass::ShaderImporter)
	DEL_A(binaryFormats);
}

bool RE_ShaderImporter::LoadFromAssets(unsigned int* ID, const char* vertexPath, const char* fragmentPath, const char* geometryPath, bool compileTest)
{
	RE_LOG("%s %s\n%s\n%s shaders.",(!compileTest) ? "Loading" : "Compile Test", vertexPath, fragmentPath, (geometryPath) ? geometryPath : "No geometry shader");

	bool ret = true;
	eastl::string last_error;

	const char* buffer = nullptr;
	int  success;
	char infoLog[512];

	unsigned int vertexShader = 0, fragmentShader = 0, geometryShader = 0;

	if (vertexPath)
	{
		//compiling vertex shader
		vertexShader = glCreateShader(GL_VERTEX_SHADER);

		//load source
		RE_FileBuffer file_vertexShader(vertexPath);
		if (file_vertexShader.Load())
		{
			buffer = file_vertexShader.GetBuffer();
			auto vSize = static_cast<GLint>(file_vertexShader.GetSize());
			glShaderSource(vertexShader, 1, &buffer, &vSize);
			glCompileShader(vertexShader);

			//check
			glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
			if (!success)
			{
				glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
				last_error += "\nVertex compilation failed from ";
				last_error += vertexPath;
				last_error += ":\n";
				last_error += eastl::string(infoLog);
				last_error += "\n";
				ret = false;
			}
		}
		else
		{
			last_error += "\nError Loading vertex shader from ";
			last_error += vertexPath;
			last_error += "\n";
			ret = false;
		}
	}
	
	if (fragmentPath)
	{
		//compiling fragment shader
		fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

		RE_FileBuffer file_fragmentShader(fragmentPath);
		if (file_fragmentShader.Load())
		{
			buffer = file_fragmentShader.GetBuffer();
			auto fSize = static_cast<GLint>(file_fragmentShader.GetSize());
			glShaderSource(fragmentShader, 1, &buffer, &fSize);
			glCompileShader(fragmentShader);

			//check
			glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
			if (!success)
			{
				glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
				last_error += "\nFragment compilation failed from ";
				last_error += fragmentPath;
				last_error += ":\n";
				last_error += eastl::string(infoLog);
				last_error += "\n";
				ret = false;
			}
		}
		else
		{
			last_error += "\nError Loading fragment shader from ";
			last_error += fragmentPath;
			last_error += "\n";
			ret = false;
		}
	}

	if (geometryPath)
	{
		//compiling geometry shader
		geometryShader = glCreateShader(GL_GEOMETRY_SHADER);

		RE_FileBuffer file_geometryShader(geometryPath);
		if (file_geometryShader.Load())
		{
			buffer = file_geometryShader.GetBuffer();
			auto gSize = static_cast<GLint>(file_geometryShader.GetSize());
			glShaderSource(geometryShader, 1, &buffer, &gSize);
			glCompileShader(geometryShader);

			//check
			glGetShaderiv(geometryShader, GL_COMPILE_STATUS, &success);
			if (!success)
			{
				glGetShaderInfoLog(geometryShader, 512, NULL, infoLog);
				last_error += "\nFragment compilation failed from ";
				last_error += geometryPath;
				last_error += ":\n";
				last_error += eastl::string(infoLog);
				last_error += "\n";
				ret = false;
			}
		}
		else
		{
			last_error += "\nError Loading fragment shader from ";
			last_error += geometryPath;
			last_error += "\n";
			ret = false;
		}
	}

	//creating Shader program, link the once shaders types
	*ID = glCreateProgram();
	if(vertexShader != 0) glAttachShader(*ID, vertexShader);
	if (fragmentShader != 0) glAttachShader(*ID, fragmentShader);
	if (geometryShader != 0) glAttachShader(*ID, geometryShader);
	glLinkProgram(*ID);

	//check
	glGetProgramiv(*ID, GL_LINK_STATUS, &success);
	if (!success)
	{
		glGetProgramInfoLog(*ID, 512, NULL, infoLog);
		last_error += "\nShader program compilation failed:\n";
		last_error += infoLog;
		last_error += "\n";
		if(!compileTest) glDeleteProgram(*ID);
		ret = false;
	}

	//deleting shaders, no needed after link
	if (vertexShader != 0) glDeleteShader(vertexShader);
	if (fragmentShader != 0) glDeleteShader(fragmentShader);
	if (geometryShader != 0) glDeleteShader(geometryShader);
	if(compileTest) glDeleteProgram(*ID);

	if (!ret) shader_last_error = last_error;

	return ret;
}

bool RE_ShaderImporter::LoadFromBuffer(unsigned int* ID, const char* vertexBuffer, size_t vSize, const char* fragmentBuffer, size_t fSize, const char* geometryBuffer, size_t gSize)
{
	bool ret = true;
	eastl::string last_error;

	const char* buffer = nullptr;
	int  success;
	char infoLog[512];

	unsigned int vertexShader = 0;
	unsigned int fragmentShader = 0;
	unsigned int geometryShader = 0;

	if (vertexBuffer)
	{
		//compiling vertex shader
		vertexShader = glCreateShader(GL_VERTEX_SHADER);

		auto vbsize = static_cast<GLint>(vSize);
		glShaderSource(vertexShader, 1, &vertexBuffer, &vbsize);
		glCompileShader(vertexShader);

		//check
		glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
		if (!success)
		{
			RE_LOG_WARNING("Vertex compilation failed: ");
			glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
			RE_LOG_ERROR(&infoLog[0]);
			last_error += "\nVertex compilation failed from\n";
			last_error += infoLog;
			last_error += "\n";
			ret = false;
		}
	}

	if (fragmentBuffer)
	{
		//compiling fragment shader
		fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

		auto fbSize = static_cast<GLint>(fSize);
		glShaderSource(fragmentShader, 1, &fragmentBuffer, &fbSize);
		glCompileShader(fragmentShader);

		//check
		glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
		if (!success)
		{
			RE_LOG_WARNING("Fragment compilation failed: ");
			glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
			RE_LOG_ERROR(&infoLog[0]);
			last_error += "\nFragment compilation failed from\n";
			last_error += infoLog;
			last_error += "\n";
			ret = false;
		}
	}

	if (geometryBuffer)
	{
		//compiling geometry shader
		geometryShader = glCreateShader(GL_GEOMETRY_SHADER);

		auto gbSize = static_cast<GLint>(gSize);
		glShaderSource(geometryShader, 1, &buffer, &gbSize);
		glCompileShader(geometryShader);

		//check
		glGetShaderiv(geometryShader, GL_COMPILE_STATUS, &success);
		if (!success)
		{
			RE_LOG_WARNING("Geometry compilation failed: ");
			glGetShaderInfoLog(geometryShader, 512, NULL, infoLog);
			RE_LOG_ERROR(&infoLog[0]);
			last_error += "\nGeometry compilation failed from\n";
			last_error += infoLog;
			last_error += "\n";
			ret = false;
		}

	}

	//creating Shader program, link the once shaders types
	*ID = glCreateProgram();

	if (vertexShader != 0) glAttachShader(*ID, vertexShader);
	if (fragmentShader != 0) glAttachShader(*ID, fragmentShader);
	if (geometryShader != 0) glAttachShader(*ID, geometryShader);
	glLinkProgram(*ID);

	//check
	glGetProgramiv(*ID, GL_LINK_STATUS, &success);
	if (!success) {
		RE_LOG_WARNING("Shader program compilation failed:\n");
		glGetProgramInfoLog(*ID, 512, NULL, infoLog);
		RE_LOG_ERROR(&infoLog[0]);
		last_error += "\nShader program compilation failed";
		last_error += ":\n";
		last_error += infoLog;
		last_error += "\n";
		glDeleteProgram(*ID);
		ret = false;
	}

	//deleting shaders, no needed after link
	if (vertexShader != 0) glDeleteShader(vertexShader);
	if (fragmentShader != 0) glDeleteShader(fragmentShader);
	if (geometryShader != 0) glDeleteShader(geometryShader);

	if (!ret) shader_last_error = last_error;

	return ret;
}

bool RE_ShaderImporter::LoadFromBinary(const char* buffer, size_t size, unsigned int* ID)
{
	int ret;

	glProgramBinary((*ID = glCreateProgram()), binaryFormats[0], buffer, static_cast<GLsizei>(size));
	glValidateProgram(*ID);
	glGetProgramiv(*ID, GL_VALIDATE_STATUS, &ret);
	if (!ret) glDeleteProgram(*ID);

	return ret;
}

bool RE_ShaderImporter::GetBinaryProgram(unsigned int ID, char** buffer, int* size)
{
	bool ret = false;
	if (ID > 0)
	{
		glGetProgramiv(ID, GL_PROGRAM_BINARY_LENGTH, size);
		if (*size > 0)
		{
			*buffer = new char[*size];
			glGetProgramBinary(ID, *size, size, reinterpret_cast<GLenum*>(binaryFormats), *buffer);
			ret = true;
		}
	}

	return ret;
}

bool RE_ShaderImporter::Compile(const char* buffer, size_t size, unsigned int GLCompile)
{
	bool ret = true;
	eastl::string last_error;

	auto shaderScript = glCreateShader(GLCompile);

	auto bSize = static_cast<GLint>(size);
	glShaderSource(shaderScript, 1, &buffer, &bSize);
	glCompileShader(shaderScript);

	int  success;
	char infoLog[512];
	//check
	glGetShaderiv(shaderScript, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(shaderScript, 512, NULL, infoLog);
		last_error += "\nVertex compilation failed from\n";
		last_error += infoLog;
		last_error += "\n";
		ret = false;
	}
	glDeleteShader(shaderScript);

	if (!ret) shader_last_error = last_error;

	return ret;
}

const char * RE_ShaderImporter::GetShaderError()
{
	return shader_last_error.c_str();
}

void RE_ShaderImporter::use(unsigned int ID)
{
	glUseProgram(ID);
}

void RE_ShaderImporter::Delete(unsigned int ID)
{
	glUseProgram(ID);
}

int RE_ShaderImporter::getLocation(unsigned int ID, const char* name)
{
	return glGetUniformLocation(ID, name);
}

void RE_ShaderImporter::setBool(unsigned int ID, const char* name, bool value)
{
	glUniform1i(glGetUniformLocation(ID, name), (int)value);
}

void RE_ShaderImporter::setBool(int loc, bool value)
{
	glUniform1i(loc, (int)value);
}

void RE_ShaderImporter::setBool(unsigned int ID, const char* name, bool value, bool value2)
{
	glUniform2i(glGetUniformLocation(ID, name), (int)value, (int)value2);
}

void RE_ShaderImporter::setBool(int loc, bool value, bool value2)
{
	glUniform2i(loc, (int)value, (int)value2);
}

void RE_ShaderImporter::setBool(unsigned int ID, const char* name, bool value, bool value2, bool value3) 
{
	glUniform3i(glGetUniformLocation(ID, name), (int)value, (int)value2, (int)value3);
}

void RE_ShaderImporter::setBool(int loc, bool value, bool value2, bool value3)
{
	glUniform3i(loc, (int)value, (int)value2, (int)value3);
}

void RE_ShaderImporter::setBool(unsigned int ID, const char* name, bool value, bool value2, bool value3, bool value4) 
{
	glUniform4i(glGetUniformLocation(ID, name), (int)value, (int)value2, (int)value3, (int)value4);
}

void RE_ShaderImporter::setBool(int loc, bool value, bool value2, bool value3, bool value4)
{
	glUniform4i(loc, (int)value, (int)value2, (int)value3, (int)value4);
}

void RE_ShaderImporter::setInt(unsigned int ID, const char* name, int value) 
{
	glUniform1i(glGetUniformLocation(ID, name), value);
}

void RE_ShaderImporter::setInt(int loc, int value)
{
	glUniform1i(loc, value);
}

void RE_ShaderImporter::setInt(unsigned int ID, const char * name, int value, int value2) 
{
	glUniform2i(glGetUniformLocation(ID, name), value, value2);
}

void RE_ShaderImporter::setInt(int loc, int value, int value2)
{
	glUniform2i(loc, value, value2);
}

void RE_ShaderImporter::setInt(unsigned int ID, const char * name, int value, int value2, int value3) 
{
	glUniform3i(glGetUniformLocation(ID, name), value, value2, value3);
}

void RE_ShaderImporter::setInt(int loc, int value, int value2, int value3)
{
	glUniform3i(loc, value, value2, value3);
}

void RE_ShaderImporter::setInt(unsigned int ID, const char * name, int value, int value2, int value3, int value4) 
{
	glUniform4i(glGetUniformLocation(ID, name), value, value2, value3, value4);
}

void RE_ShaderImporter::setInt(int loc, int value, int value2, int value3, int value4)
{
	glUniform4i(loc, value, value2, value3, value4);
}

void RE_ShaderImporter::setFloat(unsigned int ID, const char*name, float value) 
{
	glUniform1f(glGetUniformLocation(ID, name), value);
}

void RE_ShaderImporter::setFloat(int loc, float value)
{
	glUniform1f(loc, value);
}

void RE_ShaderImporter::setFloat(unsigned int ID, const char * name, float value, float value2) 
{
	glUniform2f(glGetUniformLocation(ID, name), value, value2);
}

void RE_ShaderImporter::setFloat(int loc, float value, float value2)
{
	glUniform2f(loc, value, value2);
}

void RE_ShaderImporter::setFloat(unsigned int ID, const char * name, float value, float value2, float value3) 
{
	glUniform3f(glGetUniformLocation(ID, name), value, value2, value3);
}

void RE_ShaderImporter::setFloat(int loc, float value, float value2, float value3)
{
	glUniform3f(loc, value, value2, value3);
}

void RE_ShaderImporter::setFloat(unsigned int ID, const char * name, float value, float value2, float value3, float value4) 
{
	glUniform4f(glGetUniformLocation(ID, name), value, value2, value3, value4);
}

void RE_ShaderImporter::setFloat(int loc, float value, float value2, float value3, float value4)
{
	glUniform4f(loc, value, value2, value3, value4);
}

void RE_ShaderImporter::setFloat(unsigned int ID, const char * name, math::vec value) 
{
	glUniform3f(glGetUniformLocation(ID, name), value.x, value.y, value.z);
}

void RE_ShaderImporter::setFloat(int loc, math::vec value)
{
	glUniform3f(loc, value.x, value.y, value.z);
}

void RE_ShaderImporter::setUnsignedInt(unsigned int ID, const char * name, unsigned int value)
{
	glUniform1ui(glGetUniformLocation(ID, name), value);
}

void RE_ShaderImporter::setUnsignedInt(int loc, unsigned int value)
{
	glUniform1ui(loc, value);
}

void RE_ShaderImporter::setUnsignedInt(unsigned int ID, const char * name, unsigned int value, unsigned int value2)
{
	glUniform2ui(glGetUniformLocation(ID, name), value, value2);
}

void RE_ShaderImporter::setUnsignedInt(int loc, unsigned int value, unsigned int value2)
{
	glUniform2ui(loc, value, value2);
}

void RE_ShaderImporter::setUnsignedInt(unsigned int ID, const char * name, unsigned int value, unsigned int value2, unsigned int value3)
{
	glUniform3ui(glGetUniformLocation(ID, name), value, value2, value3);
}

void RE_ShaderImporter::setUnsignedInt(int loc, unsigned int value, unsigned int value2, unsigned int value3)
{
	glUniform3ui(loc, value, value2, value3);
}

void RE_ShaderImporter::setUnsignedInt(unsigned int ID, const char * name, unsigned int value, unsigned int value2, unsigned int value3, unsigned int value4)
{
	glUniform4ui(glGetUniformLocation(ID, name), value, value2, value3, value4);
}

void RE_ShaderImporter::setUnsignedInt(int loc, unsigned int value, unsigned int value2, unsigned int value3, unsigned int value4)
{
	glUniform4ui(loc, value, value2, value3, value4);
}

void RE_ShaderImporter::setFloat3x3(unsigned int ID, const char * name, const float * trans)
{
	glUniformMatrix3fv(glGetUniformLocation(ID, name), 1, GL_FALSE, trans);
}

void RE_ShaderImporter::setFloat3x3(int loc, const float* trans)
{
	glUniformMatrix3fv(loc, 1, GL_FALSE, trans);
}

void RE_ShaderImporter::setFloat4x4(unsigned int ID, const char * name, const float* trans)
{
	glUniformMatrix4fv(glGetUniformLocation(ID, name), 1, GL_FALSE, trans);
}

void RE_ShaderImporter::setFloat4x4(int loc, const float* trans)
{
	glUniformMatrix4fv(loc, 1, GL_FALSE, trans);
}