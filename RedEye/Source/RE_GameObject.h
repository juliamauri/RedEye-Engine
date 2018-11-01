#ifndef __RE_GAMEOBJECT_H__
#define __RE_GAMEOBJECT_H__

#include "Globals.h"
#include "MathGeoLib\include\MathGeoLib.h"
#include <list>

class RE_Component;
class RE_CompTransform;
class RE_CompMesh;
class RE_CompCamera;

class RE_GameObject
{
public:
	RE_GameObject(const char* name, RE_GameObject* parent = nullptr, const bool start_active = true);
	~RE_GameObject();

	void PreUpdate();
	void Update();
	void PostUpdate();
	void Draw();

	// Children
	void AddChild(RE_GameObject* child);
	void RemoveChild(RE_GameObject* child); //Breaks the link with the parent but does not delete the child.
	void RemoveAllChilds();
	const std::list<RE_GameObject*>* GetChilds() const;
	unsigned int ChildCount() const;

	// Parent
	RE_GameObject* GetParent() const;
	void SetParent(RE_GameObject* parent);

	// Active
	bool IsActive() const;
	void SetActive(const bool value);
	void SetActiveAll(const bool value);

	// Components
	RE_Component* AddComponent(const ushortint type, const char* file_path_data = nullptr, const bool dropped = false);
	RE_Component* GetComponent(const ushortint type) const;
	void RemoveComponent(RE_Component* component);
	void RemoveAllComponents();

	RE_CompTransform* AddCompTransform();
	RE_CompMesh* AddCompMesh(const char* file_path_data = nullptr, const bool dropped = false);
	RE_CompCamera* AddCompCamera();

	RE_CompTransform* GetTransform() const;
	RE_CompMesh* GetMesh() const;
	RE_CompCamera* GetCamera() const;

	// Transform
	void TransformModified();

	// Editor
	void DrawProperties();

private:

	bool active = true;
	std::string name;
	math::AABB bounding_box;

	RE_GameObject* parent = nullptr;
	RE_CompTransform* transform = nullptr;

	std::list<RE_GameObject*> childs;
	std::list<RE_Component*> components;
};

#endif // !__RE_GAMEOBJECT_H__