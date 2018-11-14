#include "RE_GameObject.h"

#include "Application.h"
#include "ModuleScene.h"
#include "RE_PrimitiveManager.h"
#include "RE_Component.h"
#include "RE_CompTransform.h"
#include "RE_CompPrimitive.h"
#include "RE_CompMesh.h"
#include "RE_CompCamera.h"
#include "OutputLog.h"
#include "SDL2\include\SDL_assert.h"
#include "Glew\include\glew.h"
#include "ImGui\imgui.h"

RE_GameObject::RE_GameObject(const char* name, RE_GameObject * p, bool start_active, bool isStatic)
	: name(name), parent(p), active(start_active), isStatic(isStatic)
{
	if (parent != nullptr) parent->AddChild(this);
	bounding_box.SetFromCenterAndSize(math::vec::zero, math::vec(MIN_AABB_SIZE));
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
	for (auto child : childs) child->PreUpdate();
	for (auto component : components) component->PreUpdate();
}

void RE_GameObject::Update()
{
	for (auto child : childs) child->Update();
	for (auto component : components) component->Update();
}

void RE_GameObject::PostUpdate()
{
	for (auto child : childs) child->PostUpdate();
	for (auto component : components) component->PostUpdate();
}

void RE_GameObject::Draw()
{
	for (auto child : childs) child->Draw();
	for (auto component : components) component->Draw();
}

void RE_GameObject::AddChild(RE_GameObject * child)
{
	SDL_assert(child != nullptr);
	child->parent = this;
	childs.push_back(child);
}

void RE_GameObject::RemoveChild(RE_GameObject * child)
{
	SDL_assert(child != nullptr);
	childs.remove(child);
}

void RE_GameObject::RemoveAllChilds()
{
	while (!childs.empty())
	{
		(*childs.begin())->RemoveAllChilds();
		DEL(*childs.begin());
		childs.pop_front();
	}
}

std::list<RE_GameObject*>& RE_GameObject::GetChilds()
{
	return childs;
}

const std::list<RE_GameObject*>& RE_GameObject::GetChilds() const
{
	return childs;
}


unsigned int RE_GameObject::ChildCount() const { return childs.size(); }

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
	for (auto child : childs) child->SetActiveAll(active);
}

bool RE_GameObject::IsStatic() const
{
	return isStatic;
}

void RE_GameObject::SetStatic(bool value)
{
	isStatic = value;
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

void RE_GameObject::RemoveComponent(RE_Component * component)
{
	SDL_assert(component != nullptr);
	components.remove(component);
}

void RE_GameObject::RemoveAllComponents()
{
	while (!components.empty())
	{
		delete (*components.begin());
		components.pop_front();
	}
}

RE_CompTransform * RE_GameObject::AddCompTransform()
{
	DEL(transform);
	return (transform = new RE_CompTransform(this));
}

RE_CompMesh * RE_GameObject::AddCompMesh(const char * file_path_data, const bool dropped)
{
	RE_CompMesh* ret = new RE_CompMesh(this, file_path_data, dropped);
	components.push_back(ret->AsComponent());
	return ret;
}

void RE_GameObject::AddCompMesh(RE_CompMesh * comp_mesh)
{
	components.push_back((RE_Component*)comp_mesh);
}


RE_CompCamera * RE_GameObject::AddCompCamera()
{
	RE_CompCamera* ret = new RE_CompCamera();
	components.push_back(ret->AsComponent());
	return ret;
}

RE_Component* RE_GameObject::GetComponent(const ushortint type) const
{
	RE_Component* ret = nullptr;

	if (type == ComponentType::C_TRANSFORM)
	{
		ret = transform;
	}
	else
	{
		for (auto component : components)
		{
			if (component->GetType() == type)
			{
				ret = component;
				break;
			}
		}
	}

	return ret;
}

RE_CompTransform * RE_GameObject::GetTransform() const { return transform; }

RE_CompMesh * RE_GameObject::GetMesh() const
{
	RE_CompMesh* ret = nullptr;

	for (auto component : components)
	{
		if (component->GetType() == ComponentType::C_MESH)
		{
			ret = (RE_CompMesh*)component;
			break;
		}
	}

	return ret;
}

RE_CompCamera * RE_GameObject::GetCamera() const
{
	RE_CompCamera* ret = nullptr;

	for (auto component : components)
	{
		if (component->GetType() == ComponentType::C_MESH)
		{
			ret = (RE_CompCamera*)component;
			break;
		}
	}

	return ret;
}

void RE_GameObject::TransformModified()
{
	for (auto component : components)
		component->OnTransformModified();
}

const char * RE_GameObject::GetName() const
{
	return name.c_str();
}

void RE_GameObject::DrawAABB()
{
	glBegin(GL_LINES);
	glLineWidth(3.0f);
	glColor4f(0.00f, 0.00f, 0.80f, 1.00f);

	math::vec position = transform->GetGlobalPosition();

	for (uint i = 0; i < 12; i++)
	{
		glVertex3f(bounding_box.Edge(i).a.x + position.x, bounding_box.Edge(i).a.y + position.y, bounding_box.Edge(i).a.z + position.z);
		glVertex3f(bounding_box.Edge(i).b.x + position.x, bounding_box.Edge(i).b.y + position.y, bounding_box.Edge(i).b.z + position.z);
	}

	glEnd();
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
}

void RE_GameObject::SetBoundingBox(math::AABB box)
{
	bounding_box = box;
}

void RE_GameObject::SetBoundingBoxFromChilds()
{
	for (auto child : childs)
	{
		math::AABB child_box = child->GetBoundingBox();

		// X
		if (child_box.maxPoint.x > bounding_box.maxPoint.x)
			bounding_box.maxPoint.x = child_box.maxPoint.x;
		else if (child_box.minPoint.x < bounding_box.minPoint.x)
			bounding_box.minPoint.x = child_box.minPoint.x;

		// Y
		if (child_box.maxPoint.y > bounding_box.maxPoint.y)
			bounding_box.maxPoint.y = child_box.maxPoint.y;
		else if (child_box.minPoint.y < bounding_box.minPoint.y)
			bounding_box.minPoint.y = child_box.minPoint.y;

		// Z
		if (child_box.maxPoint.z > bounding_box.maxPoint.z)
			bounding_box.maxPoint.z = child_box.maxPoint.z;
		else if (child_box.minPoint.z < bounding_box.minPoint.z)
			bounding_box.minPoint.z = child_box.minPoint.z;
	}
}

math::AABB RE_GameObject::GetBoundingBox() const
{
	return bounding_box;
}

void RE_GameObject::DrawProperties()
{
	/*if (ImGui::BeginMenu("Options"))
	{
		// if (ImGui::MenuItem("Save as prefab")) {}

		ImGui::EndMenu();
	}*/

	char name_holder[64];
	sprintf_s(name_holder, 64, "%s", name.c_str());
	if (ImGui::InputText("Name", name_holder, 64))
		name = name_holder;

	if (ImGui::Checkbox("Active", &active))
		active != active;

	ImGui::SameLine();

	if (ImGui::Checkbox("Static", &isStatic))
		isStatic != isStatic;

	if (ImGui::TreeNode("Bounding Box"))
	{
		ImGui::TextWrapped("Min: { %d, %d, %d}", bounding_box.minPoint.x, bounding_box.minPoint.y, bounding_box.minPoint.z);
		ImGui::TextWrapped("Max: { %d, %d, %d}", bounding_box.maxPoint.x, bounding_box.maxPoint.y, bounding_box.maxPoint.z);

		ImGui::TreePop();
	}

	for (auto component : components)
		component->DrawProperties();
}

void RE_GameObject::DrawHeriarchy()
{
	if (ImGui::TreeNodeEx(name.c_str(), ImGuiTreeNodeFlags_OpenOnArrow + ImGuiTreeNodeFlags_OpenOnDoubleClick))
	{
		if (ImGui::IsItemClicked())
			App->scene->SetSelected(this);

		for (auto child : childs)
			child->DrawHeriarchy();

		ImGui::TreePop();
	}
}
