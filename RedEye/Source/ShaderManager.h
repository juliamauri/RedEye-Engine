#ifndef __SHADERMANAGER_H__
#define __SHADERMANAGER_H__

#include "RE_Math.h"

#include <string>
#include <vector>

class ShaderManager
{
public:
	// constructor reads and builds the shader
	ShaderManager(const char* folderPath);

	//delete all programs loaded
	~ShaderManager();

	//Load shaders and put in vector
	bool Load(const char* name, unsigned int* ID);
	//get last error
	const char* GetShaderError();
	// use/activate the shader
	void use(unsigned int ID);
	//Delete manually shader
	void Delete(unsigned int ID);

	// utility uniform functions
	void setBool(unsigned int ID, const char* name, bool value) const;
	void setBool(unsigned int ID, const char* name, bool value, bool value2) const;
	void setBool(unsigned int ID, const char* name, bool value, bool value2, bool value3) const;
	void setBool(unsigned int ID, const char* name, bool value, bool value2, bool value3, bool value4) const;

	void setInt(unsigned int ID, const char* name, int value) const;
	void setInt(unsigned int ID, const char* name, int value, int value2) const;
	void setInt(unsigned int ID, const char* name, int value, int value2, int value3) const;
	void setInt(unsigned int ID, const char* name, int value, int value2, int value3, int value4) const;

	void setFloat(unsigned int ID, const char* name, float value) const;
	void setFloat(unsigned int ID, const char* name, float value, float value2) const;
	void setFloat(unsigned int ID, const char* name, float value, float value2, float value3) const;
	void setFloat(unsigned int ID, const char* name, float value, float value2, float value3, float value4) const;
	void setFloat(unsigned int ID, const char* name, math::vec value) const;

	void setUnsignedInt(unsigned int ID, const char* name, unsigned int value);
	void setUnsignedInt(unsigned int ID, const char* name, unsigned int value, unsigned int value2);
	void setUnsignedInt(unsigned int ID, const char* name, unsigned int value, unsigned int value2, unsigned int value3);
	void setUnsignedInt(unsigned int ID, const char* name, unsigned int value, unsigned int value2, unsigned int value3, unsigned int value4);

	void setFloat4x4(unsigned int ID, const char* name, float* trans);

private:
	const char* folderPath;
	// the program ID
	std::vector<unsigned int> shaders;
	//Last error
	std::string last_error;
};

#endif // __SHADERMANAGER_H__