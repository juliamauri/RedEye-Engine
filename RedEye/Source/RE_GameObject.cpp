#include "RE_GameObject.h"

#include "Application.h"
#include "ModuleScene.h"
#include "RE_PrimitiveManager.h"
#include "RE_Component.h"
#include "RE_CompTransform.h"
#include "RE_CompPrimitive.h"
#include "RE_CompMesh.h"
#include "OutputLog.h"
#include "SDL2\include\SDL_assert.h"

RE_GameObject::RE_GameObject(RE_GameObject * p, const bool start_active) : active(start_active)
{
	parent = (p);// != nullptr) ? p : App->scene->GetRoot();
	bounding_box.minPoint = bounding_box.maxPoint = math::vec::zero;
	transform = new RE_CompTransform(this);
	components.push_back((RE_Component*)transform);
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

void RE_GameObject::Draw()
{
	std::list<RE_GameObject*>::iterator it_go = sons.begin();
	for (; it_go != sons.end(); it_go++)
		(*it_go)->Draw();

	std::list<RE_Component*>::iterator it_comp = components.begin();
	for (; it_comp != components.end(); it_comp++)
		(*it_comp)->Draw();
}

bool RE_GameObject::AddChild(RE_GameObject * child)
{
	bool ret = (child != nullptr);

	if (ret)
	{
		child->parent = this;
		sons.push_back(child);
	}

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

const std::list<RE_GameObject*>* RE_GameObject::GetChilds() const { return &sons; }

unsigned int RE_GameObject::ChildCount() const { return sons.size(); }

RE_GameObject * RE_GameObject::GetParent() const { return parent; }

void RE_GameObject::SetParent(RE_GameObject * p)
{
	SDL_assert(p != nullptr);
	parent = p;
}

bool RE_GameObject::IsActive() const { return active; }

void RE_GameObject::SetActive(bool value) { active = value; }

void RE_GameObject::SetActiveAll(bool value)
{
	active = value;

	std::list<RE_GameObject*>::iterator it_go = sons.begin();
	for (; it_go != sons.end(); it_go++)
		(*it_go)->SetActiveAll(active);
}

RE_Component* RE_GameObject::AddComponent(const ushortint type, const char* file_path_data, const bool drop)
{
	SDL_assert(type < MAX_COMPONENT_TYPES);
	RE_Component* ret = nullptr;

	if (ComponentType(type) == C_TRANSFORM)
	{
		if (transform != nullptr)
		{
			RemoveComponent((RE_Component*)transform);
			transform = nullptr;
		}
		transform = new RE_CompTransform(this);
		ret = (RE_Component*)transform;
	}
	else if (App->primitives)
	{
		switch (ComponentType(type))
		{
		case C_AXIS:
		{
			ret = (RE_Component*)(App->primitives->CreateAxis(this));
			break;
		}
		case C_POINT:
		{
			ret = (RE_Component*)(App->primitives->CreatePoint(this, math::vec::zero));
			break;
		}
		case C_LINE:
		{
			ret = (RE_Component*)(App->primitives->CreateLine(this, math::vec::zero, math::vec::one));
			break;
		}
		case C_RAY:
		{
			ret = (RE_Component*)(App->primitives->CreateRay(this));
			break;
		}
		case C_TRIANGLE:
		{
			ret = (RE_Component*)(App->primitives->CreateTriangle(this));
			break;
		}
		case C_PLANE:
		{
			ret = (RE_Component*)(App->primitives->CreatePlane(this));
			break;
		}
		case C_CUBE:
		{
			ret = (RE_Component*)(App->primitives->CreateCube(this));
			break;
		}
		case C_FUSTRUM:
		{
			ret = (RE_Component*)(App->primitives->CreateFustrum(this));
			break;
		}
		case C_SPHERE:
		{
			ret = (RE_Component*)(App->primitives->CreateSphere(this));
			break;
		}
		case C_CYLINDER:
		{
			ret = (RE_Component*)(App->primitives->CreateCylinder(this));
			break;
		}
		case C_CAPSULE:
		{
			ret = (RE_Component*)(App->primitives->CreateCapsule(this));
			break;
		}
		default:
			LOG_ERROR("Component of type %u is unsupported", type);
		}
	}
	else if (ComponentType(type) == C_MESH)
	{
		ret = (RE_Component*)new RE_CompMesh(this, file_path_data, drop);
	}

	if (ret != nullptr)
		components.push_back(ret);
	else
		LOG_ERROR("GameObject could not add type %u component", type);

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

RE_Component* RE_GameObject::GetComponent(const ushortint type) const
{
	RE_Component* ret = nullptr;

	if (ComponentType(type) == C_TRANSFORM)
	{
		ret = transform;
	}
	else
	{
		std::list<RE_Component*>::const_iterator it_comp = components.cbegin();
		for (; it_comp != components.cend() && !ret; it_comp++)
		{
			if ((*it_comp)->GetType() == ComponentType(type))
				ret = (*it_comp);
		}
	}

	return ret;
}

RE_CompTransform * RE_GameObject::GetTransform() const { return transform; }

void RE_GameObject::TransformModified()
{
	std::list<RE_Component*>::iterator it_comp = components.begin();
	for (; it_comp != components.end(); it_comp++)
		(*it_comp)->OnTransformModified();
}
