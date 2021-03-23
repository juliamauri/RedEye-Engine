#include "RE_GameObject.h"

#include "RE_Json.h"
#include "Application.h"
#include "ModuleInput.h"
#include "ModuleScene.h"
#include "ModuleRenderer3D.h"
#include "RE_CameraManager.h"
#include "RE_GLCache.h"
#include "RE_ResourceManager.h"
#include "RE_ECS_Pool.h"
#include "RE_PrimitiveManager.h"
#include "RE_Component.h"

#include "Glew\include\glew.h"
#include "ImGui\imgui.h"
#include <EASTL/unordered_set.h>
#include <EASTL/queue.h>
#include <EASTL/stack.h>
#include <EASTL/internal/char_traits.h>
#include <EAStdC/EASprintf.h>

RE_GameObject::RE_GameObject() {}
RE_GameObject::~RE_GameObject() {}

void RE_GameObject::SetUp(GameObjectsPool* goPool, ComponentsPool* compPool, const char* _name, const UID parent, const bool start_active, const bool _isStatic)
{
	active = start_active;
	isStatic = _isStatic;
	name = _name;
	pool_comps = compPool;
	pool_gos = goPool;

	transform = pool_comps->GetNewComponentPtr(ComponentType::C_TRANSFORM)->PoolSetUp(pool_gos, go_uid);

	if (parent_uid = parent) pool_gos->AtPtr(parent)->childs.push_back(go_uid);

	local_bounding_box.SetFromCenterAndSize(math::vec::zero, math::vec::zero);
	global_bounding_box.SetFromCenterAndSize(math::vec::zero, math::vec::zero);
}

void RE_GameObject::DrawProperties()
{
	if (ImGui::BeginMenu("Child Options"))
	{
		if (ImGui::MenuItem("Activate Childs")) SetActiveWithChilds(true);
		if (ImGui::MenuItem("Deactivate Childs")) SetActiveWithChilds(false);
		if (ImGui::MenuItem("Childs to Static")) SetStaticWithChilds(true);
		if (ImGui::MenuItem("Childs to non-Static")) SetStaticWithChilds(false);
		ImGui::EndMenu();
	}

	char name_holder[64];
	EA::StdC::Snprintf(name_holder, 64, "%s", name.c_str());
	if (ImGui::InputText("Name", name_holder, 64)) name = name_holder;

	bool tmp_active = active;
	if (ImGui::Checkbox("Active", &tmp_active)) SetActive(tmp_active);

	ImGui::SameLine();

	bool tmp_static = isStatic;
	if (ImGui::Checkbox("Static", &tmp_static)) SetStatic(tmp_static);

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

	for (auto component : GetComponentsPtr()) component->DrawProperties();
}

void RE_GameObject::DrawItselfOnly() const
{
	if (render_geo.uid) CompPtr(render_geo)->Draw();
}

void RE_GameObject::DrawWithChilds() const
{
	DrawItselfOnly();
	DrawChilds();
}

void RE_GameObject::DrawChilds() const
{
	eastl::stack<UID> gos;
	for (auto child : childs) gos.push(child);
	while (!gos.empty())
	{
		const RE_GameObject* go = pool_gos->AtPtr(gos.top());
		gos.pop();

		if (go->active)
		{
			go->DrawItselfOnly();
			for (auto child : go->childs) gos.push(child);
		}
	}
}

RE_Component* RE_GameObject::GetCompPtr(const ushortint type) const
{
	RE_Component* ret = nullptr;
	switch (static_cast<ComponentType>(type))
	{
	case ComponentType::C_TRANSFORM:
	{
		ret = CompPtr(transform, ComponentType::C_TRANSFORM);
		break;
	}
	case ComponentType::C_CAMERA:
	{
		if (camera) ret = CompPtr(camera, ComponentType::C_CAMERA);
		break;
	}
	case ComponentType::C_LIGHT:
	{
		if (light) ret = CompPtr(light, ComponentType::C_LIGHT);
		break;
	}
	default:
	{
		if (render_geo.type == type && render_geo.uid)
		{
			ret = CompPtr(render_geo);
		}
		else
		{
			for (auto comp : components)
			{
				if (comp.type == type)
				{
					ret = CompPtr(comp.uid, comp.type);
					break;
				}
			}
		}

		break;
	}
	}

	return ret;
}

UID RE_GameObject::GetCompUID(const ushortint type) const
{
	UID ret = 0;


	switch (type)
	{
	case C_TRANSFORM: ret = transform; break;
	case ComponentType::C_CAMERA: ret = camera; break;
	case ComponentType::C_LIGHT: ret = light; break;
	default:
	{
		if (render_geo.uid && render_geo.type == type)
		{
			ret = render_geo.uid;
		}
		else
		{
			for (auto comp : components)
			{
				if (comp.type == type)
				{
					ret = comp.uid;
					break;
				}
			}
		}

		break;
	}
	}

	return ret;
}

bool RE_GameObject::HasRenderGeo() const
{
	return render_geo.uid;
}

bool RE_GameObject::HasActiveRenderGeo() const
{
	return active && render_geo.uid;
}

RE_CompTransform* RE_GameObject::GetTransformPtr() const
{
	return dynamic_cast<RE_CompTransform*>(CompPtr(transform, ComponentType::C_TRANSFORM));
}

RE_Component* RE_GameObject::GetRenderGeo() const
{
	return render_geo.uid ? CompPtr(render_geo) : nullptr;
}

RE_CompMesh* RE_GameObject::GetMesh() const
{
	return (render_geo.type == ComponentType::C_MESH) ? dynamic_cast<RE_CompMesh*>(CompPtr(render_geo)) : nullptr;
}

RE_CompWater* RE_GameObject::GetWater() const
{
	return (render_geo.type == ComponentType::C_WATER) ? dynamic_cast<RE_CompWater*>(CompPtr(render_geo)) : nullptr;
}

RE_CompParticleEmitter* RE_GameObject::GetParticleSystem() const
{
	return (render_geo.type == ComponentType::C_PARTICLEEMITER) ? dynamic_cast<RE_CompParticleEmitter*>(CompPtr(render_geo)) : nullptr;
}

RE_CompCamera* RE_GameObject::GetCamera() const
{
	return camera ? dynamic_cast<RE_CompCamera*>(CompPtr(camera, ComponentType::C_CAMERA)) : nullptr;
}

RE_CompLight* RE_GameObject::GetLight() const
{
	return light ? dynamic_cast<RE_CompLight*>(CompPtr(light, ComponentType::C_LIGHT)) : nullptr;
}

RE_CompPrimitive* RE_GameObject::GetPrimitive() const
{
	return (render_geo.type > ComponentType::C_PRIMIVE_MIN && render_geo.type < ComponentType::C_PRIMIVE_MAX) ?
		dynamic_cast<RE_CompPrimitive*>(CompPtr(render_geo)) : nullptr;
}

eastl::list<RE_Component*> RE_GameObject::GetComponentsPtr() const
{
	eastl::list<RE_Component*> ret;

	ret.push_back(CompPtr(transform, ComponentType::C_TRANSFORM));
	if (render_geo.uid) ret.push_back(CompPtr(render_geo));
	if (camera) ret.push_back(CompPtr(camera, ComponentType::C_CAMERA));
	if (light) ret.push_back(CompPtr(light, ComponentType::C_LIGHT));
	for (auto comp : components) ret.push_back(CompPtr(comp));

	return ret;
}

eastl::list<RE_Component*> RE_GameObject::GetStackableComponentsPtr() const
{
	eastl::list<RE_Component*> ret;
	for (auto comp : components) ret.push_back(CompPtr(comp));
	return ret;
}

eastl::stack<RE_Component*> RE_GameObject::GetAllChildsComponents(const unsigned short type) const
{
	eastl::stack<RE_Component*> ret;

	eastl::stack<UID> gos;
	for (auto child : childs)
		gos.push(child);

	while (!gos.empty())
	{
		const RE_GameObject* go = pool_gos->AtCPtr(gos.top());
		gos.pop();

		for (auto comp : go->AllCompData())
			if (comp.type == type)
				ret.push(CompPtr(comp));

		for (auto child : go->childs)
			gos.push(child);
	}

	return ret;
}

eastl::stack<RE_Component*> RE_GameObject::GetAllChildsActiveRenderGeos() const
{
	eastl::stack<RE_Component*> ret;

	eastl::stack<const RE_GameObject*> gos;
	for (auto child : GetChildsPtr())
		if (child->IsActive())
			gos.push(child);

	while (!gos.empty())
	{
		const RE_GameObject* go = gos.top();
		gos.pop();

		if (go->render_geo.uid)
			ret.push(CompPtr(render_geo));

		for (auto child : go->GetChildsPtr())
			if (child->IsActive())
				gos.push(child);
	}

	return ret;
}

eastl::stack<RE_Component*> RE_GameObject::GetAllChildsActiveRenderGeos(const UID stencil_mask) const
{
	eastl::stack<RE_Component*> ret;

	eastl::stack<const RE_GameObject*> gos;
	for (auto child : GetChildsPtr())
		if (child->IsActive())
			gos.push(child);

	while (!gos.empty())
	{
		const RE_GameObject* go = gos.top();
		gos.pop();

		if (go->go_uid != stencil_mask && go->render_geo.uid)
			ret.push(CompPtr(render_geo));

		for (auto child : go->GetChildsPtr())
			if (child->IsActive())
				gos.push(child);
	}

	return ret;
}

void RE_GameObject::ReportComponent(const UID id, const ushortint type)
{
	RE_ASSERT(id > 0);
	switch (static_cast<ComponentType>(type))
	{
	case ComponentType::C_TRANSFORM: transform = id; break;
	case ComponentType::C_CAMERA: camera = id; break;
	case ComponentType::C_LIGHT: light = id; break;
	default:
	{
		if (IsRenderGeo(type)) render_geo = { id, type };
		else components.push_back({ id, type });
		break;
	}
	}
}

RE_Component* RE_GameObject::AddNewComponent(const ushortint type)
{
	RE_Component* ret = nullptr;
	ComponentType _type = static_cast<ComponentType>(type);
	RE_ASSERT(_type < MAX_COMPONENT_TYPES);

	switch (_type)
	{
	case C_TRANSFORM:
	{
		if (transform) pool_comps->DestroyComponent(_type, transform);
		transform = (ret = pool_comps->GetNewComponentPtr(_type))->PoolSetUp(pool_gos, go_uid);
		break;
	}
	case C_MESH:
	case C_WATER:
	case C_PARTICLEEMITER:
	{
		if (render_geo.uid) pool_comps->DestroyComponent(static_cast<ComponentType>(render_geo.type), render_geo.uid);
		render_geo = { (ret = pool_comps->GetNewComponentPtr(_type))->PoolSetUp(pool_gos, go_uid), type };
		break;
	}
	case C_CAMERA:
	{
		if (camera) pool_comps->DestroyComponent(_type, camera);
		camera = (ret = pool_comps->GetNewComponentPtr(_type))->PoolSetUp(pool_gos, go_uid);
		RE_CompCamera* new_cam = dynamic_cast<RE_CompCamera*>(ret);
		new_cam->SetProperties();
		if (!RE_INPUT->Paused()) RE_SCENE->cams->AddMainCamera(new_cam);
		break;
	}
	case C_LIGHT:
	{
		if (light) pool_comps->DestroyComponent(_type, light);
		light = (ret = pool_comps->GetNewComponentPtr(_type))->PoolSetUp(pool_gos, go_uid);
		break;
	}
	default:
	{
		if (IsRenderGeo(type))
		{
			if (render_geo.uid) pool_comps->DestroyComponent(static_cast<ComponentType>(render_geo.type), render_geo.uid);
			ret = pool_comps->GetNewComponentPtr(_type);
			render_geo = { ret->PoolSetUp(pool_gos, go_uid), type };
			RE_SCENE->primitives->SetUpComponentPrimitive(dynamic_cast<RE_CompPrimitive*>(ret));
		}
		else
		{
			components.push_back({ (ret = pool_comps->GetNewComponentPtr(_type))->PoolSetUp(pool_gos, go_uid), type });
		}
		break;
	}
	}

	return ret;
}

void RE_GameObject::ReleaseComponent(const UID id, const ushortint type)
{
	ComponentType _type = static_cast<ComponentType>(type);
	RE_ASSERT(_type < MAX_COMPONENT_TYPES);
	switch (_type)
	{
	case C_TRANSFORM: transform = 0; break;
	case C_CAMERA: camera = 0; break;
	case C_LIGHT: light = 0; break;
	// TODO: particle emitter case C_PARTICLEEMITER: break;
	default:
	{
		if (IsRenderGeo(static_cast<ushortint>(type)))
		{
			render_geo = { 0, 0 };
		}
		else
		{
			unsigned int count = 0u;
			for (auto comp : components)
			{
				if (comp.uid == id)
				{
					components.erase(&components[count]);
					break;
				}

				count++;
			}
		}

		break;
	}
	}
}

void RE_GameObject::DestroyComponent(const UID id, const ushortint type)
{
	ComponentType _type = static_cast<ComponentType>(type);
	RE_ASSERT(_type < MAX_COMPONENT_TYPES);
	switch (_type)
	{
	case C_TRANSFORM:
	{
		if (transform) pool_comps->DestroyComponent(_type, transform);
		transform = 0;
		break;
	}
	case C_CAMERA:
	{
		if (camera) pool_comps->DestroyComponent(_type, camera);
		camera = 0;
		break;
	}
	case C_LIGHT:
	{
		if (light) pool_comps->DestroyComponent(_type, light);
		light = 0;
		break;
	}
	default:
	{
		if (IsRenderGeo(static_cast<ushortint>(type)))
		{
			if (render_geo.uid) pool_comps->DestroyComponent(static_cast<ComponentType>(render_geo.type), render_geo.uid);
			render_geo = { 0, 0 };
		}
		else
		{
			unsigned int count = 0u;
			for (auto comp : components)
			{
				if (comp.uid == id)
				{
					pool_comps->DestroyComponent(_type, id);
					components.erase(&components[count]);
					break;
				}

				count++;
			}
		}

		break;
	}
	}
}

const eastl::vector<UID>& RE_GameObject::GetChilds() const { return childs; }

eastl::list<RE_GameObject*> RE_GameObject::GetChildsPtr() const
{
	eastl::list<RE_GameObject*> ret;
	for (auto child : childs) ret.push_back(pool_gos->AtPtr(child));
	return ret;
}
eastl::list<RE_GameObject*> RE_GameObject::GetChildsPtrReversed() const
{
	eastl::list<RE_GameObject*> ret;
	for (auto child : childs) ret.push_front(pool_gos->AtPtr(child));
	return ret;
}
eastl::list<const RE_GameObject*> RE_GameObject::GetChildsCPtr() const
{
	eastl::list<const RE_GameObject*> ret;
	for (auto child : childs) ret.push_back(pool_gos->AtCPtr(child));
	return ret;
}

eastl::vector<UID> RE_GameObject::GetGOandChilds() const
{
	eastl::vector<UID> ret;
	ret.push_back(go_uid);
	eastl::stack<UID> gos;
	for (auto child : childs) gos.push(child);

	while (!gos.empty()) {
		UID go = gos.top();
		gos.pop();

		ret.push_back(go);

		RE_GameObject* goPtr = pool_gos->AtPtr(go);
		for (auto child : goPtr->childs) gos.push(child);
	}

	return ret;
}

eastl::vector<RE_GameObject*> RE_GameObject::GetGOandChildsPtr()
{
	eastl::vector<RE_GameObject*> ret;
	ret.push_back(this);
	eastl::stack<UID> gos;
	for (auto child : childs) gos.push(child);

	while (!gos.empty()) {
		UID go = gos.top();
		gos.pop();

		RE_GameObject* goPtr = pool_gos->AtPtr(go);
		ret.push_back(goPtr);

		for (auto child : goPtr->childs) gos.push(child);
	}

	return ret;
}

eastl::vector<const RE_GameObject*> RE_GameObject::GetGOandChildsCPtr() const
{
	eastl::vector<const RE_GameObject*> ret;
	ret.push_back(this);
	eastl::stack<UID> gos;
	for (auto child : childs) gos.push(child);

	while (!gos.empty()) {
		UID go = gos.top();
		gos.pop();

		const RE_GameObject* goPtr = pool_gos->AtCPtr(go);
		ret.push_back(goPtr);

		for (auto child : goPtr->childs) gos.push(child);
	}

	return ret;
}

eastl::vector<RE_GameObject*> RE_GameObject::GetActiveDrawableChilds() const
{
	eastl::vector<RE_GameObject*> ret;

	eastl::queue<RE_GameObject*> go_queue;
	for (auto child : GetChildsPtr())
		if (child->active)
			go_queue.push(child);

	while (!go_queue.empty())
	{
		RE_GameObject* go = go_queue.front();
		go_queue.pop();
		if (go->HasActiveRenderGeo()) ret.push_back(go);

		for (auto child : GetChildsPtr())
			if (child->active)
				go_queue.push(child);
	}

	return ret;
}

eastl::vector<RE_GameObject*> RE_GameObject::GetActiveDrawableGOandChildsPtr()
{
	eastl::vector<RE_GameObject*> ret;

	eastl::queue<RE_GameObject*> go_queue;
	go_queue.push(this);

	while (!go_queue.empty())
	{
		RE_GameObject* go = go_queue.front();
		go_queue.pop();

		if (go->HasActiveRenderGeo()) ret.push_back(go);

		for (auto child : go->GetChildsPtr()) go_queue.push(child);
	}

	return ret;
}

eastl::vector<const RE_GameObject*> RE_GameObject::GetActiveDrawableGOandChildsCPtr() const
{
	eastl::vector<const RE_GameObject*> ret;

	eastl::queue<const RE_GameObject*> go_queue;
	if (active) go_queue.push(this);

	while (!go_queue.empty())
	{
		const RE_GameObject* go = go_queue.front();
		go_queue.pop();
		if (go->HasActiveRenderGeo()) ret.push_back(go);

		for (auto child : go->GetChildsCPtr())
			if (child->active)
				go_queue.push(child);
	}

	return ret;
}

const UID RE_GameObject::GetFirstChildUID() const
{
	return childs.empty() ? 0 : childs.front();
}

RE_GameObject* RE_GameObject::GetFirstChildPtr() const
{
	return childs.empty() ? nullptr : pool_gos->AtPtr(childs.front());
}

const RE_GameObject* RE_GameObject::GetFirstChildCPtr() const
{
	return childs.empty() ? nullptr : pool_gos->AtCPtr(childs.front());
}

const UID RE_GameObject::GetLastChildUID() const
{
	return childs.empty() ? 0 : childs.back();
}

RE_GameObject* RE_GameObject::GetLastChildPtr() const
{
	return childs.empty() ? nullptr : pool_gos->AtPtr(childs.back());
}

const RE_GameObject* RE_GameObject::GetLastChildCPtr() const
{
	return childs.empty() ? nullptr : pool_gos->AtCPtr(childs.back());
}

RE_GameObject* RE_GameObject::AddNewChild(const char* _name, const bool start_active, const bool isStatic)
{
	RE_GameObject* ret = nullptr;

	UID child = pool_gos->GetNewGOUID();
	if (child)
	{
		childs.push_back(child);
		(ret = ChildPtr(child))->SetUp(pool_gos, pool_comps,
			_name ? ("Son " + eastl::to_string(childs.size()) + " of " + name).c_str() : _name,
			start_active, isStatic);
	}

	return ret;
}

void RE_GameObject::ReleaseChild(const UID id)
{
	unsigned int count = 0u;
	for (auto child : childs)
	{
		if (child == id)
		{
			childs.erase(&childs[count]);
			break;
		}
		else count++;
	}
}

void RE_GameObject::DestroyChild(const UID id)
{
	unsigned int count = 0u;
	for (auto child : childs)
	{
		if (child == id)
		{
			pool_gos->DeleteGO(id);
			childs.erase(&childs[count]);
			break;
		}

		count++;
	}
}

unsigned int RE_GameObject::ChildCount() const { return childs.size(); }
bool RE_GameObject::IsLastChild() const { return parent_uid && (GetParentCPtr()->GetLastChildUID() == go_uid); }
bool RE_GameObject::isParent(UID parent) const
{
	bool ret = false;
	if (parent_uid){
		if(parent == parent_uid) ret = true;
		else ret = GetParentPtr()->isParent(parent);
	}
	return ret;
}
UID RE_GameObject::GetParentUID() const { return parent_uid; }
RE_GameObject* RE_GameObject::GetParentPtr() const { return pool_gos->AtPtr(parent_uid); }
const RE_GameObject* RE_GameObject::GetParentCPtr() const { return pool_gos->AtCPtr(parent_uid); }

void RE_GameObject::SetParent(const UID id, bool unlink_previous, bool link_new)
{
	if (unlink_previous && parent_uid) GetParentPtr()->ReleaseChild(go_uid);
	parent_uid = id;
	if (link_new && parent_uid) GetParentPtr()->childs.push_back(go_uid);
}

void RE_GameObject::UnlinkParent()
{
	if (parent_uid) GetParentPtr()->ReleaseChild(go_uid);
	parent_uid = 0;
}

UID RE_GameObject::GetRootUID() const { return pool_gos->GetRootUID(); }
RE_GameObject* RE_GameObject::GetRootPtr() const { return pool_gos->GetRootPtr(); }
const RE_GameObject* RE_GameObject::GetRootCPtr() const { return pool_gos->GetRootCPtr(); }

bool RE_GameObject::IsActive() const { return active; }
bool RE_GameObject::IsStatic() const { return isStatic; }
bool RE_GameObject::IsActiveStatic() const { return active && isStatic; }
bool RE_GameObject::IsActiveNonStatic() const { return active && !isStatic; }

void RE_GameObject::SetActive(const bool value, const bool broadcast)
{
	if (active != value)
	{
		active = value;
		if (broadcast) RE_INPUT->Push(active ? GO_CHANGED_TO_ACTIVE : GO_CHANGED_TO_INACTIVE, RE_SCENE, go_uid);
	}
}

void RE_GameObject::SetActiveWithChilds(bool val, bool broadcast)
{
	bool tmp = active;
	eastl::stack<RE_GameObject*> gos;
	gos.push(this);

	while (!gos.empty())
	{
		RE_GameObject* go = gos.top();
		go->SetActive(val, broadcast);
		gos.pop();
		for (auto child : go->childs) gos.push(ChildPtr(child));
	}

	active = tmp;
}

void RE_GameObject::SetStatic(const bool value, bool broadcast)
{
	if (isStatic != value)
	{
		isStatic = value;
		if (active && broadcast) RE_INPUT->Push(isStatic ? GO_CHANGED_TO_STATIC : GO_CHANGED_TO_NON_STATIC, RE_SCENE, go_uid);
	}
}

void RE_GameObject::SetStaticWithChilds(bool val, bool broadcast)
{
	eastl::stack<RE_GameObject*> gos;
	gos.push(this);
	while (!gos.empty())
	{
		RE_GameObject* go = gos.top();
		go->SetStatic(val, broadcast);
		gos.pop();
		for (auto child : go->childs) gos.push(ChildPtr(child));
	}
}

void RE_GameObject::OnPlay()
{
	for (auto component : GetComponentsPtr()) component->OnPlay();
	for (auto child : childs) ChildPtr(child)->OnPlay();
}

void RE_GameObject::OnPause()
{
	for (auto component : GetComponentsPtr()) component->OnPause();
	for (auto child : childs) ChildPtr(child)->OnPause();
}

void RE_GameObject::OnStop()
{
	for (auto component : GetComponentsPtr()) component->OnStop();
	for (auto child : childs) ChildPtr(child)->OnStop();
}

void RE_GameObject::OnTransformModified()
{
	eastl::queue<RE_GameObject*> gos;
	if (active) gos.push(this);

	while (!gos.empty())
	{
		RE_GameObject* go = gos.front();
		gos.pop();

		// Update components
		if (go->camera) go->CompPtr(go->camera, C_CAMERA)->OnTransformModified();
		for (auto component : go->GetStackableComponentsPtr()) component->OnTransformModified();

		// Add active childs to queue
		for (auto child : go->GetChildsPtr())
			if (child->active)
				gos.push(child);
	}
}

void RE_GameObject::AddToBoundingBox(math::AABB box)
{
	local_bounding_box.Enclose(box);
}

math::AABB RE_GameObject::GetGlobalBoundingBoxWithChilds() const
{
	math::AABB ret;

	if (render_geo.uid) ret = global_bounding_box;
	else ret.SetFromCenterAndSize(math::vec::zero, math::vec::zero);

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
			math::AABB child_aabb = ChildCPtr(child)->GetGlobalBoundingBoxWithChilds();

			points[cursor++].Set(child_aabb.minPoint.x, child_aabb.minPoint.y, child_aabb.minPoint.z);
			points[cursor++].Set(child_aabb.maxPoint.x, child_aabb.maxPoint.y, child_aabb.maxPoint.z);
		}

		// Enclose stored points
		ret.SetFrom(&points[0], points.size());
	}

	return ret;
}

bool RE_GameObject::CheckRayCollision(const math::Ray& global_ray, float& distance) const
{
	bool ret = false;

	if (render_geo.uid)
	{
		math::Ray local_ray = global_ray;
		local_ray.Transform(GetTransformPtr()->GetGlobalMatrix().Transposed().Inverted());

		switch (render_geo.type) {
		case C_MESH:
			ret = dynamic_cast<RE_CompMesh*>(CompPtr(render_geo))->CheckFaceCollision(local_ray, distance);
			break;
		case C_WATER:
			ret = dynamic_cast<RE_CompWater*>(CompPtr(render_geo))->CheckFaceCollision(local_ray, distance);
			break;		
		default:
			ret = dynamic_cast<RE_CompPrimitive*>(CompPtr(render_geo))->CheckFaceCollision(local_ray, distance);
			break;
		}
	}

	return ret;
}

void RE_GameObject::UseResources()
{
	eastl::stack<RE_Component*> cmps = GetAllChildsComponents(C_WATER);
	while (!cmps.empty()) {
		cmps.top()->UseResources();
		cmps.pop();
	}
	cmps = GetAllChildsComponents(C_MESH);
	while (!cmps.empty()) {
		cmps.top()->UseResources();
		cmps.pop();
	}
	cmps = GetAllChildsComponents(C_CAMERA);
	while (!cmps.empty()) {
		cmps.top()->UseResources();
		cmps.pop();
	}
}

void RE_GameObject::UnUseResources()
{
	eastl::stack<RE_Component*> cmps = GetAllChildsComponents(C_WATER);
	while (!cmps.empty()) {
		cmps.top()->UnUseResources();
		cmps.pop();
	}
	cmps = GetAllChildsComponents(C_MESH);
	while (!cmps.empty()) {
		cmps.top()->UnUseResources();
		cmps.pop();
	}
	cmps = GetAllChildsComponents(C_CAMERA);
	while (!cmps.empty()) {
		cmps.top()->UnUseResources();
		cmps.pop();
	}
}

void RE_GameObject::ResetLocalBoundingBox()
{
	if (render_geo.uid)
	{
		switch (render_geo.type) {
		case C_MESH:		 local_bounding_box = dynamic_cast<RE_CompMesh*>(CompPtr(render_geo))->GetAABB(); break;
		case C_WATER:		 local_bounding_box = dynamic_cast<RE_CompWater*>(CompPtr(render_geo))->GetAABB(); break;
		case C_GRID: {
			float dist = dynamic_cast<RE_CompGrid*>(CompPtr(render_geo))->GetDistance();
			local_bounding_box.FromCenterAndSize(math::vec::zero, { dist , 1.0f, dist }); 
			break;
		}
		case C_CUBE:		 local_bounding_box.FromCenterAndSize(math::vec::one * 0.5f, math::vec::one * 0.5f); break;
		case C_DODECAHEDRON: local_bounding_box.FromCenterAndSize(math::vec::one * 0.5f, math::vec::one * 0.5f); break;
		case C_TETRAHEDRON:	 local_bounding_box.FromCenterAndSize(math::vec::one * 0.5f, math::vec::one * 0.5f); break;
		case C_OCTOHEDRON:	 local_bounding_box.FromCenterAndSize(math::vec::one * 0.5f, math::vec::one * 0.5f); break;
		case C_ICOSAHEDRON:	 local_bounding_box.FromCenterAndSize(math::vec::one * 0.5f, math::vec::one * 0.5f); break;
		case C_PLANE:		 /*local_bounding_box.Enclose(math::AABB(math::vec::zero, math::vec::one));*/ break;
		case C_FUSTRUM:		 /*local_bounding_box.Enclose(math::AABB(math::vec::zero, math::vec::one));*/ break;
		case C_SPHERE:		 local_bounding_box.FromCenterAndSize(math::vec::zero, math::vec::one); break;
		case C_CYLINDER:	 local_bounding_box.FromCenterAndSize(math::vec::zero, math::vec::one); break;
		case C_HEMISHPERE:	 local_bounding_box.FromCenterAndSize(math::vec::zero, math::vec::one); break;
		case C_TORUS:		 local_bounding_box.FromCenterAndSize(math::vec::zero, math::vec::one); break;
		case C_TREFOILKNOT:	 local_bounding_box.FromCenterAndSize(math::vec::zero, math::vec::one); break;
		case C_ROCK:		 local_bounding_box.FromCenterAndSize(math::vec::zero, math::vec::one); break; }
	}
	else local_bounding_box = { math::vec::zero, math::vec::zero };
}

void RE_GameObject::ResetGlobalBoundingBox()
{
	// Global Bounding Box
	global_bounding_box = local_bounding_box;
	global_bounding_box.TransformAsAABB(GetTransformPtr()->GetGlobalMatrix().Transposed());
}

void RE_GameObject::ResetBoundingBoxes()
{
	ResetLocalBoundingBox();
	ResetGlobalBoundingBox();
}

void RE_GameObject::ResetGOandChildsAABB()
{
	// Push queue of GO together with parent's global matrix
	eastl::queue<eastl::pair<RE_GameObject*, math::float4x4>> go_queue;
	go_queue.push({ this, parent_uid ? GetParentCPtr()->GetTransformPtr()->GetGlobalMatrix() : math::float4x4::identity });
	while (!go_queue.empty())
	{
		eastl::pair<RE_GameObject*, math::float4x4> go = go_queue.front();
		go_queue.pop();

		// Add Childs to queue with parent's new global matrix
		math::float4x4 go_mat = go.first->GetTransformPtr()->UpdateGlobalMatrixFromParent(go.second);
		for (auto child : go.first->GetChildsPtr()) go_queue.push({ child, go_mat });

		// Update AABB with parent matrix
		go.first->ResetLocalBoundingBox();
		(go.first->global_bounding_box = go.first->local_bounding_box).TransformAsAABB(go_mat.Transposed());
	}
}

void RE_GameObject::DrawAABB(math::vec color) const
{
	RE_GLCache::ChangeShader(0);

	glMatrixMode(GL_MODELVIEW);
	glLoadMatrixf((dynamic_cast<RE_CompTransform*>(CompPtr(transform, C_TRANSFORM))->GetGlobalMatrix() * RE_CameraManager::CurrentCamera()->GetView()).ptr());

	glColor3f(color.x, color.y, color.z);
	glBegin(GL_LINES);

	for (uint i = 0; i < 12; i++)
	{
		glVertex3f(local_bounding_box.Edge(i).a.x, local_bounding_box.Edge(i).a.y, local_bounding_box.Edge(i).a.z);
		glVertex3f(local_bounding_box.Edge(i).b.x, local_bounding_box.Edge(i).b.y, local_bounding_box.Edge(i).b.z);
	}

	glEnd();
}

void RE_GameObject::DrawGlobalAABB() const
{
	for (uint i = 0; i < 12; i++)
	{
		glVertex3f(global_bounding_box.Edge(i).a.x, global_bounding_box.Edge(i).a.y, global_bounding_box.Edge(i).a.z);
		glVertex3f(global_bounding_box.Edge(i).b.x, global_bounding_box.Edge(i).b.y, global_bounding_box.Edge(i).b.z);
	}
}

math::AABB RE_GameObject::GetLocalBoundingBox() const { return local_bounding_box; }
math::AABB RE_GameObject::GetGlobalBoundingBox() const { return global_bounding_box; }

unsigned int RE_GameObject::GetBinarySize()const
{
	uint size = (sizeof(float) * 9) + sizeof(uint) + name.length() + sizeof(UID);
	return size;
}

void RE_GameObject::SerializeJson(RE_Json * node)
{
	node->PushString("name", name.c_str());
	if (parent_uid) node->PushUnsignedLongLong("Parent Pool ID", parent_uid);

	const RE_CompTransform* t = dynamic_cast<RE_CompTransform*>(CompPtr(transform, C_TRANSFORM));
	node->PushFloatVector("position", t->GetLocalPosition());
	node->PushFloatVector("rotation", t->GetLocalEulerRotation());
	node->PushFloatVector("scale", t->GetLocalScale());
}

void RE_GameObject::DeserializeJSON(RE_Json* node, GameObjectsPool* goPool, ComponentsPool* cmpsPool)
{
	SetUp(goPool, cmpsPool, node->PullString("name", "GameObject"), node->PullUnsignedLongLong("Parent Pool ID", 0));

	RE_CompTransform* t = dynamic_cast<RE_CompTransform*>(CompPtr(transform, C_TRANSFORM));
	t->SetPosition(node->PullFloatVector("position", math::vec::zero));
	t->SetRotation(node->PullFloatVector("rotation", math::vec::zero));
	t->SetScale(node->PullFloatVector("scale", math::vec::one));
}

void RE_GameObject::SerializeBinary(char*& cursor)
{
	uint size = sizeof(uint);
	uint strLenght = name.length();
	memcpy(cursor, &strLenght, size);
	cursor += size;
	size = sizeof(char) * strLenght;
	memcpy(cursor, name.c_str(), size);
	cursor += size;

	size = sizeof(UID);
	memcpy(cursor, &parent_uid, size);
	cursor += size;

	size = sizeof(float) * 3;
	RE_CompTransform* t = dynamic_cast<RE_CompTransform*>(CompPtr(transform, C_TRANSFORM));
	memcpy(cursor, &t->GetLocalPosition()[0], size);
	cursor += size;
	memcpy(cursor, &t->GetLocalEulerRotation()[0], size);
	cursor += size;
	memcpy(cursor, &t->GetLocalScale()[0], size);
	cursor += size;
}

void RE_GameObject::DeserializeBinary(char*& cursor, GameObjectsPool* goPool, ComponentsPool* compPool)
{
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
	char nullchar = '\0';
	memcpy(strNameCursor, &nullchar, sizeof(char));

	size = sizeof(UID);
	UID goParentID;
	memcpy(&goParentID, cursor, size);
	cursor += size;
	SetUp(goPool, compPool, strName, goParentID);
	DEL_A(strName);

	float vec[3] = { 0.0, 0.0, 0.0 };
	size = sizeof(float) * 3;
	RE_CompTransform* t = dynamic_cast<RE_CompTransform*>(CompPtr(transform, C_TRANSFORM));
	memcpy(vec, cursor, size);
	cursor += size;
	t->SetPosition(math::vec(vec));
	memcpy(vec, cursor, size);
	cursor += size;
	t->SetRotation(math::vec(vec));
	memcpy(vec, cursor, size);
	cursor += size;
	t->SetScale(math::vec(vec));
}

UID RE_GameObject::GetUID() const
{
	return go_uid;
}

inline RE_Component* RE_GameObject::CompPtr(ComponentData comp) const
{
	return pool_comps->GetComponentPtr(comp.uid, static_cast<ComponentType>(comp.type));
}

inline RE_Component* RE_GameObject::CompPtr(UID id, ushortint type) const
{
	return pool_comps->GetComponentPtr(id, static_cast<ComponentType>(type));
}

inline const RE_Component* RE_GameObject::CompCPtr(ComponentData comp) const
{
	return pool_comps->GetComponentCPtr(comp.uid, static_cast<ComponentType>(comp.type));
}

inline const RE_Component* RE_GameObject::CompCPtr(UID id, ushortint type) const
{
	return pool_comps->GetComponentCPtr(id, static_cast<ComponentType>(type));
}

eastl::list<RE_GameObject::ComponentData> RE_GameObject::AllCompData() const
{
	eastl::list<ComponentData> ret;

	ret.push_back({ transform, ComponentType::C_TRANSFORM });
	if (render_geo.uid) ret.push_back(render_geo);
	if (camera) ret.push_back({ camera, ComponentType::C_CAMERA });
	if (light) ret.push_back({ light, ComponentType::C_LIGHT });
	for (auto comp : components) ret.push_back(comp);

	return ret;
}

inline RE_GameObject* RE_GameObject::ChildPtr(const UID child) const
{
	return pool_gos->AtPtr(child);
}

inline const RE_GameObject* RE_GameObject::ChildCPtr(const UID child) const
{
	return pool_gos->AtCPtr(child);
}

inline bool RE_GameObject::IsRenderGeo(ushortint type) const
{
	return (type == ComponentType::C_MESH || type == ComponentType::C_WATER || type == ComponentType::C_PARTICLEEMITER || (type > ComponentType::C_PRIMIVE_MIN && type < ComponentType::C_PRIMIVE_MAX));
}
