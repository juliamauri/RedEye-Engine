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
#include "RE_ShaderImporter.h"
#include "ModuleRenderer3D.h"
#include "OutputLog.h"
#include "RE_CameraManager.h"
#include "SDL2\include\SDL_assert.h"
#include "Glew\include\glew.h"
#include "ImGui\imgui.h"
#include <stack>
#include <unordered_set>

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

	RE_CompCamera* comp_camera;
	
	for (RE_Component* cmpGO : go.components)
	{
		switch (cmpGO->GetType())
		{
		case C_TRANSFORM:
			components.push_back(transform = new RE_CompTransform(*(RE_CompTransform*)cmpGO, this));
			break;
		case C_CAMERA:
			comp_camera = new RE_CompCamera(*(RE_CompCamera*)cmpGO, this);
			App->cams->AddMainCamera(comp_camera);
			components.push_back(comp_camera);
			break;
		case C_MESH:
			components.push_back(new RE_CompMesh(*(RE_CompMesh*)cmpGO, this));
			break;
		case C_CUBE:
			components.push_back((RE_CompPrimitive*)new RE_CompCube(*(RE_CompCube*)((RE_CompPrimitive*)cmpGO), this));
			break;
		case C_SPHERE:
			components.push_back((RE_CompPrimitive*)new RE_CompSphere(*(RE_CompSphere*)((RE_CompPrimitive*)cmpGO), this));
			break;
		}
	}

	if (parent != nullptr)
		parent->AddChild(this);

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

void RE_GameObject::DrawWithChilds() const
{
	if (active)
	{
		for (auto component : components)
			component->Draw();

		std::stack<const RE_GameObject*> gos;
		gos.push(this);
		while (!gos.empty())
		{
			const RE_GameObject * go = gos.top();
			gos.pop();

			for (auto component : go->components)
				component->Draw();

			for (auto child : go->GetChilds())
				if (child->IsActive())
					gos.push(child);
		}
	}
}

void RE_GameObject::DrawItselfOnly() const
{
	if (active)
		for (auto component : components)
			component->Draw();
}

std::vector<const char*> RE_GameObject::GetAllResources(bool root)
{
	std::vector<const char*> ret;
	for (auto child : childs) { 

		std::vector<const char*> childRet = child->GetAllResources(false);
		if(!childRet.empty())
			ret.insert(ret.end(), childRet.begin(), childRet.end());
	}

	if (root) { //unique resources
		//https://stackoverflow.com/questions/1041620/whats-the-most-efficient-way-to-erase-duplicates-and-sort-a-vector
		std::unordered_set<const char*> s;
		for (const char* i : ret)
			s.insert(i);
		ret.assign(s.begin(), s.end());
	}

	return ret;
}

void RE_GameObject::SerializeJson(JSONNode * node, std::map<int, const char*>* resources)
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

	float_array.PushBack(GetTransform()->GetLocalPosition().x, fill_node->GetDocument()->GetAllocator()).PushBack(GetTransform()->GetLocalPosition().y, fill_node->GetDocument()->GetAllocator()).PushBack(GetTransform()->GetLocalPosition().z, fill_node->GetDocument()->GetAllocator());
	val_go.AddMember(rapidjson::Value::StringRefType("position"), float_array.Move(), fill_node->GetDocument()->GetAllocator());

	float_array.SetArray();
	float_array.PushBack(GetTransform()->GetLocalEulerRotation().x, fill_node->GetDocument()->GetAllocator()).PushBack(GetTransform()->GetLocalEulerRotation().y, fill_node->GetDocument()->GetAllocator()).PushBack(GetTransform()->GetLocalEulerRotation().z, fill_node->GetDocument()->GetAllocator());
	val_go.AddMember(rapidjson::Value::StringRefType("rotation"), float_array.Move(), fill_node->GetDocument()->GetAllocator());

	float_array.SetArray();
	float_array.PushBack(GetTransform()->GetLocalScale().x, fill_node->GetDocument()->GetAllocator()).PushBack(GetTransform()->GetLocalScale().y, fill_node->GetDocument()->GetAllocator()).PushBack(GetTransform()->GetLocalScale().z, fill_node->GetDocument()->GetAllocator());
	val_go.AddMember(rapidjson::Value::StringRefType("scale"), float_array.Move(), fill_node->GetDocument()->GetAllocator());

	rapidjson::Value val_comp(rapidjson::kArrayType);
	for (auto component : components) component->SerializeJson(node, resources);
	val_go.AddMember(rapidjson::Value::StringRefType("components"), val_comp, fill_node->GetDocument()->GetAllocator());

	node->PushValue(&val_go);

	for (auto child : childs) { child->SerializeJson(node, resources); }
}

void RE_GameObject::SerializeBinary(char*& cursor, std::map<int, const char*>* resources)
{



}

RE_GameObject* RE_GameObject::DeserializeJSON(JSONNode* node, std::map<int, const char*>* resources)
{
	RE_GameObject* rootGo = nullptr;
	RE_GameObject* new_go = nullptr;
	rapidjson::Value* val = rapidjson::Pointer(pointerPath.c_str()).Get(config->document);

	if (val->IsArray())
	{
		for (auto& v : val->GetArray())
		{
			UUID uuid;
			UUID parent_uuid;

			UuidFromStringA((RPC_CSTR)v.FindMember("UUID")->value.GetString(), &uuid);
			if (rootGo != nullptr) UuidFromStringA((RPC_CSTR)v.FindMember("Parent UUID")->value.GetString(), &parent_uuid);
			(rootGo == nullptr) ? rootGo = new_go = new RE_GameObject(v.FindMember("name")->value.GetString(), uuid) : new_go = new RE_GameObject(v.FindMember("name")->value.GetString(), uuid, rootGo->GetGoFromUUID(parent_uuid));

			rapidjson::Value& vector = v.FindMember("position")->value;
			new_go->GetTransform()->SetPosition({ vector.GetArray()[0].GetFloat() , vector.GetArray()[1].GetFloat() , vector.GetArray()[2].GetFloat() });

			vector = v.FindMember("scale")->value;
			new_go->GetTransform()->SetScale({ vector.GetArray()[0].GetFloat() , vector.GetArray()[1].GetFloat() , vector.GetArray()[2].GetFloat() });

			vector = v.FindMember("rotation")->value;
			new_go->GetTransform()->SetRotation({ vector.GetArray()[0].GetFloat() , vector.GetArray()[1].GetFloat() , vector.GetArray()[2].GetFloat() });

			rapidjson::Value& components = v.FindMember("components")->value;
			if (components.IsArray())
			{
				for (auto& c : components.GetArray())
				{
					ComponentType type = (ComponentType)c.FindMember("type")->value.GetInt();

					RE_CompMesh* mesh = nullptr;
					rapidjson::Value* textures_val = nullptr;
					math::vec position = math::vec::zero;
					math::vec scale = math::vec::zero;
					math::vec rotation = math::vec::zero;
					std::string file;
					const char* materialResource = nullptr;
					const char* meshResource = nullptr;
					const char* reference = nullptr;
					switch (type)
					{
					case C_MESH:
						file = c.FindMember("file")->value.GetString();
						reference = c.FindMember("reference")->value.GetString();
						meshResource = App->resources->CheckFileLoaded(file.c_str(), reference, Resource_Type::R_MESH);
						if (meshResource)
						{
							mesh = new RE_CompMesh(new_go, meshResource);
							textures_val = &c.FindMember("material")->value;
							if (textures_val->IsArray())
								for (auto& t : textures_val->GetArray())
								{
									file = t.FindMember("path")->value.GetString();
									reference = t.FindMember("md5")->value.GetString();
									materialResource = App->resources->CheckFileLoaded(file.c_str(), reference, Resource_Type::R_MATERIAL);
									if (materialResource) {
										mesh->SetMaterial(materialResource);
									}
									else {
										LOG_ERROR("Can't Load Material from mesh.\nmd5: %s\nAsset path: ", reference, file.c_str());
									}
								}
							new_go->AddCompMesh(mesh);
						}
						else {
							LOG_ERROR("Can't load mesh from Scene Serialized.\nmd5: %s\nAsset path: %s\n", reference, file.c_str());
						}
						break;
					case C_CAMERA:
						new_go->AddCompCamera(
							c.FindMember("isPrespective")->value.GetBool(),
							c.FindMember("near_plane")->value.GetFloat(),
							c.FindMember("far_plane")->value.GetFloat(),
							c.FindMember("v_fov_rads")->value.GetFloat(),
							c.FindMember("draw_frustum")->value.GetBool());
						break;
					case C_SPHERE:
					{
						RE_CompPrimitive* newSphere = nullptr;
						new_go->AddComponent(newSphere = App->primitives->CreateSphere(new_go, c.FindMember("slices")->value.GetInt(), c.FindMember("stacks")->value.GetInt()));
						vector = c.FindMember("color")->value;
						newSphere->SetColor(vector.GetArray()[0].GetFloat(), vector.GetArray()[1].GetFloat(), vector.GetArray()[2].GetFloat());
					}
					break;
					case C_CUBE:
					{
						RE_CompPrimitive* newCube = nullptr;
						new_go->AddComponent(newCube = App->primitives->CreateCube(new_go));
						vector = c.FindMember("color")->value;
						newCube->SetColor(vector.GetArray()[0].GetFloat(), vector.GetArray()[1].GetFloat(), vector.GetArray()[2].GetFloat());
					}
					break;
					}
				}
			}
		}
	}

	return rootGo;
}

RE_GameObject* RE_GameObject::DeserializeBinary(JSONNode* node, std::map<int, const char*>* resources)
{
	return nullptr;
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

void RE_GameObject::IterativeSetActive(bool val)
{
	bool tmp = active;
	std::stack<RE_GameObject*> gos;
	gos.push(this);

	while (!gos.empty())
	{
		RE_GameObject * go = gos.top();
		go->active = val;
		gos.pop();

		for (auto child : go->childs)
			gos.push(child);
	}

	active = tmp;
}

void RE_GameObject::IterativeSetStatic(bool val)
{
	bool tmp = isStatic;
	std::stack<RE_GameObject*> gos;
	gos.push(this);

	while (!gos.empty())
	{
		RE_GameObject * go = gos.top();
		go->isStatic = val;
		gos.pop();

		for (auto child : go->childs)
			gos.push(child);
	}

	isStatic = tmp;
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

RE_CompCamera * RE_GameObject::AddCompCamera(bool prespective, float near_plane, float far_plane, float v_fov_rads, short aspect_ratio_t, bool draw_frustum)
{
	RE_CompCamera* comp_camera = new RE_CompCamera(this, prespective, near_plane, far_plane, v_fov_rads, aspect_ratio_t, draw_frustum);
	components.push_back((RE_Component*)comp_camera);
	App->cams->AddMainCamera(comp_camera);
	return comp_camera;
}

void RE_GameObject::AddComponent(RE_Component * component)
{
	components.push_back(component);
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
		RE_CompMesh* mesh_comp = new RE_CompMesh(this, file_path_data, drop);
		//AddToBoundingBox(mesh_comp->GetAABB());
		ret = (RE_Component*)mesh_comp;
	}
	else if (ComponentType(type) == C_CAMERA)
	{
		RE_CompCamera* comp_camera = new RE_CompCamera(this);
		App->cams->AddMainCamera(comp_camera);
		ret = (RE_Component*)comp_camera;
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
	//AddToBoundingBox(ret->GetAABB());
	components.push_back(ret->AsComponent());
	return ret;
}

void RE_GameObject::AddCompMesh(RE_CompMesh * comp_mesh)
{
	//AddToBoundingBox(comp_mesh->GetAABB());
	components.push_back((RE_Component*)comp_mesh);
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
		if (component->GetType() == ComponentType::C_CAMERA)
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
	if (isStatic)
		App->scene->StaticTransformed();

	ResetGlobalBoundingBox();

	for (auto component : components)
		component->OnTransformModified();

	for (auto child : childs)
		child->TransformModified();
}

const char * RE_GameObject::GetName() const
{
	return name.c_str();
}

void RE_GameObject::DrawAABB(math::vec color)
{
	RE_ShaderImporter::use(0);

	glMatrixMode(GL_MODELVIEW);
	glLoadMatrixf((transform->GetMatrixModel() * RE_CameraManager::CurrentCamera()->GetView()).ptr());

	glColor3f(color.x, color.y, color.z);
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
}

void RE_GameObject::DrawGlobalAABB()
{
	for (uint i = 0; i < 12; i++)
	{
		glVertex3f(
			global_bounding_box.Edge(i).a.x,
			global_bounding_box.Edge(i).a.y,
			global_bounding_box.Edge(i).a.z);
		glVertex3f(
			global_bounding_box.Edge(i).b.x,
			global_bounding_box.Edge(i).b.y,
			global_bounding_box.Edge(i).b.z);
	}
}

void RE_GameObject::DrawAllAABB()
{
	if (active)
	{
		if (App->scene->GetSelected() == this)
		{
			if (!App->scene->DrawingSelAABB())
				DrawGlobalAABB();
		}
		else if (parent != nullptr)
			DrawGlobalAABB();

		for (auto child : childs)
			child->DrawAllAABB();
	}
}

void RE_GameObject::AddToBoundingBox(math::AABB box)
{
	if (transform != nullptr)
		local_bounding_box.Enclose(box);
}

void RE_GameObject::ResetBoundingBoxFromChilds()
{
	// Local Bounding Box
	local_bounding_box.SetFromCenterAndSize(math::vec::zero, math::vec::one * 0.1f);

	for (RE_Component* comp : components)
	{
		switch (comp->GetType())
		{
		case C_MESH: local_bounding_box = GetMesh()->GetAABB(); break;
		case C_SPHERE: local_bounding_box = math::AABB(-math::vec::one, math::vec::one); break;
		case C_CUBE: local_bounding_box = math::AABB(math::vec::zero, math::vec::one); break;
		}
	}

	if (!childs.empty())
	{
		// Create vector to store all contained points
		unsigned int cursor = 0;
		std::vector<math::vec> points;
		points.resize(2 + (childs.size() * 2));

		// Store local mesh AABB max and min points
		points[cursor++].Set(local_bounding_box.minPoint.x, local_bounding_box.minPoint.y, local_bounding_box.minPoint.z);
		points[cursor++].Set(local_bounding_box.maxPoint.x, local_bounding_box.maxPoint.y, local_bounding_box.maxPoint.z);

		// Store child AABBs max and min points
		for (std::list<RE_GameObject*>::iterator child = childs.begin(); child != childs.end(); child++)
		{
			// Update child AABB
			(*child)->ResetBoundingBoxFromChilds();

			math::AABB child_aabb = (*child)->GetLocalBoundingBox();
			child_aabb.TransformAsAABB((*child)->transform->GetLocalMatrixModel().Transposed());

			points[cursor++].Set(child_aabb.minPoint.x, child_aabb.minPoint.y, child_aabb.minPoint.z);
			points[cursor++].Set(child_aabb.maxPoint.x, child_aabb.maxPoint.y, child_aabb.maxPoint.z);
		}

		// Enclose stored points
		local_bounding_box.SetFrom(&points[0], points.size());
	}

	ResetGlobalBoundingBox();
}

void RE_GameObject::ResetGlobalBoundingBox()
{
	// Global Bounding Box
	global_bounding_box = local_bounding_box;
	global_bounding_box.TransformAsAABB(transform->GetMatrixModel().Transposed());
}

math::AABB RE_GameObject::GetLocalBoundingBox() const
{
	return local_bounding_box;
}

math::AABB RE_GameObject::GetGlobalBoundingBox() const
{
	return global_bounding_box;
}

void RE_GameObject::DrawProperties()
{
	if (ImGui::BeginMenu("Child Options"))
	{
		// if (ImGui::MenuItem("Save as prefab")) {}

		if (ImGui::MenuItem("Activate Childs"))
			IterativeSetActive(true);
		if (ImGui::MenuItem("Deactivate Childs"))
			IterativeSetActive(false);

		if (ImGui::MenuItem("Childs to Static"))
			IterativeSetStatic(true);
		if (ImGui::MenuItem("Childs to non-Static"))
			IterativeSetStatic(false);

		ImGui::EndMenu();
	}

	char name_holder[64];
	sprintf_s(name_holder, 64, "%s", name.c_str());
	if (ImGui::InputText("Name", name_holder, 64))
		name = name_holder;

	ImGui::Checkbox("Active", &active);

	ImGui::SameLine();

	if (ImGui::Checkbox("Static", &isStatic))
		App->scene->StaticTransformed();

	if (ImGui::TreeNode("Local Bounding Box"))
	{
		ImGui::TextWrapped("Min: { %.2f, %.2f, %.2f}", local_bounding_box.minPoint.x, local_bounding_box.minPoint.y, local_bounding_box.minPoint.z);
		ImGui::TextWrapped("Max: { %.2f, %.2f, %.2f}", local_bounding_box.maxPoint.x, local_bounding_box.maxPoint.y, local_bounding_box.maxPoint.z);

		ImGui::TreePop();
	}
	if (ImGui::TreeNode("Global Bounding Box"))
	{
		ImGui::TextWrapped("Min: { %.2f, %.2f, %.2f}", global_bounding_box.minPoint.x, global_bounding_box.minPoint.y, global_bounding_box.minPoint.z);
		ImGui::TextWrapped("Max: { %.2f, %.2f, %.2f}", global_bounding_box.maxPoint.x, global_bounding_box.maxPoint.y, global_bounding_box.maxPoint.z);

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
