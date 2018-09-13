#ifndef __SHADER_H__
#define __SHADER_H__

#include <string>

#include "MathGeoLib/include/MathGeoLib.h"

class Shader
{
public:
	// constructor reads and builds the shader
	Shader(const char* vertexPath, const char* fragmentPath);
	// use/activate the shader
	void use();
	// utility uniform functions
	void setBool(const char* name, bool value) const;
	void setBool(const char* name, bool value, bool value2) const;
	void setBool(const char* name, bool value, bool value2, bool value3) const;
	void setBool(const char* name, bool value, bool value2, bool value3, bool value4) const;

	void setInt(const char* name, int value) const;
	void setInt(const char* name, int value, int value2) const;
	void setInt(const char* name, int value, int value2, int value3) const;
	void setInt(const char* name, int value, int value2, int value3, int value4) const;

	void setFloat(const char* name, float value) const;
	void setFloat(const char* name, float value, float value2) const;
	void setFloat(const char* name, float value, float value2, float value3) const;
	void setFloat(const char* name, float value, float value2, float value3, float value4) const;

	void setUnsignedInt(const char* name, unsigned int value);
	void setUnsignedInt(const char* name, unsigned int value, unsigned int value2);
	void setUnsignedInt(const char* name, unsigned int value, unsigned int value2, unsigned int value3);
	void setUnsignedInt(const char* name, unsigned int value, unsigned int value2, unsigned int value3, unsigned int value4);

	void setFloat4x4(const char* name, math::float4x4* trans);

private:
	// the program ID
	unsigned int ID;
};

#endif // __SHADER_H__