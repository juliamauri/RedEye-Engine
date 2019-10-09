#include "ShaderManager.h"

#include "Application.h"
#include "ModuleEditor.h"
#include "FileSystem.h"
#include "OutputLog.h"
#include "SDL2/include/SDL.h"
#include "Glew/include/glew.h"

ShaderManager::ShaderManager(const char * folderPath) : folderPath(folderPath) {}

ShaderManager::~ShaderManager()
{
	for (unsigned int ID : shaders) {
		glDeleteProgram(ID);
	}
}

bool ShaderManager::Init()
{
	LOG("Initializing Shader Manager");

	App->ReportSoftware("GLSLang", (char*)glGetString(GL_SHADING_LANGUAGE_VERSION), "https://www.opengl.org/sdk/docs/tutorials/ClockworkCoders/glsl_overview.php");
	
	bool ret = (folderPath != nullptr);
	if (!ret) LOG_ERROR("Shader Manager could not read folder path");

	return ret;
}

bool ShaderManager::Load(const char* name, unsigned int* ID)
{
	LOG("Loading %s shader.", name);

	bool ret = true;
	last_error.clear();

	const char* buffer = nullptr;
	int  success;
	char infoLog[512];

	//compiling vertex shader
	unsigned int vertexShader;
	vertexShader = glCreateShader(GL_VERTEX_SHADER);

	//load source
	std::string path(this->folderPath);
	path += name;
	path += ".vert";
	RE_FileIO file_vertexShader(path.c_str());
	if (file_vertexShader.Load())
	{
		buffer = file_vertexShader.GetBuffer();
		glShaderSource(vertexShader, 1, &buffer, NULL);
		glCompileShader(vertexShader);

		//check
		glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
		if (!success)
		{
			glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
			last_error += "\nVertex compilation failed from ";
			last_error += name;
			last_error += ":\n";
			last_error += infoLog;
			last_error += "\n";
			ret = false;
		}
	}
	else
	{
		last_error += "\nError Loading vertex shader from ";
		last_error += path.c_str();
		last_error += "\n";
		ret = false;
	}

	//compiling fragment shader
	unsigned int fragmentShader;
	fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

	//load source
	path = this->folderPath;
	path += name;
	path += ".frag";
	RE_FileIO file_fragmentShader(path.c_str());
	if (file_fragmentShader.Load())
	{
		buffer = file_fragmentShader.GetBuffer();
		glShaderSource(fragmentShader, 1, &buffer, NULL);
		glCompileShader(fragmentShader);

		//check
		glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
		if (!success)
		{
			glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
			last_error += "\nFragment compilation failed from ";
			last_error += name;
			last_error += ":\n";
			last_error += infoLog;
			last_error += "\n";
			ret = false;
		}
	}
	else
	{
		last_error += "\nError Loading fragment shader from ";
		last_error += path.c_str();
		last_error += "\n";
		ret = false;
	}

	//creating Shader program, link the once shaders types
	*ID = glCreateProgram();

	glAttachShader(*ID, vertexShader);
	glAttachShader(*ID, fragmentShader);
	glLinkProgram(*ID);

	//check
	glGetProgramiv(*ID, GL_LINK_STATUS, &success);
	if (!success) {
		glGetProgramInfoLog(*ID, 512, NULL, infoLog);
		last_error += "\nShader program compilation failed from ";
		last_error += name;
		last_error += ":\n";
		last_error += infoLog;
		last_error += "\n";
		glDeleteProgram(*ID);
		ret = false;
	}
	else
		shaders.push_back(*ID);

	//deleting shaders, no needed after link
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	return ret;
}

const char * ShaderManager::GetShaderError()
{
	return last_error.c_str();
}

void ShaderManager::use(unsigned int ID)
{
	glUseProgram(ID);
}

void ShaderManager::Delete(unsigned int ID)
{
	glUseProgram(ID);
}

void ShaderManager::setBool(unsigned int ID, const char* name, bool value)
{
	glUniform1i(glGetUniformLocation(ID, name), (int)value);
}

void ShaderManager::setBool(unsigned int ID, const char* name, bool value, bool value2)
{
	glUniform2i(glGetUniformLocation(ID, name), (int)value, (int)value2);
}

void ShaderManager::setBool(unsigned int ID, const char* name, bool value, bool value2, bool value3) 
{
	glUniform3i(glGetUniformLocation(ID, name), (int)value, (int)value2, (int)value3);
}

void ShaderManager::setBool(unsigned int ID, const char* name, bool value, bool value2, bool value3, bool value4) 
{
	glUniform4i(glGetUniformLocation(ID, name), (int)value, (int)value2, (int)value3, (int)value4);
}

void ShaderManager::setInt(unsigned int ID, const char* name, int value) 
{
	glUniform1i(glGetUniformLocation(ID, name), value);
}

void ShaderManager::setInt(unsigned int ID, const char * name, int value, int value2) 
{
	glUniform2i(glGetUniformLocation(ID, name), value, value2);
}

void ShaderManager::setInt(unsigned int ID, const char * name, int value, int value2, int value3) 
{
	glUniform3i(glGetUniformLocation(ID, name), value, value2, value3);
}

void ShaderManager::setInt(unsigned int ID, const char * name, int value, int value2, int value3, int value4) 
{
	glUniform4i(glGetUniformLocation(ID, name), value, value2, value3, value4);
}

void ShaderManager::setFloat(unsigned int ID, const char*name, float value) 
{
	glUniform1f(glGetUniformLocation(ID, name), value);
}

void ShaderManager::setFloat(unsigned int ID, const char * name, float value, float value2) 
{
	glUniform2f(glGetUniformLocation(ID, name), value, value2);
}

void ShaderManager::setFloat(unsigned int ID, const char * name, float value, float value2, float value3) 
{
	glUniform3f(glGetUniformLocation(ID, name), value, value2, value3);
}

void ShaderManager::setFloat(unsigned int ID, const char * name, float value, float value2, float value3, float value4) 
{
	glUniform4f(glGetUniformLocation(ID, name), value, value2, value3, value4);
}

void ShaderManager::setFloat(unsigned int ID, const char * name, math::vec value) 
{
	glUniform3f(glGetUniformLocation(ID, name), value.x, value.y, value.z);
}

void ShaderManager::setUnsignedInt(unsigned int ID, const char * name, unsigned int value)
{
	glUniform1ui(glGetUniformLocation(ID, name), value);
}

void ShaderManager::setUnsignedInt(unsigned int ID, const char * name, unsigned int value, unsigned int value2)
{
	glUniform2ui(glGetUniformLocation(ID, name), value, value2);
}

void ShaderManager::setUnsignedInt(unsigned int ID, const char * name, unsigned int value, unsigned int value2, unsigned int value3)
{
	glUniform3ui(glGetUniformLocation(ID, name), value, value2, value3);
}

void ShaderManager::setUnsignedInt(unsigned int ID, const char * name, unsigned int value, unsigned int value2, unsigned int value3, unsigned int value4)
{
	glUniform4ui(glGetUniformLocation(ID, name), value, value2, value3, value4);
}

void ShaderManager::setFloat3x3(unsigned int ID, const char * name, const float * trans)
{
	glUniformMatrix3fv(glGetUniformLocation(ID, name), 1, GL_FALSE, trans);
}

void ShaderManager::setFloat4x4(unsigned int ID, const char * name, const float* trans)
{
	glUniformMatrix4fv(glGetUniformLocation(ID, name), 1, GL_FALSE, trans);
}
