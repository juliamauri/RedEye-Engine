#ifndef __RE_SHADERIMPORTER_H__
#define __RE_SHADERIMPORTER_H__

#include "RE_DataTypes.h"
#include <MGL/Math/float3.h>

namespace RE_ShaderImporter
{
	void Init();
	void Clear();

	//Load shaders and put in vector
	bool LoadFromAssets(uint* ID, const char* vertexPath, const char* fragmentPath, const char* geometryPath = nullptr, bool compileTest = false);
	bool LoadFromBuffer(uint* ID, const char* vertexBuffer, size_t vSize, const char* fragmentBuffer, size_t fSize, const char* geometryBuffer = nullptr, size_t gSize = 0 );
	bool LoadFromBinary(const char* buffer, size_t size, uint* ID);
	bool GetBinaryProgram(uint ID, char** buffer, int* size);

	bool Compile(const char* buffer, size_t size, uint GLCompile = 0x8B31);

	//get last error
	const char* GetShaderError();
	
	void use(uint ID); // Use/Activate shader
	void Delete(uint ID); // Delete shader manually
	int getLocation(uint ID, const char* name);

	// utility uniform functions
	void setBool(uint ID, const char* name, bool value);
	void setBool(uint ID, const char* name, bool value, bool value2);
	void setBool(uint ID, const char* name, bool value, bool value2, bool value3);
	void setBool(uint ID, const char* name, bool value, bool value2, bool value3, bool value4);
	
	void setBool(int loc, bool value);
	void setBool(int loc, bool value, bool value2);
	void setBool(int loc, bool value, bool value2, bool value3);
	void setBool(int loc, bool value, bool value2, bool value3, bool value4);
	
	void setInt(uint ID, const char* name, int value);
	void setInt(uint ID, const char* name, int value, int value2);
	void setInt(uint ID, const char* name, int value, int value2, int value3);
	void setInt(uint ID, const char* name, int value, int value2, int value3, int value4);
	
	void setInt(int loc, int value);
	void setInt(int loc, int value, int value2);
	void setInt(int loc, int value, int value2, int value3);
	void setInt(int loc, int value, int value2, int value3, int value4);
	
	void setFloat(uint ID, const char* name, float value);
	void setFloat(uint ID, const char* name, float value, float value2);
	void setFloat(uint ID, const char* name, float value, float value2, float value3);
	void setFloat(uint ID, const char* name, float value, float value2, float value3, float value4);
	void setFloat(uint ID, const char* name, math::vec value);
	
	void setFloat(int loc, float value);
	void setFloat(int loc, float value, float value2);
	void setFloat(int loc, float value, float value2, float value3);
	void setFloat(int loc, float value, float value2, float value3, float value4);
	void setFloat(int loc, math::vec value);
	
	void setUnsignedInt(uint ID, const char* name, uint value);
	void setUnsignedInt(uint ID, const char* name, uint value, uint value2);
	void setUnsignedInt(uint ID, const char* name, uint value, uint value2, uint value3);
	void setUnsignedInt(uint ID, const char* name, uint value, uint value2, uint value3, uint value4);

	void setUnsignedInt(int loc, uint value);
	void setUnsignedInt(int loc, uint value, uint value2);
	void setUnsignedInt(int loc, uint value, uint value2, uint value3);
	void setUnsignedInt(int loc, uint value, uint value2, uint value3, uint value4);

	void setFloat3x3(uint ID, const char* name, const float* trans);
	void setFloat4x4(uint ID, const char* name, const float* trans);

	void setFloat3x3(int loc, const float* trans);
	void setFloat4x4(int loc, const float* trans);
};

#endif // __RE_SHADERIMPORTER_H__