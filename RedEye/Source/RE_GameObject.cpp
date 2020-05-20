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

#include <EASTL/unordered_set.h>
#include <EASTL/queue.h>
#include <EASTL/stack.h>
#include <EASTL/internal/char_traits.h>

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
		case C_DODECAHEDRON:
			components.push_back((RE_CompPrimitive*)new RE_CompDodecahedron(*(RE_CompDodecahedron*)((RE_CompPrimitive*)cmpGO), this));
			break;
		case C_TETRAHEDRON:
			components.push_back((RE_CompPrimitive*)new RE_CompTetrahedron(*(RE_CompTetrahedron*)((RE_CompPrimitive*)cmpGO), this));
			break;
		case C_OCTOHEDRON:
			components.push_back((RE_CompPrimitive*)new RE_CompOctohedron(*(RE_CompOctohedron*)((RE_CompPrimitive*)cmpGO), this));
			break;
		case C_ICOSAHEDRON:
			components.push_back((RE_CompPrimitive*)new RE_CompIcosahedron(*(RE_CompIcosahedron*)((RE_CompPrimitive*)cmpGO), this));
			break;
		case C_SPHERE:
			components.push_back((RE_CompPrimitive*)new RE_CompSphere(*(RE_CompSphere*)((RE_CompPrimitive*)cmpGO), this));
			break;
		case C_CYLINDER:
			components.push_back((RE_CompPrimitive*)new RE_CompCylinder(*(RE_CompCylinder*)((RE_CompPrimitive*)cmpGO), this));
			break;
		case C_HEMISHPERE:
			components.push_back((RE_CompPrimitive*)new RE_CompHemiSphere(*(RE_CompHemiSphere*)((RE_CompPrimitive*)cmpGO), this));
			break;
		case C_TORUS:
			components.push_back((RE_CompPrimitive*)new RE_CompTorus(*(RE_CompTorus*)((RE_CompPrimitive*)cmpGO), this));
			break;
		case C_TREFOILKNOT:
			components.push_back((RE_CompPrimitive*)new RE_CompTrefoiKnot(*(RE_CompTrefoiKnot*)((RE_CompPrimitive*)cmpGO), this));
			break;
		case C_ROCK:
			components.push_back((RE_CompPrimitive*)new RE_CompRock(*(RE_CompRock*)((RE_CompPrimitive*)cmpGO), this));
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
		eastl::stack<const RE_GameObject*> gos;
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

eastl::stack<RE_Component*> RE_GameObject::GetDrawableComponentsWithChilds(RE_GameObject* ignoreStencil) const
{
	eastl::stack<RE_Component*> ret;

	if (active)
	{
		eastl::stack<const RE_GameObject*> gos;
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

eastl::stack<RE_Component*> RE_GameObject::GetDrawableComponentsItselfOnly() const
{
	eastl::stack<RE_Component*> ret;
	for (auto component : components)
	{
		ComponentType cT = component->GetType();
		if (cT == ComponentType::C_MESH || (cT > ComponentType::C_PRIMIVE_MIN && cT < ComponentType::C_PRIMIVE_MAX)) ret.push(component);
	}
	return ret;
}

eastl::stack<RE_Component*> RE_GameObject::GetAllComponentWithChilds(unsigned short type) const
{
	eastl::stack<RE_Component*> ret;

	eastl::stack<const RE_GameObject*> gos;
	gos.push(this);
	while (!gos.empty())
	{
		const RE_GameObject* go = gos.top();
		gos.pop();

			for (auto component : go->components)
			{
				ComponentType cT = component->GetType();
				if (cT == type) ret.push(component);
			}

		for (auto child : go->GetChilds())
			gos.push(child);
	}

	return ret;
}

eastl::vector<RE_GameObject*> RE_GameObject::GetAllGO()
{
	eastl::vector<RE_GameObject*> ret;
	ret.push_back(this);
	for (auto child : childs) {

		eastl::vector<RE_GameObject*> childRet = child->GetAllGO();
		if (!childRet.empty())
			ret.insert(ret.end(), childRet.begin(), childRet.end());
	}
	return ret;
}

eastl::vector<RE_GameObject*> RE_GameObject::GetActiveChildsWithDrawComponents()
{
	eastl::vector<RE_GameObject*> ret;

	if (active)
	{
		eastl::queue<RE_GameObject*> go_queue;
		go_queue.push(this);
		
		while (!go_queue.empty())
		{
			RE_GameObject* go = go_queue.front();
			go_queue.pop();

			if (go->HasDrawComponents())
				ret.push_back(go);

			for (auto child : go->childs)
				if (child->active)
					go_queue.push(child);
		}
	}

	return ret;
}

bool RE_GameObject::HasDrawComponents() const
{
	for (auto component : components)
	{
		ushortint cT = component->GetType();

		if (cT == ComponentType::C_MESH ||
			(cT > ComponentType::C_PRIMIVE_MIN && cT < ComponentType::C_PRIMIVE_MAX))
			return true;
	}

	return false;
}

eastl::vector<const char*> RE_GameObject::GetAllResources(bool root)
{
	eastl::vector<const char*> allResources;

	for (auto comp : components) {
		eastl::vector<const char*> cmpRet = comp->GetAllResources();
		if (!cmpRet.empty())
			allResources.insert(allResources.end(), cmpRet.begin(), cmpRet.end());
	}

	for (auto child : childs) {

		eastl::vector<const char*> childRet = child->GetAllResources(false);
		if(!childRet.empty())
			allResources.insert(allResources.end(), childRet.begin(), childRet.end());
	}

	if (root)
	{
		//unique resources
		eastl::vector<const char*> ret;

		int resSize = 0;
		for (auto res : allResources) {
			bool repeat = false;
			for (auto uniqueRes : ret) {
				resSize = eastl::CharStrlen(res);
				if (resSize > 0 && eastl::Compare(res, uniqueRes, resSize) == 0) {
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

void RE_GameObject::SerializeJson(JSONNode * node, eastl::map<const char*, int>* resources)
{
	eastl::vector<RE_GameObject*> allGOs = GetAllGO();
	JSONNode* gameObjects = node->PushJObject("gameobjects");
	gameObjects->PushUInt("gameobjectsSize", allGOs.size());

	uint count = 0;
	eastl::string ref;
	for (RE_GameObject* go : allGOs) {
		ref = "go" + eastl::to_string(count++);
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
			if ((type >= C_CUBE && type <= C_PRIMIVE_MAX) || type == C_MESH || type == C_CAMERA)
				cmpSize++;
		}
		comps->PushUInt("ComponentsSize", cmpSize);
		uint count = 0;
		for (auto component : go->components) {
			unsigned short type = component->GetType();
			if (type < C_CUBE && type > C_PRIMIVE_MAX && type != C_MESH && type != C_CAMERA)
				continue;
			ref = "cmp" + eastl::to_string(count++);
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
	size += eastl::CharStrlen(GetName()) * sizeof(char);

	for (auto component : components) size += component->GetBinarySize();

	for (auto child : childs) size += child->GetBinarySize();

	return size;
}

void RE_GameObject::SerializeBinary(char*& cursor, eastl::map<const char*, int>* resources)
{
	eastl::vector<RE_GameObject*> allGOs = GetAllGO();

	size_t size = sizeof(uint);
	uint goSize = allGOs.size();
	memcpy(cursor, &goSize, size);
	cursor += size;

	for (RE_GameObject* go : allGOs) {
		uint strLenght = eastl::CharStrlen(go->GetName());
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
			if ((type >= C_CUBE && type <= C_PRIMIVE_MAX) || type == C_MESH || type == C_CAMERA)
				cmpSize++;
		}
		memcpy(cursor, &cmpSize, size);
		cursor += size;

		for (auto component : go->components) {
			unsigned short type = component->GetType();
			if (type < C_CUBE && type > C_PRIMIVE_MAX && type != C_MESH && type != C_CAMERA)
				continue;
			size = sizeof(unsigned short);
			memcpy(cursor, &type, size);
			cursor += size;

			component->SerializeBinary(cursor, resources);
		}
	}
}

RE_GameObject* RE_GameObject::DeserializeJSON(JSONNode* node, eastl::map<int, const char*>* resources)
{
	RE_GameObject* rootGo = nullptr;
	RE_GameObject* new_go = nullptr;
	JSONNode* gameObjects = node->PullJObject("gameobjects");
	uint GOCount = gameObjects->PullUInt("gameobjectsSize", 0);

	eastl::string ref;
	for (uint count = 0; count < GOCount; count++) {
		ref = "go" + eastl::to_string(count);
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
			ref = "cmp" + eastl::to_string(count);
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
			case C_DODECAHEDRON:
			{
				RE_CompPrimitive* newDode = nullptr;
				new_go->AddComponent(newDode = App->primitives->CreateDodecahedron(new_go));
				newDode->SetColor(cmpNode->PullFloatVector("color", math::vec::one));
				break;
			}
			case C_TETRAHEDRON:
			{
				RE_CompPrimitive* newTetra = nullptr;
				new_go->AddComponent(newTetra = App->primitives->CreateTetrahedron(new_go));
				newTetra->SetColor(cmpNode->PullFloatVector("color", math::vec::one));
				break;
			}
			case C_OCTOHEDRON:
			{
				RE_CompPrimitive* newOcto = nullptr;
				new_go->AddComponent(newOcto = App->primitives->CreateOctohedron(new_go));
				newOcto->SetColor(cmpNode->PullFloatVector("color", math::vec::one));
				break;
			}
			case C_ICOSAHEDRON:
			{
				RE_CompPrimitive* newIcosa = nullptr;
				new_go->AddComponent(newIcosa = App->primitives->CreateIcosahedron(new_go));
				newIcosa->SetColor(cmpNode->PullFloatVector("color", math::vec::one));
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
			case C_CYLINDER:
			{
				RE_CompPrimitive* newCylinder = nullptr;
				new_go->AddComponent(newCylinder = App->primitives->CreateCylinder(new_go, cmpNode->PullInt("slices", 3), cmpNode->PullInt("stacks", 3)));
				newCylinder->SetColor(cmpNode->PullFloatVector("color", math::vec::one));
				break;
			}
			case C_HEMISHPERE:
			{
				RE_CompPrimitive* newHemiSphere = nullptr;
				new_go->AddComponent(newHemiSphere = App->primitives->CreateHemiSphere(new_go, cmpNode->PullInt("slices", 3), cmpNode->PullInt("stacks", 3)));
				newHemiSphere->SetColor(cmpNode->PullFloatVector("color", math::vec::one));
				break;
			}
			case C_TORUS:
			{
				RE_CompPrimitive* newTorus = nullptr;
				new_go->AddComponent(newTorus = App->primitives->CreateTorus(new_go, cmpNode->PullInt("slices", 3), cmpNode->PullInt("stacks", 3), cmpNode->PullFloat("radius", 0.1f)));
				newTorus->SetColor(cmpNode->PullFloatVector("color", math::vec::one));
				break;
			}
			case C_TREFOILKNOT:
			{
				RE_CompPrimitive* newTrefoilKnot = nullptr;
				new_go->AddComponent(newTrefoilKnot = App->primitives->CreateTrefoilKnot(new_go, cmpNode->PullInt("slices", 3), cmpNode->PullInt("stacks", 3), cmpNode->PullFloat("radius", 0.1f)));
				newTrefoilKnot->SetColor(cmpNode->PullFloatVector("color", math::vec::one));
				break;
			}
			case C_ROCK:
			{
				RE_CompPrimitive* newRock = nullptr;
				new_go->AddComponent(newRock = App->primitives->CreateRock(new_go, cmpNode->PullInt("seed", 5), cmpNode->PullInt("nsubdivisions", 20)));
				newRock->SetColor(cmpNode->PullFloatVector("color", math::vec::one));
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

RE_GameObject* RE_GameObject::DeserializeBinary(char*& cursor, eastl::map<int, const char*>* resources)
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
			case C_DODECAHEDRON:
			{
				RE_CompPrimitive* newDode = nullptr;
				new_go->AddComponent(newDode = App->primitives->CreateDodecahedron(new_go));

				size = sizeof(float) * 3;
				memcpy(vec, cursor, size);
				cursor += size;
				newDode->SetColor(math::vec(vec));
				break;
			}
			case C_TETRAHEDRON:
			{
				RE_CompPrimitive* newTetra = nullptr;
				new_go->AddComponent(newTetra = App->primitives->CreateTetrahedron(new_go));

				size = sizeof(float) * 3;
				memcpy(vec, cursor, size);
				cursor += size;
				newTetra->SetColor(math::vec(vec));
				break;
			}
			case C_OCTOHEDRON:
			{
				RE_CompPrimitive* newOcto = nullptr;
				new_go->AddComponent(newOcto = App->primitives->CreateOctohedron(new_go));

				size = sizeof(float) * 3;
				memcpy(vec, cursor, size);
				cursor += size;
				newOcto->SetColor(math::vec(vec));
				break;
			}
			case C_ICOSAHEDRON:
			{
				RE_CompPrimitive* newIcosa = nullptr;
				new_go->AddComponent(newIcosa = App->primitives->CreateIcosahedron(new_go));

				size = sizeof(float) * 3;
				memcpy(vec, cursor, size);
				cursor += size;
				newIcosa->SetColor(math::vec(vec));
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
			case C_CYLINDER:
			{
				RE_CompPrimitive* newCylinder = nullptr;

				int slices = 0, stacks = 0;
				size = sizeof(int);
				memcpy(&slices, cursor, size);
				cursor += size;

				memcpy(&stacks, cursor, size);
				cursor += size;

				new_go->AddComponent(newCylinder = App->primitives->CreateCylinder(new_go, slices, stacks));

				size = sizeof(float) * 3;
				memcpy(vec, cursor, size);
				cursor += size;

				newCylinder->SetColor(math::vec(vec));
				break;
			}
			case C_HEMISHPERE:
			{
				RE_CompPrimitive* newHemiSphere = nullptr;

				int slices = 0, stacks = 0;
				size = sizeof(int);
				memcpy(&slices, cursor, size);
				cursor += size;

				memcpy(&stacks, cursor, size);
				cursor += size;

				new_go->AddComponent(newHemiSphere = App->primitives->CreateHemiSphere(new_go, slices, stacks));

				size = sizeof(float) * 3;
				memcpy(vec, cursor, size);
				cursor += size;

				newHemiSphere->SetColor(math::vec(vec));
				break;
			}
			case C_TORUS:
			{
				RE_CompPrimitive* newTorus = nullptr;

				int slices = 0, stacks = 0;
				float radius = 0;
				size = sizeof(int);
				memcpy(&slices, cursor, size);
				cursor += size;

				memcpy(&stacks, cursor, size);
				cursor += size;

				size = sizeof(float);
				memcpy(&radius, cursor, size);
				cursor += size;

				new_go->AddComponent(newTorus = App->primitives->CreateTorus(new_go, slices, stacks, radius));

				size = sizeof(float) * 3;
				memcpy(vec, cursor, size);
				cursor += size;

				newTorus->SetColor(math::vec(vec));
				break;
			}
			case C_TREFOILKNOT:
			{
				RE_CompPrimitive* newTrefoilKnot = nullptr;

				int slices = 0, stacks = 0;
				float radius = 0;
				size = sizeof(int);
				memcpy(&slices, cursor, size);
				cursor += size;

				memcpy(&stacks, cursor, size);
				cursor += size;

				size = sizeof(float);
				memcpy(&radius, cursor, size);
				cursor += size;

				new_go->AddComponent(newTrefoilKnot = App->primitives->CreateTrefoilKnot(new_go, slices, stacks, radius));

				size = sizeof(float) * 3;
				memcpy(vec, cursor, size);
				cursor += size;

				newTrefoilKnot->SetColor(math::vec(vec));
				break;
			}
			case C_ROCK:
			{
				RE_CompPrimitive* newRock = nullptr;

				int seed = 0, nsubdivisions = 0;
				size = sizeof(int);
				memcpy(&seed, cursor, size);
				cursor += size;

				memcpy(&nsubdivisions, cursor, size);
				cursor += size;

				new_go->AddComponent(newRock = App->primitives->CreateRock(new_go, seed, nsubdivisions));

				size = sizeof(float) * 3;
				memcpy(vec, cursor, size);
				cursor += size;

				newRock->SetColor(math::vec(vec));
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

eastl::list<RE_GameObject*>& RE_GameObject::GetChilds()
{
	return childs;
}

const eastl::list<RE_GameObject*>& RE_GameObject::GetChilds() const
{
	return childs;
}

void RE_GameObject::GetChilds(eastl::list<const RE_GameObject*>& out_childs) const
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
			Event::Push(active ? GO_CHANGED_TO_ACTIVE : GO_CHANGED_TO_INACTIVE, App->scene, this);
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
	eastl::stack<RE_GameObject*> gos;
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
			Event::Push(isStatic ? GO_CHANGED_TO_STATIC : GO_CHANGED_TO_NON_STATIC, App->scene, this);
	}
}

void RE_GameObject::IterativeSetStatic(bool val)
{
	SetStatic(val);

	eastl::stack<RE_GameObject*> gos;
	for (auto child : childs)
		gos.push(child);

	while (!gos.empty())
	{
		RE_GameObject * go = gos.top();
		go->SetStatic(val);
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
		case C_DODECAHEDRON:
		{
			ret = (RE_Component*)(App->primitives->CreateDodecahedron(this));
			break;
		}
		case C_TETRAHEDRON:
		{
			ret = (RE_Component*)(App->primitives->CreateTetrahedron(this));
			break;
		}
		case C_OCTOHEDRON:
		{
			ret = (RE_Component*)(App->primitives->CreateOctohedron(this));
			break;
		}
		case C_ICOSAHEDRON:
		{
			ret = (RE_Component*)(App->primitives->CreateIcosahedron(this));
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
		case C_HEMISHPERE:
		{
			ret = (RE_Component*)(App->primitives->CreateHemiSphere(this));
			break;
		}
		case C_TORUS:
		{
			ret = (RE_Component*)(App->primitives->CreateTorus(this));
			break;
		}
		case C_TREFOILKNOT:
		{
			ret = (RE_Component*)(App->primitives->CreateTrefoilKnot(this));
			break;
		}
		case C_ROCK:
		{
			ret = (RE_Component*)(App->primitives->CreateRock(this));
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
	for (auto component : components)
		component->OnTransformModified();

	ResetGlobalBoundingBox();

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

math::AABB RE_GameObject::GetGlobalBoundingBoxWithChilds() const
{
	math::AABB ret;

	if (HasDrawComponents())
	{
		ret = global_bounding_box;
	}
	else
		ret.SetFromCenterAndSize(math::vec::zero, math::vec::zero);

	if (!childs.empty())
	{
		// Create vector to store all contained points
		unsigned int cursor = 0;
		eastl::vector<math::vec> points;
		points.resize(2 + (childs.size() * 2));

		// Store local mesh AABB max and min points
		points[cursor++].Set(ret.minPoint.x, ret.minPoint.y, ret.minPoint.z);
		points[cursor++].Set(ret.maxPoint.x, ret.maxPoint.y, ret.maxPoint.z);

		// Store child AABBs max and min points
		for (auto child : childs)
		{
			// Update child AABB
			math::AABB child_aabb = child->GetGlobalBoundingBoxWithChilds();

			points[cursor++].Set(child_aabb.minPoint.x, child_aabb.minPoint.y, child_aabb.minPoint.z);
			points[cursor++].Set(child_aabb.maxPoint.x, child_aabb.maxPoint.y, child_aabb.maxPoint.z);
		}

		// Enclose stored points
		ret.SetFrom(&points[0], points.size());
	}

	return ret;
}

void RE_GameObject::ResetLocalBoundingBox()
{
	// Local Bounding Box
	local_bounding_box.SetFromCenterAndSize(math::vec::zero, math::vec::zero);

	for (RE_Component* comp : components)
	{
		switch (comp->GetType())
		{
		case C_MESH: local_bounding_box.Enclose(((RE_CompMesh*)comp)->GetAABB()); break;
		case C_SPHERE: local_bounding_box.Enclose(math::AABB(-math::vec::one, math::vec::one)); break;
		case C_CYLINDER: local_bounding_box.Enclose(math::AABB(-math::vec::one, math::vec::one)); break;
		case C_HEMISHPERE: local_bounding_box.Enclose(math::AABB(-math::vec::one, math::vec::one)); break;
		case C_TORUS: local_bounding_box.Enclose(math::AABB(-math::vec::one, math::vec::one)); break;
		case C_TREFOILKNOT: local_bounding_box.Enclose(math::AABB(-math::vec::one, math::vec::one)); break;
		case C_ROCK: local_bounding_box.Enclose(math::AABB(-math::vec::one, math::vec::one)); break;
		case C_CUBE: local_bounding_box.Enclose(math::AABB(math::vec::zero, math::vec::one)); break;
		case C_DODECAHEDRON: local_bounding_box.Enclose(math::AABB(math::vec::zero, math::vec::one)); break;
		case C_TETRAHEDRON: local_bounding_box.Enclose(math::AABB(math::vec::zero, math::vec::one)); break;
		case C_OCTOHEDRON: local_bounding_box.Enclose(math::AABB(math::vec::zero, math::vec::one)); break;
		case C_ICOSAHEDRON: local_bounding_box.Enclose(math::AABB(math::vec::zero, math::vec::one)); break;
		}
	}
}

void RE_GameObject::ResetGlobalBoundingBox()
{
	// Global Bounding Box
	global_bounding_box = local_bounding_box;
	global_bounding_box.TransformAsAABB(transform->GetMatrixModel().Transposed());
}

void RE_GameObject::ResetBoundingBoxes()
{
	ResetLocalBoundingBox();
	ResetGlobalBoundingBox();
}

void RE_GameObject::ResetBoundingBoxForAllChilds()
{
	eastl::queue<RE_GameObject*> go_queue;
	go_queue.push(this);

	while (!go_queue.empty())
	{
		RE_GameObject* go = go_queue.front();
		go_queue.pop();

		go->ResetBoundingBoxes();

		for (auto child : go->childs)
			go_queue.push(child);
	}
}

void RE_GameObject::ResetGlobalBoundingBoxForAllChilds()
{
	eastl::queue<RE_GameObject*> go_queue;
	go_queue.push(this);

	while (!go_queue.empty())
	{
		RE_GameObject* go = go_queue.front();
		go_queue.pop();

		go->ResetGlobalBoundingBox();

		for (auto child : go->childs)
			go_queue.push(child);
	}
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

	bool tmp_active = active;
	if (ImGui::Checkbox("Active", &tmp_active))
		SetActive(tmp_active);

	ImGui::SameLine();

	bool tmp_static = isStatic;
	if (ImGui::Checkbox("Static", &tmp_static))
		SetStatic(tmp_static);

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
