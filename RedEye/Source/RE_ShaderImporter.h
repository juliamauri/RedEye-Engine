#ifndef __RE_SHADERIMPORTER_H__
#define __RE_SHADERIMPORTER_H__

#include "RE_Math.h"

#include <EASTL/string.h>

namespace RE_ShaderImporter
{
	void Init();

	//Load shaders and put in vector
	bool LoadFromAssets(unsigned int* ID, const char* vertexPath, const char* fragmentPath, const char* geometryPath = nullptr, bool compileTest = false);
	bool LoadFromBuffer(unsigned int* ID, const char* vertexBuffer, unsigned int vSize, const char* fragmentBuffer, unsigned int fSize, const char* geometryBuffer = nullptr, unsigned int gSize = 0 );
	bool LoadFromBinary(const char* buffer, unsigned int size, unsigned int* ID);
	bool GetBinaryProgram(unsigned int ID, char** buffer, int* size);

	bool Compile(const char* buffer, unsigned int size, unsigned int GLCompile = 0x8B31);

	//get last error
	const char* GetShaderError();

	// use/activate the shader
	void use(unsigned int ID);
	//Delete manually shader
	void Delete(unsigned int ID);
	//Returns location
	int getLocation(unsigned int ID, const char* name);

	// utility uniform functions
	void setBool(unsigned int ID, const char* name, bool value);
	void setBool(unsigned int ID, const char* name, bool value, bool value2);
	void setBool(unsigned int ID, const char* name, bool value, bool value2, bool value3);
	void setBool(unsigned int ID, const char* name, bool value, bool value2, bool value3, bool value4);

	void setBool(int loc, bool value);
	void setBool(int loc, bool value, bool value2);
	void setBool(int loc, bool value, bool value2, bool value3);
	void setBool(int loc, bool value, bool value2, bool value3, bool value4);

	void setInt(unsigned int ID, const char* name, int value);
	void setInt(unsigned int ID, const char* name, int value, int value2);
	void setInt(unsigned int ID, const char* name, int value, int value2, int value3);
	void setInt(unsigned int ID, const char* name, int value, int value2, int value3, int value4);

	void setInt(int loc, int value);
	void setInt(int loc, int value, int value2);
	void setInt(int loc, int value, int value2, int value3);
	void setInt(int loc, int value, int value2, int value3, int value4);

	void setFloat(unsigned int ID, const char* name, float value);
	void setFloat(unsigned int ID, const char* name, float value, float value2);
	void setFloat(unsigned int ID, const char* name, float value, float value2, float value3);
	void setFloat(unsigned int ID, const char* name, float value, float value2, float value3, float value4);
	void setFloat(unsigned int ID, const char* name, math::vec value);

	void setFloat(int loc, float value);
	void setFloat(int loc, float value, float value2);
	void setFloat(int loc, float value, float value2, float value3);
	void setFloat(int loc, float value, float value2, float value3, float value4);
	void setFloat(int loc, math::vec value);
	
	void setUnsignedInt(unsigned int ID, const char* name, unsigned int value);
	void setUnsignedInt(unsigned int ID, const char* name, unsigned int value, unsigned int value2);
	void setUnsignedInt(unsigned int ID, const char* name, unsigned int value, unsigned int value2, unsigned int value3);
	void setUnsignedInt(unsigned int ID, const char* name, unsigned int value, unsigned int value2, unsigned int value3, unsigned int value4);

	void setUnsignedInt(int loc, unsigned int value);
	void setUnsignedInt(int loc, unsigned int value, unsigned int value2);
	void setUnsignedInt(int loc, unsigned int value, unsigned int value2, unsigned int value3);
	void setUnsignedInt(int loc, unsigned int value, unsigned int value2, unsigned int value3, unsigned int value4);

	void setFloat3x3(unsigned int ID, const char* name, const float* trans);
	void setFloat4x4(unsigned int ID, const char* name, const float* trans);

	void setFloat3x3(int loc, const float* trans);
	void setFloat4x4(int loc, const float* trans);

	namespace Internal
	{
		static eastl::string last_error;
		static int* binaryFormats = nullptr;
	}
};

#endif // __RE_SHADERIMPORTER_H__