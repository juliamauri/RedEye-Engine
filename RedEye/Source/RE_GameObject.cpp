#include "RE_GameObject.h"

#include "RE_Component.h"
#include "RE_CompTransform.h"
#include "RE_CompPrimitive.h"
#include "RE_CompMesh.h"

RE_GameObject::RE_GameObject(RE_GameObject * parent, bool start_active) :
	parent(parent), active(start_active)
{
	AddComponent(C_TRANSFORM);
}

RE_GameObject::~RE_GameObject()
{
	RemoveAllChilds();
	RemoveAllComponents();
}

void RE_GameObject::PreUpdate()
{
	std::list<RE_GameObject*>::iterator it_go = sons.begin();
	for (; it_go != sons.end(); it_go++)
		(*it_go)->PreUpdate();

	std::list<RE_Component*>::iterator it_comp = components.begin();
	for (; it_comp != components.end(); it_comp++)
		(*it_comp)->PreUpdate();
}

void RE_GameObject::Update()
{
	std::list<RE_GameObject*>::iterator it_go = sons.begin();
	for (; it_go != sons.end(); it_go++)
		(*it_go)->Update();

	std::list<RE_Component*>::iterator it_comp = components.begin();
	for (; it_comp != components.end(); it_comp++)
		(*it_comp)->Update();
}

void RE_GameObject::PostUpdate()
{
	std::list<RE_GameObject*>::iterator it_go = sons.begin();
	for (; it_go != sons.end(); it_go++)
		(*it_go)->PostUpdate();

	std::list<RE_Component*>::iterator it_comp = components.begin();
	for (; it_comp != components.end(); it_comp++)
		(*it_comp)->PostUpdate();
}

bool RE_GameObject::AddChild(RE_GameObject * child)
{
	bool ret = child != nullptr;

	if (ret) sons.push_back(child);

	return ret;
}

bool RE_GameObject::RemoveChild(RE_GameObject * child)
{
	bool ret = child != nullptr;

	if (ret) sons.remove(child);

	return ret;
}

void RE_GameObject::RemoveAllChilds()
{
	while (!sons.empty())
	{
		(*sons.begin())->RemoveAllChilds();
		delete (*sons.begin());
		sons.pop_front();
	}
}

const std::list<RE_GameObject*>* RE_GameObject::GetChilds() const
{
	return &sons;
}

unsigned int RE_GameObject::ChildCount() const
{
	return sons.size();
}

RE_GameObject * RE_GameObject::GetParent() const
{
	return parent;
}

void RE_GameObject::SetParent(RE_GameObject * p)
{
	parent = p;
}

bool RE_GameObject::IsActive() const
{
	return active;
}

void RE_GameObject::SetActive(bool value)
{
	active = value;
}

void RE_GameObject::SetActiveAll(bool value)
{
	active = value;

	std::list<RE_GameObject*>::iterator it_go = sons.begin();
	for (; it_go != sons.end(); it_go++)
		(*it_go)->SetActiveAll(active);
}

RE_Component* RE_GameObject::AddComponent(short unsigned int type, char* file_path_data)
{
	RE_Component* ret = nullptr;

	switch (ComponentType(type))
	{
	case C_TRANSFORM:
	{
		if (transform != nullptr)
		{
			RemoveComponent((RE_Component*)transform);
			transform = nullptr;
		}
		transform = new RE_CompTransform(this);
		ret = (RE_Component*)transform;
		break;
	}
	case C_POINT:
	{
		components.push_back(ret = (RE_Component*)new RE_CompPoint());
		break;
	}
	case C_CUBE:
	{
		//components.push_back(ret = (RE_Component*)new RE_CompCube());
		break;
	}
	case C_MESH:
	{
		components.push_back(ret = (RE_Component*)new RE_CompMesh(file_path_data));
		break;
	}
	}

	return ret;
}

bool RE_GameObject::RemoveComponent(RE_Component * component)
{
	bool ret = component != nullptr;

	if (ret) components.remove(component);

	return ret;
}

void RE_GameObject::RemoveAllComponents()
{
	while (!components.empty())
	{
		delete (*components.begin());
		components.pop_front();
	}
}

void RE_GameObject::TransformModified()
{
	std::list<RE_Component*>::iterator it_comp = components.begin();
	for (; it_comp != components.end(); it_comp++)
		(*it_comp)->OnTransformModified();
}
