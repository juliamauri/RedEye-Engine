#include "RE_GameObject.h"

#include "Application.h"
#include "ModuleScene.h"
#include "RE_FileSystem.h"
#include "RE_PrimitiveManager.h"
#include "RE_Component.h"
#include "RE_CompPrimitive.h"
#include "RE_CompMesh.h"
#include "RE_CompLight.h"
#include "RE_GOManager.h"
#include "RE_CompParticleEmiter.h"
#include "RE_ShaderImporter.h"
#include "RE_GLCache.h"
#include "RE_Shader.h"
#include "RE_InternalResources.h"
#include "RE_ResourceManager.h"
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

RE_GameObject::RE_GameObject()
{
}

RE_GameObject::~RE_GameObject()
{
}

void RE_GameObject::SetUp(ComponentsPool* compPool, const char* name, RE_GameObject* parent, bool start_active, bool isStatic)
{
	poolComponents = compPool;
	this->isStatic = isStatic;
	this->name = name;

	transform = (RE_CompTransform*)poolComponents->GetNewComponent(ComponentType::C_TRANSFORM);
	transform->SetUp(this);

	local_bounding_box.SetFromCenterAndSize(math::vec::zero, math::vec::zero);
	global_bounding_box.SetFromCenterAndSize(math::vec::zero, math::vec::zero);

	if (parent != nullptr)
		parent->AddChild(this, false);
}

void RE_GameObject::PreUpdate()
{
	for (auto component : GetComponents()) component->PreUpdate();
	for (auto child : childs) child->PreUpdate();
}

void RE_GameObject::Update()
{
	for (auto component : GetComponents()) component->Update();
	for (auto child : childs) child->Update();
}

void RE_GameObject::PostUpdate()
{
	for (auto component : GetComponents()) component->PostUpdate();
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

			for (auto component : go->GetComponents())
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
		for (auto component : GetComponents())
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
				for (auto component : go->GetComponents())
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
	for (auto component : GetComponents())
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

			for (auto component : go->GetComponents())
			{
				ComponentType cT = component->GetType();
				if (cT == type) ret.push(component);
			}

		for (auto child : go->GetChilds())
			gos.push(child);
	}

	return ret;
}

eastl::list<RE_Component*> RE_GameObject::GetComponents() const
{
	eastl::list<RE_Component*> ret;
	for (const cmpPoolID compid : componentsID)
		ret.push_back(poolComponents->GetComponent(compid.poolId, (ComponentType)compid.cType));
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
	for (auto component : GetComponents())
	{
		ushortint cT = component->GetType();

		if (cT == ComponentType::C_MESH ||
			(cT > ComponentType::C_PRIMIVE_MIN && cT < ComponentType::C_PRIMIVE_MAX))
			return true;
	}

	return false;
}

void RE_GameObject::SerializeJson(JSONNode * node)
{
	node->PushString("name", GetName());

	if (parent != nullptr)
		node->PushInt("Parent Pool ID", parent->GetPoolID());

	node->PushFloatVector("position", transform->GetLocalPosition());
	node->PushFloatVector("rotation", transform->GetLocalEulerRotation());
	node->PushFloatVector("scale", transform->GetLocalScale());

}

void RE_GameObject::DeserializeJSON(JSONNode* node, ComponentsPool* cmpsPool, eastl::map<int, RE_GameObject*>* idGO)
{
	if (!idGO->empty())
		SetUp(cmpsPool, node->PullString("name", "GameObject"), idGO->at(node->PullInt("Parent Pool ID", -1)));
	else
		SetUp(cmpsPool, node->PullString("name", "GameObject"), nullptr);

	transform->SetPosition(node->PullFloatVector("position", math::vec::zero));
	transform->SetRotation(node->PullFloatVector("rotation", math::vec::zero));
	transform->SetScale(node->PullFloatVector("scale", math::vec::one));

	idGO->insert(eastl::pair < int, RE_GameObject*>(poolID, this));
}


unsigned int RE_GameObject::GetBinarySize()const
{
	uint size = sizeof(uint) + sizeof(float) * 9;
	size += eastl::CharStrlen(GetName());
	if (parent != nullptr) size += sizeof(int);
	return size;
}

void RE_GameObject::SerializeBinary(char*& cursor)
{
	uint size = sizeof(uint);
	const char* name = GetName();
	uint strLenght = eastl::CharStrlen(name);
	memcpy(cursor, &strLenght, size);
	cursor += size;

	size = sizeof(char) * strLenght;
	memcpy(cursor, name, size);
	cursor += size;

	if (parent != nullptr)
	{
		size = sizeof(int);
		int parentID = parent->GetPoolID();
		memcpy(cursor, &parentID, size);
		cursor += size;
	}

	size = sizeof(float) * 3;
	memcpy(cursor, &transform->GetLocalPosition()[0], size);
	cursor += size;

	memcpy(cursor, &transform->GetLocalEulerRotation()[0], size);
	cursor += size;

	memcpy(cursor, &transform->GetLocalScale()[0], size);
	cursor += size;
}

void RE_GameObject::DeserializeBinary(char*& cursor, ComponentsPool* compPool, eastl::map<int, RE_GameObject*>* idGO)
{
	char nullchar = '\0';

	uint strLenght = 0;
	uint size = sizeof(uint);
	memcpy(&strLenght, cursor, size);
	cursor += size;

	char* strName = new char[strLenght + 1];
	char* strNameCursor = strName;
	size = sizeof(char) * strLenght;
	memcpy(strName, cursor, size);
	strNameCursor += size;
	cursor += size;
	memcpy(strNameCursor, &nullchar, sizeof(char));


	if (!idGO->empty()) {
		size = sizeof(int);
		int goParentID;
		memcpy(&goParentID, cursor, size);
		cursor += size;

		SetUp(compPool, strName, idGO->at(goParentID));
	}
	else
		SetUp(compPool, strName, nullptr);
	DEL_A(strName);

	float vec[3] = { 0.0, 0.0, 0.0 };
	size = sizeof(float) * 3;

	memcpy(vec, cursor, size);
	cursor += size;
	transform->SetPosition(math::vec(vec));

	memcpy(vec, cursor, size);
	cursor += size;
	transform->SetRotation(math::vec(vec));

	memcpy(vec, cursor, size);
	cursor += size;
	transform->SetScale(math::vec(vec));

	idGO->insert(eastl::pair < int, RE_GameObject*>(poolID, this));
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
	for (auto component : GetComponents()) component->OnPlay();
	for (auto child : childs) child->OnPlay();
}

void RE_GameObject::OnPause()
{
	for (auto component : GetComponents()) component->OnPause();
	for (auto child : childs) child->OnPause();
}

void RE_GameObject::OnStop()
{
	for (auto component : GetComponents()) component->OnStop();
	for (auto child : childs) child->OnStop();
}

RE_CompCamera * RE_GameObject::AddCompCamera(bool prespective, float near_plane, float far_plane, float v_fov_rads, short aspect_ratio_t, bool draw_frustum, bool usingSkybox, const char* skyboxMD5)
{
	RE_CompCamera* comp_camera = (RE_CompCamera*)poolComponents->GetNewComponent(ComponentType::C_CAMERA);
	comp_camera->SetUp(this, prespective, near_plane, far_plane, v_fov_rads, aspect_ratio_t, draw_frustum, usingSkybox, skyboxMD5);
	if(!Event::isPaused()) App->cams->AddMainCamera(comp_camera);
	return comp_camera;
}

RE_CompPrimitive* RE_GameObject::AddCompPrimitive(ushortint type, int _slices, int _stacks, float _radius)
{
	RE_CompPrimitive* ret = nullptr;
	ret = (RE_CompPrimitive * )poolComponents->GetNewComponent(ComponentType(type));

	switch (ComponentType(type))
	{
	case C_CUBE:
	case C_DODECAHEDRON:
	case C_TETRAHEDRON:
	case C_OCTOHEDRON:
	case C_ICOSAHEDRON:
		App->primitives->SetUpComponentPrimitive(ret, this);
			break;
	case C_PLANE:
	case C_SPHERE:
	case C_CYLINDER:
	case C_HEMISHPERE:
	case C_TORUS:
	case C_TREFOILKNOT:
		((RE_CompParametric*)ret)->SetUp(this, ((RE_Shader*)App->resources->At(App->internalResources->GetDefaultShader()))->GetID(), _slices, _stacks, true, _radius);
		break;
	}
	return ret;
}

RE_CompPrimitive* RE_GameObject::AddCompRock(int _rockSeed, int _rockNSubdivvisions)
{
	RE_CompPrimitive* ret = nullptr;
	ret = (RE_CompPrimitive*)poolComponents->GetNewComponent(C_ROCK);
	((RE_CompRock*)ret)->SetUp(this, ((RE_Shader*)App->resources->At(App->internalResources->GetDefaultShader()))->GetID(), _rockSeed, _rockNSubdivvisions);
	return ret;
}

void RE_GameObject::AddComponent(RE_Component * component)
{
	componentsID.push_back(cmpPoolID(component->GetPoolID(), component->GetType()));
}

RE_Component* RE_GameObject::AddComponent(const ushortint type)
{
	SDL_assert(type < MAX_COMPONENT_TYPES);
	RE_Component* ret = nullptr;

	if (ComponentType(type) == C_TRANSFORM)
	{
		if (transform) {
			RemoveComponent(transform);
			poolComponents->DeleteTransform(transform->GetPoolID());
		}
		transform = (RE_CompTransform*)poolComponents->GetNewComponent(ComponentType::C_TRANSFORM);
		transform->SetUp(this);
		ret = (RE_Component*)transform;
	}
	else if (ComponentType(type) == C_MESH)
	{
		RE_CompMesh* mesh_comp = (RE_CompMesh * )poolComponents->GetNewComponent(ComponentType::C_MESH);
		mesh_comp->SetUp(this);
		//AddToBoundingBox(mesh_comp->GetAABB());
		ret = (RE_Component*)mesh_comp;
	}
	else if (ComponentType(type) == C_CAMERA)
	{
		RE_CompCamera* comp_camera = (RE_CompCamera*)poolComponents->GetNewComponent(ComponentType::C_CAMERA);
		comp_camera->SetUp(this);
		App->cams->AddMainCamera(comp_camera);
		ret = (RE_Component*)comp_camera;
	}
	else if (ComponentType(type) == C_LIGHT)
	{
		RE_CompLight* comp_light = (RE_CompLight*)poolComponents->GetNewComponent(ComponentType::C_LIGHT);
		comp_light->SetUp(this);
		ret = (RE_Component*)comp_light;
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
			ret = (RE_Component*)(App->primitives->CreateGrid(this));
			break;
		case C_CUBE:
		case C_DODECAHEDRON:
		case C_TETRAHEDRON:
		case C_OCTOHEDRON:
		case C_ICOSAHEDRON:
		case C_SPHERE:
		case C_CYLINDER:
		case C_HEMISHPERE:
		case C_TORUS:
		case C_TREFOILKNOT:
		case C_ROCK:
		case C_PLANE:
			ret = poolComponents->GetNewComponent(ComponentType(type));
		}
		if (type != C_GRID && ret != nullptr) App->primitives->SetUpComponentPrimitive((RE_CompPrimitive*)ret, this);

	}

	if (ret == nullptr)
		LOG_ERROR("GameObject could not add type %u component", type);

	return ret;
}

void RE_GameObject::RemoveComponent(RE_Component * component)
{
	SDL_assert(component != nullptr);

	unsigned int count = 0;
	for (cmpPoolID compid : componentsID) {
		if(compid.poolId == component->GetPoolID() && compid.cType == component->GetType())
			componentsID.erase(&componentsID[count]);
		count++;
	}
}

RE_CompTransform * RE_GameObject::AddCompTransform()
{
	if (transform) {
		RemoveComponent(transform);
		poolComponents->DeleteTransform(transform->GetPoolID());
	}
	transform = (RE_CompTransform*)poolComponents->GetNewComponent(ComponentType::C_TRANSFORM);
	transform->SetUp(this);
	return transform;
}

RE_CompMesh* RE_GameObject::AddCompMesh()
{
	return (RE_CompMesh*)poolComponents->GetNewComponent(ComponentType::C_MESH);
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
		for (auto component : GetComponents())
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

	for (auto component : GetComponents())
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

	for (auto component : GetComponents())
	{
		if (component->GetType() == ComponentType::C_CAMERA)
		{
			ret = (RE_CompCamera*)component;
			break;
		}
	}

	return ret;
}

RE_CompLight* RE_GameObject::GetLight() const
{
	RE_CompLight* ret = nullptr;

	for (auto component : GetComponents())
	{
		if (component->GetType() == ComponentType::C_LIGHT)
		{
			ret = (RE_CompLight*)component;
			break;
		}
	}

	return ret;
}

void RE_GameObject::RecieveEvent(const Event & e)
{
}

void RE_GameObject::TransformModified(bool broadcast)
{
	for (auto component : GetComponents())
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

	for (RE_Component* comp : GetComponents())
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

	for (auto component : GetComponents())
		component->DrawProperties();
}

int RE_GameObject::GetPoolID() const
{
	return poolID;
}

void RE_GameObject::SetPoolID(int id)
{
	poolID = id;
}
