#ifndef __RE_GAMEOBJECT_H__
#define __RE_GAMEOBJECT_H__

#include "Globals.h"
#include "MathGeoLib\include\MathGeoLib.h"
#include <list>

class RE_Component;
class RE_CompTransform;

class RE_GameObject
{
public:
	RE_GameObject(RE_GameObject* parent = nullptr, const bool start_active = true);
	~RE_GameObject();

	void PreUpdate();
	void Update();
	void PostUpdate();
	void Draw();

	// Children
	bool AddChild(RE_GameObject* child);
	bool RemoveChild(RE_GameObject* child); //Breaks the link with the parent but does not delete the child.
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
	bool RemoveComponent(RE_Component* component);
	void RemoveAllComponents();

	// Transform
	RE_CompTransform* GetTransform() const;
	void TransformModified();

private:

	bool active = true;

	math::AABB bounding_box;

	RE_GameObject* parent = nullptr;
	std::list<RE_GameObject*> sons;
	std::list<RE_Component*> components;

	RE_CompTransform* transform = nullptr;

};

#endif // !__RE_GAMEOBJECT_H__