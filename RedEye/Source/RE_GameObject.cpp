#include "RE_GameObject.h"

#include "Application.h"
#include "ModuleScene.h"
#include "RE_FileSystem.h"
#include "RE_PrimitiveManager.h"
#include "RE_Component.h"
#include "RE_CompTransform.h"
#include "RE_CompPrimitive.h"
#include "RE_CompMesh.h"
#include "RE_CompCamera.h"
#include "RE_CompParticleEmiter.h"
#include "RE_ShaderImporter.h"
#include "RE_GLCache.h"
#include "ModuleRenderer3D.h"
#include "ModuleEditor.h"
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
		parent->AddChild(this, false);
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
			if(!Event::isPaused()) App->cams->AddMainCamera(comp_camera);
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
		case C_PLANE:
			components.push_back((RE_CompPrimitive*)new RE_CompPlane(*(RE_CompPlane*)((RE_CompPrimitive*)cmpGO), this));
			break;
		}
	}

	if (parent != nullptr)
		parent->AddChild(this, false);

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

std::stack<RE_Component*> RE_GameObject::GetDrawableComponentsWithChilds(RE_GameObject* ignoreStencil) const
{
	std::stack<RE_Component*> ret;

	if (active)
	{
		std::stack<const RE_GameObject*> gos;
		gos.push(this);
		while (!gos.empty())
		{
			const RE_GameObject* go = gos.top();
			gos.pop();
			
			if (go != ignoreStencil) {
				for (auto component : go->components)
				{
					ComponentType cT = component->GetType();
					if (cT == ComponentType::C_MESH || (cT > ComponentType::C_PRIMIVE_MIN&& cT < ComponentType::C_PRIMIVE_MAX)) ret.push(component);
				}
			}

			for (auto child : go->GetChilds())
				if (child->IsActive())
					gos.push(child);
		}
	}
	return ret;
}

std::stack<RE_Component*> RE_GameObject::GetDrawableComponentsItselfOnly() const
{
	std::stack<RE_Component*> ret;
	for (auto component : components)
	{
		ComponentType cT = component->GetType();
		if (cT == ComponentType::C_MESH || (cT > ComponentType::C_PRIMIVE_MIN && cT < ComponentType::C_PRIMIVE_MAX)) ret.push(component);
	}
	return ret;
}

std::vector<RE_GameObject*> RE_GameObject::GetAllGO()
{
	std::vector<RE_GameObject*> ret;
	ret.push_back(this);
	for (auto child : childs) {

		std::vector<RE_GameObject*> childRet = child->GetAllGO();
		if (!childRet.empty())
			ret.insert(ret.end(), childRet.begin(), childRet.end());
	}
	return ret;
}

std::vector<const char*> RE_GameObject::GetAllResources(bool root)
{
	std::vector<const char*> allResources;

	for (auto comp : components) {
		std::vector<const char*> cmpRet = comp->GetAllResources();
		if (!cmpRet.empty())
			allResources.insert(allResources.end(), cmpRet.begin(), cmpRet.end());
	}

	for (auto child : childs) {

		std::vector<const char*> childRet = child->GetAllResources(false);
		if(!childRet.empty())
			allResources.insert(allResources.end(), childRet.begin(), childRet.end());
	}

	if (root) { //unique resources
		std::vector<const char*> ret;

		for (auto res : allResources) {
			bool repeat = false;
			for (auto uniqueRes : ret) {
				if (std::strcmp(res, uniqueRes) == 0) {
					repeat = true;
					break;
				}
			}
			if (!repeat) ret.push_back(res);
		}

		return ret;
	}

	return allResources;
}

void RE_GameObject::SerializeJson(JSONNode * node, std::map<const char*, int>* resources)
{
	std::vector<RE_GameObject*> allGOs = GetAllGO();
	JSONNode* gameObjects = node->PushJObject("gameobjects");
	gameObjects->PushUInt("gameobjectsSize", allGOs.size());

	uint count = 0;
	std::string ref;
	for (RE_GameObject* go : allGOs) {
		ref = "go" + std::to_string(count++);
		JSONNode* goNode = gameObjects->PushJObject(ref.c_str());

		goNode->PushString("name", go->GetName());

		char* str = nullptr;
		UuidToStringA(&go->uuid, (RPC_CSTR*)&str);
		goNode->PushString("UUID", str);
		RpcStringFreeA((RPC_CSTR*)&str);

		if (go->parent != nullptr)
		{
			UuidToStringA(&go->parent->uuid, (RPC_CSTR*)&str);
			goNode->PushString("Parent UUID", str);
			RpcStringFreeA((RPC_CSTR*)&str);
		}

		goNode->PushFloatVector("position", go->GetTransform()->GetLocalPosition());
		goNode->PushFloatVector("rotation", go->GetTransform()->GetLocalEulerRotation());
		goNode->PushFloatVector("scale", go->GetTransform()->GetLocalScale());

		JSONNode* comps = goNode->PushJObject("components");
		uint cmpSize = 0;
		for (auto component : go->components) {
			unsigned short type = component->GetType();
			if (type == C_CUBE || type == C_PLANE || type == C_SPHERE || type == C_MESH || type == C_CAMERA)
				cmpSize++;
		}
		comps->PushUInt("ComponentsSize", cmpSize);
		uint count = 0;
		for (auto component : go->components) {
			unsigned short type = component->GetType();
			if (type != C_CUBE && type != C_PLANE && type != C_SPHERE && type != C_MESH && type != C_CAMERA)
				continue;
			ref = "cmp" + std::to_string(count++);
			JSONNode* comp = comps->PushJObject(ref.c_str());
			comp->PushInt("type", type);
			component->SerializeJson(comp, resources);
			DEL(comp);
		}
		DEL(comps);
		DEL(goNode);
	}
	DEL(gameObjects);
}

unsigned int RE_GameObject::GetBinarySize()const
{
	uint size = sizeof(uint) * 3 + 36 * sizeof(char) + sizeof(float) * 9 + sizeof(unsigned short);
	if (parent != nullptr) size += 36 * sizeof(char);
	size += std::strlen(GetName()) * sizeof(char);

	for (auto component : components) size += component->GetBinarySize();

	for (auto child : childs) size += child->GetBinarySize();

	return size;
}

void RE_GameObject::SerializeBinary(char*& cursor, std::map<const char*, int>* resources)
{
	std::vector<RE_GameObject*> allGOs = GetAllGO();

	size_t size = sizeof(uint);
	uint goSize = allGOs.size();
	memcpy(cursor, &goSize, size);
	cursor += size;

	for (RE_GameObject* go : allGOs) {
		uint strLenght = std::strlen(go->GetName());
		size = sizeof(uint);
		memcpy(cursor, &strLenght, size);
		cursor += size;

		size = sizeof(char) * strLenght;
		memcpy(cursor, go->GetName(), size);
		cursor += size;

		size = sizeof(char) * 36;
		char* strUUID = nullptr;
		UuidToStringA(&go->uuid, (RPC_CSTR*)&strUUID);
		memcpy(cursor, strUUID, size);
		cursor += size;
		RpcStringFreeA((RPC_CSTR*)&strUUID);

		if (go->parent != nullptr)
		{
			UuidToStringA(&go->parent->uuid, (RPC_CSTR*)&strUUID);
			memcpy(cursor, strUUID, size);
			cursor += size;
			RpcStringFreeA((RPC_CSTR*)&strUUID);
		}

		size = sizeof(float) * 3;
		memcpy(cursor, &go->GetTransform()->GetLocalPosition()[0], size);
		cursor += size;

		memcpy(cursor, &go->GetTransform()->GetLocalEulerRotation()[0], size);
		cursor += size;

		memcpy(cursor, &go->GetTransform()->GetLocalScale()[0], size);
		cursor += size;

		size = sizeof(uint);
		uint cmpSize = 0;
		for (auto component : go->components) {
			unsigned short type = component->GetType();
			if (type == C_CUBE || type == C_PLANE || type == C_SPHERE || type == C_MESH || type == C_CAMERA)
				cmpSize++;
		}
		memcpy(cursor, &cmpSize, size);
		cursor += size;

		for (auto component : go->components) {
			unsigned short type = component->GetType();
			if (type != C_CUBE && type != C_PLANE && type != C_SPHERE && type != C_MESH && type != C_CAMERA)
				continue;
			size = sizeof(unsigned short);
			memcpy(cursor, &type, size);
			cursor += size;

			component->SerializeBinary(cursor, resources);
		}
	}
}

RE_GameObject* RE_GameObject::DeserializeJSON(JSONNode* node, std::map<int, const char*>* resources)
{
	RE_GameObject* rootGo = nullptr;
	RE_GameObject* new_go = nullptr;
	JSONNode* gameObjects = node->PullJObject("gameobjects");
	uint GOCount = gameObjects->PullUInt("gameobjectsSize", 0);

	std::string ref;
	for (uint count = 0; count < GOCount; count++) {
		ref = "go" + std::to_string(count);
		JSONNode* goNode = gameObjects->PullJObject(ref.c_str());

		UUID uuid;
		UUID parent_uuid;

		UuidFromStringA((RPC_CSTR)goNode->PullString("UUID", ""), &uuid);
		if (rootGo != nullptr) UuidFromStringA((RPC_CSTR)goNode->PullString("Parent UUID",""), &parent_uuid);
		(rootGo == nullptr) ? rootGo = new_go = new RE_GameObject(goNode->PullString("name", "GameObject"), uuid) : new_go = new RE_GameObject(goNode->PullString("name", "GameObject"), uuid, rootGo->GetGoFromUUID(parent_uuid));

		new_go->GetTransform()->SetPosition(goNode->PullFloatVector("position", math::vec::zero));
		new_go->GetTransform()->SetRotation(goNode->PullFloatVector("rotation", math::vec::zero));
		new_go->GetTransform()->SetScale(goNode->PullFloatVector("scale", math::vec::one));

		JSONNode* comps = goNode->PullJObject("components");
		uint compCount = comps->PullUInt("ComponentsSize", 0);
		for (uint count = 0; count < compCount; count++) {
			ref = "cmp" + std::to_string(count);
			JSONNode* cmpNode = comps->PullJObject(ref.c_str());

			ComponentType type = (ComponentType)cmpNode->PullInt("type", ComponentType::C_EMPTY);

			switch (type)
			{
			case C_CUBE:
			{
				RE_CompPrimitive* newCube = nullptr;
				new_go->AddComponent(newCube = App->primitives->CreateCube(new_go));
				newCube->SetColor(cmpNode->PullFloatVector("color", math::vec::one));
				break;
			}
			case C_PLANE:
			{
				RE_CompPrimitive* newPlane = nullptr;
				new_go->AddComponent(newPlane = App->primitives->CreatePlane(new_go, cmpNode->PullInt("slices", 3), cmpNode->PullInt("stacks", 3)));
				newPlane->SetColor(cmpNode->PullFloatVector("color", math::vec::one));
				break;
			}
			case C_SPHERE:
			{
				RE_CompPrimitive* newSphere = nullptr;
				new_go->AddComponent(newSphere = App->primitives->CreateSphere(new_go, cmpNode->PullInt("slices", 3), cmpNode->PullInt("stacks", 3)));
				newSphere->SetColor(cmpNode->PullFloatVector("color", math::vec::one));
				break;
			}
			case C_MESH:
			{
				RE_CompMesh* newMesh = nullptr;
				int meshID = cmpNode->PullInt("meshResource", -1);
				const char* meshMD5 = nullptr;
				if (meshID != -1) {
					meshMD5 = resources->at(meshID);

					newMesh = new RE_CompMesh(new_go, meshMD5);

					const char* materialMD5 = nullptr;
					int materialID = cmpNode->PullInt("materialResource", -1);
					if (materialID != -1) {
						materialMD5 = resources->at(materialID);
						newMesh->SetMaterial(materialMD5);
					}
				}
				break;
			}
			case C_CAMERA:
			{
				int skyboxid = cmpNode->PullInt("skyboxResource", -1);
				new_go->AddCompCamera(
					cmpNode->PullBool("isPrespective", true),
					cmpNode->PullFloat("near_plane", 1),
					cmpNode->PullFloat("far_plane", 10000),
					cmpNode->PullFloat("v_fov_rads", 30.0f),
					cmpNode->PullInt("aspect_ratio", RE_CompCamera::AspectRatioTYPE::Fit_Window),
					cmpNode->PullBool("draw_frustum", true),
					cmpNode->PullBool("usingSkybox", true),
					(skyboxid != -1) ? resources->at(skyboxid) : nullptr);
				break;
			}
			}
			DEL(cmpNode);
		}
		DEL(comps);
		DEL(goNode);
	}
	DEL(gameObjects);

	return rootGo;
}

RE_GameObject* RE_GameObject::DeserializeBinary(char*& cursor, std::map<int, const char*>* resources)
{
	RE_GameObject* rootGo = nullptr;
	RE_GameObject* new_go = nullptr;

	size_t size = sizeof(uint);
	uint GOCount = 0;
	memcpy(&GOCount, cursor, size);
	cursor += size;

	for (uint count = 0; count < GOCount; count++) {
		char nullchar = '\0';

		uint strLenght = 0;
		size = sizeof(uint);
		memcpy(&strLenght, cursor, size);
		cursor += size;

		char* strName = new char[strLenght + 1];
		char* strNameCursor = strName;
		size = sizeof(char) * strLenght;
		memcpy(strName, cursor, size);
		strNameCursor += size;
		cursor += size;
		memcpy(strNameCursor, &nullchar, sizeof(char));

		UUID uuid;
		UUID parent_uuid;

		size = sizeof(char) * 36;
		char* strUUID = new char[36 + 1];
		char* strUUIDCursor = strUUID;
		memcpy(strUUID, cursor, size);
		cursor += size;
		strUUIDCursor += size;
		memcpy(strUUIDCursor, &nullchar, sizeof(char));
		UuidFromStringA((RPC_CSTR)strUUID, &uuid);
		DEL_A(strUUID);

		if (rootGo != nullptr) {
			char* strUUIDParent = new char[36 + 1];
			char* strUUIDParentCursor = strUUIDParent;
			memcpy(strUUIDParent, cursor, size);
			cursor += size;
			strUUIDParentCursor += size;
			memcpy(strUUIDParentCursor, &nullchar, sizeof(char));
			UuidFromStringA((RPC_CSTR)strUUIDParent, &parent_uuid);
			DEL_A(strUUIDParent);
		}

		(rootGo == nullptr) ? rootGo = new_go = new RE_GameObject(strName, uuid) : new_go = new RE_GameObject(strName, uuid, rootGo->GetGoFromUUID(parent_uuid));
		DEL_A(strName);

		float vec[3] = { 0.0, 0.0, 0.0 };
		size = sizeof(float) * 3;

		memcpy(vec, cursor, size);
		cursor += size;
		new_go->GetTransform()->SetPosition(math::vec(vec));

		memcpy(vec, cursor, size);
		cursor += size;
		new_go->GetTransform()->SetRotation(math::vec(vec));

		memcpy(vec, cursor, size);
		cursor += size;
		new_go->GetTransform()->SetScale(math::vec(vec));

		size = sizeof(uint);
		uint compCount = 0;
		memcpy(&compCount, cursor, size);
		cursor += size;
		for (uint count = 0; count < compCount; count++) {

			unsigned short typeInt = 0;
			size = sizeof(unsigned short);
			memcpy(&typeInt, cursor, size);
			cursor += size;

			ComponentType type = (ComponentType)typeInt;
			switch (type)
			{
			case C_CUBE:
			{
				RE_CompPrimitive* newCube = nullptr;
				new_go->AddComponent(newCube = App->primitives->CreateCube(new_go));

				size = sizeof(float) * 3;
				memcpy(vec, cursor, size);
				cursor += size;
				newCube->SetColor(math::vec(vec));
				break;
			}
			case C_PLANE:
			{
				RE_CompPrimitive* newPlane = nullptr;

				int slices = 0, stacks = 0;
				size = sizeof(int);
				memcpy(&slices, cursor, size);
				cursor += size;

				memcpy(&stacks, cursor, size);
				cursor += size;

				new_go->AddComponent(newPlane = App->primitives->CreatePlane(new_go, slices, stacks));

				size = sizeof(float) * 3;
				memcpy(vec, cursor, size);
				cursor += size;

				newPlane->SetColor(math::vec(vec));
				break;
			}
			case C_SPHERE:
			{
				RE_CompPrimitive* newSphere = nullptr;

				int slices = 0, stacks = 0;
				size = sizeof(int);
				memcpy(&slices, cursor, size);
				cursor += size;

				memcpy(&stacks, cursor, size);
				cursor += size;

				new_go->AddComponent(newSphere = App->primitives->CreateSphere(new_go, slices, stacks));

				size = sizeof(float) * 3;
				memcpy(vec, cursor, size);
				cursor += size;

				newSphere->SetColor(math::vec(vec));
				break;
			}
			case C_MESH:
			{
				RE_CompMesh* newMesh = nullptr;

				int meshID = -1;
				size = sizeof(int);
				memcpy(&meshID, cursor, size);
				cursor += size;

				const char* meshMD5 = nullptr;
				if (meshID != -1) {
					meshMD5 = resources->at(meshID);
					newMesh = new RE_CompMesh(new_go, meshMD5);

					const char* materialMD5 = nullptr;
					int materialID = -1;
					memcpy(&materialID, cursor, size);
					cursor += size;
					if (materialID != -1) {
						materialMD5 = resources->at(materialID);
						newMesh->SetMaterial(materialMD5);
					}
				}
				new_go->AddCompMesh(newMesh);
				break;
			}
			case C_CAMERA:
			{
				bool isPrespective = true;
				size = sizeof(bool);
				memcpy(&isPrespective, cursor, size);
				cursor += size;

				float nearPlane = 1.0;
				size = sizeof(float);
				memcpy(&nearPlane, cursor, size);
				cursor += size;

				float farPlane = 10000.0;
				memcpy(&farPlane, cursor, size);
				cursor += size;

				float vfovrads = 30.0;
				memcpy(&vfovrads, cursor, size);
				cursor += size;

				int aspectRatioInt = 0;
				size = sizeof(int);
				memcpy(&aspectRatioInt, cursor, size);
				cursor += size;
				RE_CompCamera::AspectRatioTYPE aspectRatio = (RE_CompCamera::AspectRatioTYPE)aspectRatioInt;

				bool drawFrustum = true;
				size = sizeof(bool);
				memcpy(&drawFrustum, cursor, size);
				cursor += size;

				bool usingSkybox = true;
				memcpy(&usingSkybox, cursor, size);
				cursor += size;

				int skyboxid = -1;
				size = sizeof(int);
				memcpy(&skyboxid, cursor, size);
				cursor += size;

				new_go->AddCompCamera(
					isPrespective,
					nearPlane,
					farPlane,
					vfovrads,
					aspectRatio,
					drawFrustum,
					usingSkybox,
					(skyboxid != -1) ? resources->at(skyboxid) : nullptr);
				break;
			}
			}
		}
	}

	return rootGo;
}

void RE_GameObject::AddChild(RE_GameObject * child, bool broadcast)
{
	SDL_assert(child != nullptr);

	child->parent = this;
	childs.push_back(child);

	if (broadcast)
		Event::Push(GO_HAS_NEW_CHILD, App->scene, this, child);
}

void RE_GameObject::AddChildsFromGO(RE_GameObject * go, bool broadcast)
{
	SDL_assert(go != nullptr);

	if (!go->childs.empty())
	{
		for (auto child : go->childs)
			AddChild(child);
	}
}

void RE_GameObject::RemoveChild(RE_GameObject * child, bool broadcast)
{
	SDL_assert(child != nullptr);

	if (!broadcast)
		childs.remove(child);
	else
		Event::Push(GO_REMOVE_CHILD, App->scene, this, child);
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

void RE_GameObject::GetChilds(std::list<const RE_GameObject*>& out_childs) const
{
	for (auto child : childs)
		out_childs.push_back(child);
}

unsigned int RE_GameObject::ChildCount() const { return childs.size(); }

bool RE_GameObject::IsLastChild() const
{
	return parent != nullptr && (parent->childs.back() == this);
}

RE_GameObject * RE_GameObject::GetParent() const { return parent; }

const RE_GameObject * RE_GameObject::GetParent_c() const { return parent; }

void RE_GameObject::SetParent(RE_GameObject * p)
{
	SDL_assert(p != nullptr);
	parent = p;
}

bool RE_GameObject::IsActive() const { return active; }

void RE_GameObject::SetActive(const bool value, const bool broadcast)
{
	if (active != value)
	{
		active = value;

		if (broadcast)
			Event::Push(active ? GO_CHANGED_TO_ACTIVE : GO_CHANGED_TO_INACTIVE, App->scene);
	}
}

void RE_GameObject::SetActiveRecursive(bool value)
{
	active = value;

	for (auto child : childs)
		child->SetActiveRecursive(active);
}

void RE_GameObject::IterativeSetActive(bool val)
{
	bool tmp = active;
	std::stack<RE_GameObject*> gos;
	gos.push(this);

	while (!gos.empty())
	{
		RE_GameObject * go = gos.top();
		go->SetActive(val);
		gos.pop();

		for (auto child : go->childs)
			gos.push(child);
	}

	active = tmp;
}

bool RE_GameObject::IsStatic() const
{
	return isStatic;
}

bool RE_GameObject::IsActiveStatic() const
{
	return active && isStatic;
}

void RE_GameObject::SetStatic(const bool value, const bool broadcast)
{
	if (isStatic != value)
	{
		isStatic = value;

		if (active && broadcast)
			Event::Push(isStatic ? GO_CHANGED_TO_STATIC : GO_CHANGED_TO_NON_STATIC, App->scene);
	}
}

void RE_GameObject::IterativeSetStatic(bool val)
{
	SetStatic(val);

	std::stack<RE_GameObject*> gos;
	for (auto child : childs)
		gos.push(child);

	while (!gos.empty())
	{
		RE_GameObject * go = gos.top();
		go->SetStatic(val, false);
		gos.pop();

		for (auto child : go->childs)
			gos.push(child);
	}
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

void RE_GameObject::UseResources()
{
	for (auto comp : components) comp->UseResources();
	for (auto child : childs) child->UseResources();
}

void RE_GameObject::UnUseResources()
{
	for (auto comp : components) comp->UnUseResources();
	for (auto child : childs) child->UnUseResources();
}

RE_CompCamera * RE_GameObject::AddCompCamera(bool prespective, float near_plane, float far_plane, float v_fov_rads, short aspect_ratio_t, bool draw_frustum, bool usingSkybox, const char* skyboxMD5)
{
	RE_CompCamera* comp_camera = new RE_CompCamera(this, prespective, near_plane, far_plane, v_fov_rads, aspect_ratio_t, draw_frustum, usingSkybox, skyboxMD5);
	components.push_back((RE_Component*)comp_camera);
	if(!Event::isPaused()) App->cams->AddMainCamera(comp_camera);
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
		case C_GRID:
		{
			ret = (RE_Component*)(App->primitives->CreateGrid(this));
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

void RE_GameObject::RecieveEvent(const Event & e)
{
}

void RE_GameObject::TransformModified(bool broadcast)
{
	ResetGlobalBoundingBox();

	for (auto component : components)
		component->OnTransformModified();

	if (!broadcast)
	{
		for (auto child : childs)
			if (!child->IsActive())
				child->TransformModified(false);
	}
	else
		Event::Push(TRANSFORM_MODIFIED, App->scene, this);
}

const char * RE_GameObject::GetName() const
{
	return name.c_str();
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

void RE_GameObject::DrawAABB(math::vec color) const
{
	RE_GLCache::ChangeShader(0);

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

void RE_GameObject::DrawGlobalAABB() const
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
		Event::Push(isStatic ? GO_CHANGED_TO_STATIC : GO_CHANGED_TO_NON_STATIC, App->scene);

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
