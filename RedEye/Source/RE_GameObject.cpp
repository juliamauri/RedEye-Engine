#include "RE_GameObject.h"

#include "Application.h"
#include "ModuleScene.h"
#include "FileSystem.h"
#include "RE_PrimitiveManager.h"
#include "RE_Component.h"
#include "RE_CompTransform.h"
#include "RE_CompPrimitive.h"
#include "RE_CompMesh.h"
#include "RE_CompCamera.h"
#include "RE_CompParticleEmiter.h"
#include "ShaderManager.h"
#include "ModuleEditor.h"
#include "OutputLog.h"
#include "SDL2\include\SDL_assert.h"
#include "Glew\include\glew.h"
#include "ImGui\imgui.h"

RE_GameObject::RE_GameObject(const char* name, UUID uuid, RE_GameObject * p, bool start_active, bool isStatic)
	: name(name), parent(p), active(start_active), isStatic(isStatic)
{
	if (uuid == GUID_NULL)
		UuidCreate(&this->uuid);
	else
		this->uuid = uuid;

	transform = new RE_CompTransform(this);
	components.push_back((RE_Component*)transform);

	local_bounding_box.SetFromCenterAndSize(math::vec::zero, math::vec::zero);
	global_bounding_box.SetFromCenterAndSize(math::vec::zero, math::vec::zero);

	if (parent != nullptr)
		parent->AddChild(this);
}

RE_GameObject::RE_GameObject(const RE_GameObject & go, RE_GameObject * p) : parent(p)
{
	UuidCreate(&this->uuid);
	isStatic = go.isStatic;
	active = go.active;
	name = go.name;

	local_bounding_box = go.local_bounding_box;
	global_bounding_box = go.global_bounding_box;

	if (parent != nullptr)
		parent->AddChild(this);
	
	for (RE_Component* cmpGO : go.components)
	{
		switch (cmpGO->GetType())
		{
		case C_TRANSFORM:
			components.push_back(transform = new RE_CompTransform(*(RE_CompTransform*)cmpGO, this));
			break;
		case C_CAMERA:
			components.push_back(new RE_CompCamera(*(RE_CompCamera*)cmpGO, this));
			break;
		case C_MESH:
			components.push_back(new RE_CompMesh(*(RE_CompMesh*)cmpGO, this));
			break;
		}
	}

	if (!go.childs.empty()) {
		for (RE_GameObject* childGO : go.childs) {
			new RE_GameObject(*childGO, this);
		}
	}
}

RE_GameObject::~RE_GameObject()
{
	RemoveAllChilds();
	RemoveAllComponents();
}

void RE_GameObject::PreUpdate()
{
	for (auto component : components) component->PreUpdate();
	for (auto child : childs) child->PreUpdate();
}

void RE_GameObject::Update()
{
	for (auto component : components) component->Update();
	for (auto child : childs) child->Update();
}

void RE_GameObject::PostUpdate()
{
	for (auto component : components) component->PostUpdate();
	for (auto child : childs) child->PostUpdate();
}

void RE_GameObject::Draw(bool recursive)
{
	if (recursive)
		for (auto child : childs) child->Draw();

	for (auto component : components) component->Draw();
}

void RE_GameObject::Serialize(JSONNode * node)
{
	JSONNode* fill_node = node;

	rapidjson::Value val_go(rapidjson::kObjectType);

	val_go.AddMember(rapidjson::Value::StringRefType("name"), rapidjson::Value().SetString(GetName(), fill_node->GetDocument()->GetAllocator()), fill_node->GetDocument()->GetAllocator());

	char* str = nullptr;
	UuidToStringA(&uuid, (RPC_CSTR*)&str);
	val_go.AddMember(rapidjson::Value::StringRefType("UUID"), rapidjson::Value().SetString(str, fill_node->GetDocument()->GetAllocator()), fill_node->GetDocument()->GetAllocator());
	RpcStringFreeA((RPC_CSTR*)&str);

	if (parent != nullptr)
	{
		UuidToStringA(&parent->uuid, (RPC_CSTR*)&str);
		val_go.AddMember(rapidjson::Value::StringRefType("Parent UUID"), rapidjson::Value().SetString(str, fill_node->GetDocument()->GetAllocator()), fill_node->GetDocument()->GetAllocator());
		RpcStringFreeA((RPC_CSTR*)&str);
	}

	rapidjson::Value float_array(rapidjson::kArrayType);

	float_array.PushBack(GetTransform()->GetPosition().x, fill_node->GetDocument()->GetAllocator()).PushBack(GetTransform()->GetPosition().y, fill_node->GetDocument()->GetAllocator()).PushBack(GetTransform()->GetPosition().z, fill_node->GetDocument()->GetAllocator());
	val_go.AddMember(rapidjson::Value::StringRefType("position"), float_array.Move(), fill_node->GetDocument()->GetAllocator());

	float_array.SetArray();
	float_array.PushBack(GetTransform()->GetEulerRotation().x, fill_node->GetDocument()->GetAllocator()).PushBack(GetTransform()->GetEulerRotation().y, fill_node->GetDocument()->GetAllocator()).PushBack(GetTransform()->GetEulerRotation().z, fill_node->GetDocument()->GetAllocator());
	val_go.AddMember(rapidjson::Value::StringRefType("rotation"), float_array.Move(), fill_node->GetDocument()->GetAllocator());

	float_array.SetArray();
	float_array.PushBack(GetTransform()->GetScale().x, fill_node->GetDocument()->GetAllocator()).PushBack(GetTransform()->GetScale().y, fill_node->GetDocument()->GetAllocator()).PushBack(GetTransform()->GetScale().z, fill_node->GetDocument()->GetAllocator());
	val_go.AddMember(rapidjson::Value::StringRefType("scale"), float_array.Move(), fill_node->GetDocument()->GetAllocator());

	rapidjson::Value val_comp(rapidjson::kArrayType);
	for (auto component : components) component->Serialize(node, &val_comp);
	val_go.AddMember(rapidjson::Value::StringRefType("components"), val_comp, fill_node->GetDocument()->GetAllocator());

	node->PushValue(&val_go);

	for (auto child : childs) { child->Serialize(node); }
}

void RE_GameObject::AddChild(RE_GameObject * child)
{
	SDL_assert(child != nullptr);
	child->parent = this;
	child->AddToBoundingBox(math::AABB(math::vec::zero, math::vec::zero));
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

void RE_GameObject::OnPlay()
{
	for (auto component : components) component->OnPlay();
	for (auto child : childs) child->OnPlay();
}

void RE_GameObject::OnPause()
{
	for (auto component : components) component->OnPause();
	for (auto child : childs) child->OnPause();
}

void RE_GameObject::OnStop()
{
	for (auto component : components) component->OnStop();
	for (auto child : childs) child->OnStop();
}

RE_CompCamera * RE_GameObject::AddCompCamera(bool prespective, float near_plane, float far_plane, float h_fov_rads, float v_fov_rads, float h_fov_degrees, float v_fov_degrees, math::vec position, math::vec rotation, math::vec scale)
{
	RE_CompCamera* comp_camera = new RE_CompCamera(this, prespective, near_plane, far_plane, h_fov_rads, v_fov_rads, h_fov_degrees, v_fov_degrees, position, rotation, scale);
	components.push_back((RE_Component*)comp_camera);
	return comp_camera;
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
	else if (ComponentType(type) == C_MESH)
	{
		ret = (RE_Component*)new RE_CompMesh(this, file_path_data, drop);
	}
	else if (ComponentType(type) == C_CAMERA)
	{
		ret = (RE_Component*)new RE_CompCamera(this);
	}
	else if (ComponentType(type) == C_PARTICLEEMITER)
	{
		ret = (RE_Component*)new RE_CompParticleEmitter(this);
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
		DEL(*components.begin());
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

RE_GameObject * RE_GameObject::GetGoFromUUID(UUID parent)
{
	RE_GameObject* ret = nullptr;

	if (uuid == parent)
		ret = this;
	else
		for (auto child : childs)
		{
			ret = child->GetGoFromUUID(parent);
			if (ret)
				break;
		}

	return ret;
}

void RE_GameObject::TransformModified()
{
	for (auto component : components)
		component->OnTransformModified();

	for (auto child : childs)
		child->TransformModified();
}

const char * RE_GameObject::GetName() const
{
	return name.c_str();
}

void RE_GameObject::DrawAABB()
{
	ShaderManager::use(0);

	glColor3f(0.0f, 0.0f, 1.0f);

	math::float4x4 model = transform->GetMatrixModel();
	RE_CompCamera* camera = App->editor->GetCamera();

	glMatrixMode(GL_MODELVIEW);
	glLoadMatrixf((model * camera->GetView()).ptr());

	glLineWidth(5.0f);

	glBegin(GL_LINES);

	for (uint i = 0; i < 12; i++)
	{
		glVertex3f(
			local_bounding_box.Edge(i).a.x,
			local_bounding_box.Edge(i).a.y,
			local_bounding_box.Edge(i).a.z);
		glVertex3f(
			local_bounding_box.Edge(i).b.x,
			local_bounding_box.Edge(i).b.y,
			local_bounding_box.Edge(i).b.z);
	}

	glEnd();

	glLineWidth(1.0f);
}

void RE_GameObject::DrawAllAABB()
{
	DrawAABB();

	for (auto child : childs)
		child->DrawAllAABB();
}

void RE_GameObject::AddToBoundingBox(math::AABB box)
{
	if (transform != nullptr)
		return;
	local_bounding_box = math::AABB(
		math::vec(
			local_bounding_box.MinX() <= box.MinX() ? local_bounding_box.MinX() : box.MinX(),
			local_bounding_box.MinY() <= box.MinY() ? local_bounding_box.MinY() : box.MinY(),
			local_bounding_box.MinZ() <= box.MinZ() ? local_bounding_box.MinZ() : box.MinZ()),
		math::vec(
			local_bounding_box.MaxX() >= box.MaxX() ? local_bounding_box.MaxX() : box.MaxX(),
			local_bounding_box.MaxY() >= box.MaxY() ? local_bounding_box.MaxY() : box.MaxY(),
			local_bounding_box.MaxZ() >= box.MaxZ() ? local_bounding_box.MaxZ() : box.MaxZ()));

	math::float4x4 global_mat = transform->GetMatrixModel();
	global_bounding_box.maxPoint = global_mat.MulPos(local_bounding_box.maxPoint);
	global_bounding_box.minPoint = global_mat.MulPos(local_bounding_box.minPoint);

	if (parent != nullptr)
	{
		math::float4x4 trs = transform->GetLocalMatrixModel();

		parent->AddToBoundingBox(math::AABB(
			trs.MulPos(local_bounding_box.maxPoint),
			trs.MulPos(local_bounding_box.minPoint)));
	}
}

void RE_GameObject::SetBoundingBoxFromChilds()
{
	global_bounding_box.SetFromCenterAndSize(math::vec::zero, math::vec::zero);

	for (auto child : childs)
	{
		math::AABB child_box = child->GetGlobalBoundingBox();

		// X
		if (child_box.maxPoint.x > global_bounding_box.maxPoint.x)
			global_bounding_box.maxPoint.x = child_box.maxPoint.x;
		else if (child_box.minPoint.x < global_bounding_box.minPoint.x)
			global_bounding_box.minPoint.x = child_box.minPoint.x;

		// Y
		if (child_box.maxPoint.y > global_bounding_box.maxPoint.y)
			global_bounding_box.maxPoint.y = child_box.maxPoint.y;
		else if (child_box.minPoint.y < global_bounding_box.minPoint.y)
			global_bounding_box.minPoint.y = child_box.minPoint.y;

		// Z
		if (child_box.maxPoint.z > global_bounding_box.maxPoint.z)
			global_bounding_box.maxPoint.z = child_box.maxPoint.z;
		else if (child_box.minPoint.z < global_bounding_box.minPoint.z)
			global_bounding_box.minPoint.z = child_box.minPoint.z;
	}

	math::float4x4 global_mat = transform->GetMatrixModel();

	local_bounding_box.maxPoint = global_mat.MulPos(global_bounding_box.maxPoint);
	local_bounding_box.minPoint = global_mat.MulPos(global_bounding_box.minPoint);
}

math::AABB RE_GameObject::GetGlobalBoundingBox() const
{
	return global_bounding_box;
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
		ImGui::TextWrapped("Min: { %.2f, %.2f, %.2f}", local_bounding_box.minPoint.x, local_bounding_box.minPoint.y, local_bounding_box.minPoint.z);
		ImGui::TextWrapped("Max: { %.2f, %.2f, %.2f}", local_bounding_box.maxPoint.x, local_bounding_box.maxPoint.y, local_bounding_box.maxPoint.z);

		ImGui::TreePop();
	}

	for (auto component : components)
		component->DrawProperties();
}

void RE_GameObject::DrawHeriarchy()
{
	if (ImGui::TreeNodeEx(name.c_str(),
		(App->scene->GetSelected() == this ?
			ImGuiTreeNodeFlags_(childs.empty() ?
				ImGuiTreeNodeFlags_Selected | ImGuiTreeNodeFlags_Leaf :
				ImGuiTreeNodeFlags_Selected | ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick) :
			ImGuiTreeNodeFlags_(childs.empty() ?
				ImGuiTreeNodeFlags_None | ImGuiTreeNodeFlags_Leaf :
				ImGuiTreeNodeFlags_None | ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick))))
	{
		if (ImGui::IsItemClicked(0))
			App->scene->SetSelected(this);

		for (auto child : childs)
			child->DrawHeriarchy();

		ImGui::TreePop();
	}
	else if (ImGui::IsItemClicked(0))
		App->scene->SetSelected(this);
}
