#ifndef __RESOURCE_H__
#define __RESOURCE_H__

// disable warning for unsafe copy
#pragma warning(disable : 4996)

class RE_Mesh;
class RE_Shader;
class RE_Texture;
class RE_Primitive;

#include <string>

enum Resource_Type : short unsigned int
{
	R_UNDEFINED = 0x00,
	R_SHADER,
	R_PRIMITIVE,
	R_TEXTURE,
	R_MESH,
	R_PREFAB,
	R_INTERNALPREFAB,
	R_MATERIAL
};

class ResourceContainer
{
public:
	ResourceContainer(const char* name = nullptr, const char* origin = nullptr, Resource_Type type = R_UNDEFINED, const char* md5 = nullptr);
	virtual ~ResourceContainer();
	const char* GetName() const;
	const char* GetFilePath() const;
	const char* GetOrigin() const;
	const char* GetMD5() const;
	Resource_Type GetType() const;
	
	void SetType(Resource_Type type);
	void SetMD5(const char* md5);
	void SetFilePath(const char* path);

private:
	const char* name;
	const char* origin;
	std::string from_file;
	const char* md5 = nullptr;
	Resource_Type type;
};

#endif // !__RESOURCE_H__