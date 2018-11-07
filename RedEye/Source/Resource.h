#ifndef __RESOURCE_H__
#define __RESOURCE_H__

class RE_Mesh;
class RE_Shader;
class RE_Texture;
class RE_Primitive;

enum Resource_Type : short unsigned int
{
	R_UNDEFINED = 0x00,
	R_SHADER,
	R_PRIMITIVE,
	R_TEXTURE,
	R_MESH
};

union Container
{
	RE_Mesh* mesh = nullptr;
	RE_Shader* shader;
	RE_Texture* texture;
	RE_Primitive* primitive;
};

class ResourceContainer
{
public:
	ResourceContainer(const char* name = nullptr, const char* origin = nullptr, Resource_Type type = R_UNDEFINED, unsigned int id = 0u);
	virtual ~ResourceContainer();
	const char* GetName() const;
	const char* GetOrigin() const;
	unsigned int GetID() const;
	Resource_Type GetType() const;
	
	void SetID(unsigned int id);
	void SetType(Resource_Type type);

private:
	const char* name;
	const char* origin;

	unsigned int id;
	Resource_Type type;
	Container contains;
};

#endif // !__RESOURCE_H__