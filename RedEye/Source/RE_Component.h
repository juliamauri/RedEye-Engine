#ifndef __RE_COMPONENT_H__
#define __RE_COMPONENT_H__

class RE_GameObject;

enum ComponentType : unsigned short int
{
	C_EMPTY,
	C_TRANSFORM,
	C_PRIMITIVE,
	C_AXIS,
	C_POINT,
	C_LINE,
	C_RAY,
	C_TRIANGLE,
	C_PLANE,
	C_CUBE,
	C_FUSTRUM,
	C_SPHERE,
	C_CYLINDER,
	C_CAPSULE,
	C_MESH,
	C_CAMERA
};

class RE_Component
{
public:
	RE_Component(ComponentType type = C_EMPTY, RE_GameObject* go = nullptr, bool start_active = true);
	virtual ~RE_Component() {}

	virtual void PreUpdate() {}
	virtual void Update() {}
	virtual void PostUpdate() {}

	virtual void DrawProperties() {}

	bool IsActive() const;
	void SetActive(bool value);

	ComponentType GetType() const;
	RE_GameObject* GetGO() const;

	virtual void Save() const {}
	virtual void Load() {}
	virtual void Reset() {}

	virtual void OnTransformModified() {}

protected:

	bool active = true;
	ComponentType type = C_EMPTY;
	RE_GameObject* go = nullptr;
};

#endif // !__RE_COMPONENT_H__