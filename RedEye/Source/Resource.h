#ifndef __RESOURCE_H__
#define __RESOURCE_H__

class ResourceContainer
{
public:
	ResourceContainer(const char* name = nullptr, const char* origin = nullptr);
	virtual ~ResourceContainer();
	const char* GetName() const;
	const char* GetOrigin() const;

private:

	const char* name;
	const char* origin;
};

#endif // !__RESOURCE_H__