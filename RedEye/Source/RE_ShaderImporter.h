#ifndef __RE_SHADERIMPORTER_H__
#define __RE_SHADERIMPORTER_H__

#include "RE_Math.h"

#include <EASTL/string.h>

class RE_ShaderImporter
{
public:
	// constructor reads and builds the shader
	RE_ShaderImporter(const char* folder = "Assets/Shaders/");

	//delete all programs loaded
	~RE_ShaderImporter();

	bool Init();

	//Load shaders and put in vector
	bool LoadFromAssets(unsigned int* ID, const char* vertexPath, const char* fragmentPath, const char* geometryPath = nullptr, bool compileTest = false);
	bool LoadFromBuffer(unsigned int* ID, const char* vertexBuffer, unsigned int vSize, const char* fragmentBuffer, unsigned int fSize, const char* geometryBuffer = nullptr, unsigned int gSize = 0 );
	bool LoadFromBinary(const char* buffer, unsigned int size, unsigned int* ID);
	bool GetBinaryProgram(unsigned int ID, char** buffer, int* size);

	bool Compile(const char* buffer, unsigned int size, unsigned int GLCompile = 0x8B31);

	//get last error
	const char* GetShaderError();

	// use/activate the shader
	static void use(unsigned int ID);
	//Delete manually shader
	static void Delete(unsigned int ID);
	//Returns location
	static int getLocation(unsigned int ID, const char* name);

	// utility uniform functions
	static void setBool(unsigned int ID, const char* name, bool value);
	static void setBool(unsigned int ID, const char* name, bool value, bool value2);
	static void setBool(unsigned int ID, const char* name, bool value, bool value2, bool value3);
	static void setBool(unsigned int ID, const char* name, bool value, bool value2, bool value3, bool value4);

	static void setBool(int loc, bool value);
	static void setBool(int loc, bool value, bool value2);
	static void setBool(int loc, bool value, bool value2, bool value3);
	static void setBool(int loc, bool value, bool value2, bool value3, bool value4);

	static void setInt(unsigned int ID, const char* name, int value);
	static void setInt(unsigned int ID, const char* name, int value, int value2);
	static void setInt(unsigned int ID, const char* name, int value, int value2, int value3);
	static void setInt(unsigned int ID, const char* name, int value, int value2, int value3, int value4);

	static void setInt(int loc, int value);
	static void setInt(int loc, int value, int value2);
	static void setInt(int loc, int value, int value2, int value3);
	static void setInt(int loc, int value, int value2, int value3, int value4);

	static void setFloat(unsigned int ID, const char* name, float value);
	static void setFloat(unsigned int ID, const char* name, float value, float value2);
	static void setFloat(unsigned int ID, const char* name, float value, float value2, float value3);
	static void setFloat(unsigned int ID, const char* name, float value, float value2, float value3, float value4);
	static void setFloat(unsigned int ID, const char* name, math::vec value);

	static void setFloat(int loc, float value);
	static void setFloat(int loc, float value, float value2);
	static void setFloat(int loc, float value, float value2, float value3);
	static void setFloat(int loc, float value, float value2, float value3, float value4);
	static void setFloat(int loc, math::vec value);
	
	static void setUnsignedInt(unsigned int ID, const char* name, unsigned int value);
	static void setUnsignedInt(unsigned int ID, const char* name, unsigned int value, unsigned int value2);
	static void setUnsignedInt(unsigned int ID, const char* name, unsigned int value, unsigned int value2, unsigned int value3);
	static void setUnsignedInt(unsigned int ID, const char* name, unsigned int value, unsigned int value2, unsigned int value3, unsigned int value4);

	static void setUnsignedInt(int loc, unsigned int value);
	static void setUnsignedInt(int loc, unsigned int value, unsigned int value2);
	static void setUnsignedInt(int loc, unsigned int value, unsigned int value2, unsigned int value3);
	static void setUnsignedInt(int loc, unsigned int value, unsigned int value2, unsigned int value3, unsigned int value4);

	static void setFloat3x3(unsigned int ID, const char* name, const float* trans);
	static void setFloat4x4(unsigned int ID, const char* name, const float* trans);

	static void setFloat3x3(int loc, const float* trans);
	static void setFloat4x4(int loc, const float* trans);


private:
	const char* folderPath;

	//Last error
	eastl::string last_error;

	int* binaryFormats = nullptr;
};

#endif // __RE_SHADERIMPORTER_H__