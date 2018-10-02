#ifndef __RE_GAMEOBJECT_H__
#define __RE_GAMEOBJECT_H__

#include <list>

class RE_Component;
class RE_CompTransform;

class RE_GameObject
{
public:
	RE_GameObject(RE_GameObject* parent = nullptr, bool start_active = true);
	~RE_GameObject();

	void PreUpdate();
	void Update();
	void PostUpdate();

	bool AddChild(RE_GameObject* child);
	bool RemoveChild(RE_GameObject* child); //Breaks the link with the parent but does not delete the child.
	void RemoveAllChilds();
	const std::list<RE_GameObject*>* GetChilds() const;
	unsigned int ChildCount() const;

	RE_GameObject* GetParent()const;
	void SetParent(RE_GameObject* parent);

	bool IsActive() const;
	void SetActive(bool value);
	void SetActiveAll(bool value);

	RE_Component* AddComponent(short unsigned int type, char* file_path_data = nullptr);
	bool RemoveComponent(RE_Component* component);
	void RemoveAllComponents();

	void TransformModified();

public:

	RE_CompTransform* transform = nullptr;

private:

	bool active = true;

	RE_GameObject* parent = nullptr;
	std::list<RE_GameObject*> sons;
	std::list<RE_Component*> components;

};

#endif // !__RE_GAMEOBJECT_H__