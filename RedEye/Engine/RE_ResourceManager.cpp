#include "Event.h"

#include "RE_ResourceManager.h"

#include "RE_Memory.h"
#include "RE_Profiler.h"
#include "Application.h"
#include "RE_FileSystem.h"
#include "ModuleInput.h"
#include "ModuleScene.h"
#include "ModuleEditor.h"
#include "ModuleRenderer3D.h"
#include "ModuleAudio.h"
#include "RE_InternalResources.h"
#include "RE_ShaderImporter.h"
#include "RE_TextureImporter.h"
#include "RE_ModelImporter.h"
#include "RE_ThumbnailManager.h"
#include "RE_ECS_Pool.h"
#include "RE_Material.h"
#include "RE_Shader.h"
#include "RE_Prefab.h"
#include "RE_Model.h"
#include "RE_Scene.h"
#include "RE_SkyBox.h"
#include "RE_Texture.h"
#include "RE_Mesh.h"
#include "RE_ParticleEmitterBase.h"
#include "RE_ParticleEmission.h"
#include "RE_ParticleRender.h"

#include <EASTL/internal/char_traits.h>
#include <EASTL/string.h>

void RE_ResourceManager::Init()
{
	RE_PROFILE(RE_ProfiledFunc::Init, RE_ProfiledClass::ResourcesManager);
	RE_LOG_SEPARATOR("Initializing Resources");

	RE_TextureImporter::Init();
	RE_ShaderImporter::Init();
	RE_InternalResources::Init();

	// Fetch Assets
	RE_FS->ReadAssetChanges(0, true);
	RE_AUDIO->ReadBanksChanges();
}

void RE_ResourceManager::Clear()
{
	RE_PROFILE(RE_ProfiledFunc::Clear, RE_ProfiledClass::ResourcesManager);
	RE_InternalResources::Clear();
	RE_ShaderImporter::Clear();

	while (!resources.empty())
	{
		DEL(resources.begin()->second)
		resources.erase(resources.begin());
	}
}

void RE_ResourceManager::RecieveEvent(const Event& e)
{
	if (e.type == RE_EventType::RESOURCE_CHANGED)
	{
		ResourceContainer* res = resources.at(e.data1.AsCharP());
		if (res->GetType() == ResourceContainer::Type::SHADER)
		{
			eastl::vector<ResourceContainer*> materials = GetResourcesByType(ResourceContainer::Type::MATERIAL);
			for (auto material : materials) material->SomeResourceChanged(res->GetMD5());
		}
	}
}

const char* RE_ResourceManager::Reference(ResourceContainer* rc)
{
	const char* retMD5 = nullptr;
	if (rc)
	{
		resources.insert(Resource(retMD5 = rc->GetMD5(), rc));
		resourcesCounter.insert(ResourceCounter(retMD5, (rc->isInMemory()) ? 1 : 0));
		RE_LOG("%s referenced: %s\n\tAsset path: %s\n\tLibrary path: %s\n\tGenerated MD5: %s",
			GetNameFromType(rc->GetType()), rc->GetName(), rc->GetAssetPath(), rc->GetLibraryPath(), rc->GetMD5());
	}
	return retMD5;
}

void RE_ResourceManager::Use(const char* resMD5)
{
	if (resourcesCounter.at(resMD5) == 0) resources.at(resMD5)->LoadInMemory();
	resourcesCounter.at(resMD5)++;
}

void RE_ResourceManager::UnUse(const char* resMD5)
{
	if (--resourcesCounter.at(resMD5) == 0) resources.at(resMD5)->UnloadMemory();
	else if (resourcesCounter.at(resMD5) < 0)
	{
		RE_LOG_WARNING("UnUse of resource already with no uses. Resource %s.",resources.at(resMD5)->GetName());
		if(resources.at(resMD5)->isInMemory()) resources.at(resMD5)->UnloadMemory();
		resourcesCounter.at(resMD5) = 0;
	}
}

void RE_ResourceManager::PushSelected(const char* resS, bool popAll)
{
	if (popAll)PopSelected(true);
	resourcesSelected.push(resS);
}

const char* RE_ResourceManager::GetSelected() const
{
	return (resourcesSelected.empty()) ? nullptr : resourcesSelected.top();
}

void RE_ResourceManager::PopSelected(bool all)
{
	if (resourcesSelected.empty()) return;

	do resourcesSelected.pop();
	while (all && !resourcesSelected.empty());
}

ResourceContainer* RE_ResourceManager::At(const char* md5) const { return resources.at(md5); }

const char* RE_ResourceManager::ReferenceByMeta(const char* metaPath, ResourceContainer::Type type)
{
	ResourceContainer* newContainer = nullptr;
	switch (type)
	{
	case ResourceContainer::Type::SHADER:				newContainer = static_cast<ResourceContainer*>(new RE_Shader(metaPath));				break;
	case ResourceContainer::Type::TEXTURE:				newContainer = static_cast<ResourceContainer*>(new RE_Texture(metaPath));				break;
	case ResourceContainer::Type::PREFAB:				newContainer = static_cast<ResourceContainer*>(new RE_Prefab(metaPath));				break;
	case ResourceContainer::Type::SKYBOX:				newContainer = static_cast<ResourceContainer*>(new RE_SkyBox(metaPath));				break;
	case ResourceContainer::Type::MATERIAL:			newContainer = static_cast<ResourceContainer*>(new RE_Material(metaPath));				break;
	case ResourceContainer::Type::MODEL:				newContainer = static_cast<ResourceContainer*>(new RE_Model(metaPath));					break;
	case ResourceContainer::Type::SCENE:				newContainer = static_cast<ResourceContainer*>(new RE_Scene(metaPath));					break; 
	case ResourceContainer::Type::PARTICLE_EMITTER:	newContainer = static_cast<ResourceContainer*>(new RE_ParticleEmitterBase(metaPath));	break; 
	case ResourceContainer::Type::PARTICLE_EMISSION:	newContainer = static_cast<ResourceContainer*>(new RE_ParticleEmission(metaPath));		break; 
	case ResourceContainer::Type::PARTICLE_RENDER:		newContainer = static_cast<ResourceContainer*>(new RE_ParticleRender(metaPath));		break;
	default: break;
	}

	const char* retMD5 = nullptr;
	if (newContainer)
	{
		newContainer->LoadMeta();
		retMD5 = Reference(newContainer);
	}

	return retMD5;
}

size_t RE_ResourceManager::TotalReferences() const { return resources.size(); }

eastl::vector<const char*> RE_ResourceManager::GetAllResourcesActiveByType(ResourceContainer::Type resT)
{
	eastl::vector<ResourceContainer*> resourcesByType = GetResourcesByType(resT);
	eastl::vector<const char*> ret;

	while (!resourcesByType.empty())
	{
		const char* resMD5 = resourcesByType.back()->GetMD5();
		if (resourcesCounter.at(resMD5) > 0)
			ret.push_back(resMD5);
		resourcesByType.pop_back();
	}
	return ret;
}

eastl::vector<const char*> RE_ResourceManager::WhereUndefinedFileIsUsed(const char* assetPath)
{
	eastl::vector<const char*> shadersUsed;
	eastl::vector<ResourceContainer*> temp_resources = GetResourcesByType(ResourceContainer::Type::SHADER);

	RE_Shader* shader = nullptr;
	for (const auto& res : temp_resources)
	{
		shader = dynamic_cast<RE_Shader*>(res);
		if (!res->isInternal() && shader->IsPathOnShader(assetPath)) shadersUsed.push_back(res->GetMD5());
	}

	eastl::vector<const char*> ret;
	for (auto s : shadersUsed)
	{
		ret.push_back(s);
		eastl::vector<const char*> sret = WhereIsUsed(s);
		if (!sret.empty()) ret.insert(ret.end(), sret.begin(), sret.end());
	}

	return ret;
}

eastl::vector<const char*> RE_ResourceManager::WhereIsUsed(const char* res)
{
	eastl::vector<const char*> ret;
	eastl::vector<ResourceContainer*> temp_resources;
	eastl::vector<ResourceContainer*> temp;

	ResourceContainer* resource = resources.at(res);
	ResourceContainer::Type rType = resource->GetType();

	switch (rType)
	{
	case ResourceContainer::Type::SHADER: //search on materials
	case ResourceContainer::Type::TEXTURE: //search on materials. no scenes will be afected
	{
		temp_resources = GetResourcesByType(ResourceContainer::Type::MATERIAL);
		for (const auto& resource : temp_resources)
		{
			auto mat = dynamic_cast<const RE_Material*>(resource);
			if ((rType == ResourceContainer::Type::SHADER && mat->ExistsOnShader(res)) || mat->ExistsOnTexture(res))
				ret.push_back(resource->GetMD5());
		}

		break;
	}
	case ResourceContainer::Type::PARTICLE_EMISSION:
	case ResourceContainer::Type::PARTICLE_RENDER:
	{
		temp_resources = GetResourcesByType(ResourceContainer::Type::PARTICLE_EMITTER);
		for (const auto& resource : temp_resources)
			if (dynamic_cast<const RE_ParticleEmitterBase*>(resource)->Contains(res))
				ret.push_back(resource->GetMD5());
		break;
	}
	case ResourceContainer::Type::SKYBOX: //search on cameras from scenes or prefabs
	case ResourceContainer::Type::MATERIAL: //search on scenes, prefabs and models(models advise you need to reimport)
	case ResourceContainer::Type::PARTICLE_EMITTER: //search on scenes and prefabs
	{
		temp_resources = GetResourcesByType(ResourceContainer::Type::SCENE);
		temp = GetResourcesByType(ResourceContainer::Type::PREFAB);
		temp_resources.insert(temp_resources.end(), temp.begin(), temp.end());
		if (rType == ResourceContainer::Type::MATERIAL)
		{
			temp = GetResourcesByType(ResourceContainer::Type::MODEL);
			temp_resources.insert(temp_resources.end(), temp.begin(), temp.end());
		}

		RE_INPUT->PauseEvents();
		for (auto& resource : temp_resources)
		{
			RE_ECS_Pool* poolGORes = nullptr;
			Use(resource->GetMD5());
			switch (resource->GetType())
			{
			case ResourceContainer::Type::SCENE: poolGORes = dynamic_cast<RE_Scene*>(resource)->GetPool(); break;
			case ResourceContainer::Type::PREFAB: poolGORes = dynamic_cast<RE_Prefab*>(resource)->GetPool(); break;
			case ResourceContainer::Type::MODEL: poolGORes = dynamic_cast<RE_Model*>(resource)->GetPool(); break;
			default: break;
			}

			eastl::stack<RE_Component*> comps;
			if (poolGORes != nullptr)
			{
				switch (rType)
				{
				case ResourceContainer::Type::SKYBOX:			 comps = RE_SCENE->GetRootPtr()->GetAllChildsComponents(RE_Component::Type::CAMERA); break;
				case ResourceContainer::Type::MATERIAL:		 comps = RE_SCENE->GetRootPtr()->GetAllChildsComponents(RE_Component::Type::MESH); break;
				case ResourceContainer::Type::PARTICLE_EMITTER: comps = RE_SCENE->GetRootPtr()->GetAllChildsComponents(RE_Component::Type::PARTICLEEMITER); break;
				default: break;
				}

				bool skip = false;
				while (!comps.empty() && !skip)
				{
					switch (rType)
					{
					case ResourceContainer::Type::SKYBOX: break;
					case ResourceContainer::Type::MATERIAL:
					{
						RE_CompMesh* mesh = dynamic_cast<RE_CompMesh*>(comps.top());
						if (mesh && mesh->GetMaterial() == res)
						{
							ret.push_back(resource->GetMD5());
							skip = true;
						}
						break;
					}
					case ResourceContainer::Type::PARTICLE_EMITTER:
					{
						RE_CompParticleEmitter* emitter = dynamic_cast<RE_CompParticleEmitter*>(comps.top());
						if (emitter && emitter->GetEmitterResource() == res)
						{
							ret.push_back(resource->GetMD5());
							skip = true;
						}
						break;
					}
					}

					comps.pop();
				}
				UnUse(resource->GetMD5());
			}
		}
		RE_INPUT->ResumeEvents();
		break;
	}
	default: break;
	}
	
	return ret;
}

ResourceContainer* RE_ResourceManager::DeleteResource(const char* res, eastl::vector<const char*> resourcesWillChange, bool resourceOnScene)
{
	ResourceContainer* resource = resources.at(res);
	ResourceContainer::Type rType = resource->GetType();

	if (TotalReferenceCount(res) > 0) resource->UnloadMemory();

	switch (rType)
	{
	case ResourceContainer::Type::SHADER: //search on materials.
	case ResourceContainer::Type::TEXTURE: //search on materials. No will afect on scene because the textures are saved on meta
	{
		RE_Material* resChange = nullptr;
		for (auto resToChange : resourcesWillChange)
		{
			(resChange = dynamic_cast<RE_Material*>(RE_RES->At(resToChange)))->LoadInMemory();
			if(rType == ResourceContainer::Type::SHADER) resChange->DeleteShader();
			else resChange->DeleteTexture(res);
			resChange->UnloadMemory();
			RE_RENDER->PushThumnailRend(resToChange, true);
		}
		break;
	}
	case ResourceContainer::Type::SKYBOX: //search on cameras from scenes or prefabs
	case ResourceContainer::Type::MATERIAL: //search on scenes, prefabs and models(models advise you need to reimport)
	case ResourceContainer::Type::PARTICLE_EMITTER://search on particle emitters from scenes or prefabs
	{
		for (auto resToChange : resourcesWillChange)
		{
			ResourceContainer* resChange = RE_RES->At(resToChange);
			ResourceContainer::Type goType = resChange->GetType();

			if (resourceOnScene && resToChange == RE_SCENE->GetCurrentScene())
				continue;

			if (goType != ResourceContainer::Type::MODEL)
			{
				RE_INPUT->PauseEvents();

				RE_ECS_Pool* poolGORes = nullptr;
				Use(resToChange);

				switch (goType)
				{
				case ResourceContainer::Type::SCENE: poolGORes = dynamic_cast<RE_Scene*>(resChange)->GetPool(); break;
				case ResourceContainer::Type::PREFAB: poolGORes = dynamic_cast<RE_Prefab*>(resChange)->GetPool(); break;
				default: break;
				}

				if (poolGORes != nullptr)
				{
					eastl::stack<RE_Component*> comps = poolGORes->GetRootPtr()->GetAllChildsComponents(
						(rType == ResourceContainer::Type::MATERIAL) ? RE_Component::Type::MESH : RE_Component::Type::CAMERA);

					while (!comps.empty())
					{
						RE_Component* go = comps.top();
						comps.pop();

						switch (rType)
						{
						case ResourceContainer::Type::SKYBOX:
						{
							RE_CompCamera* cam = dynamic_cast<RE_CompCamera*>(go);
							if (cam && cam->Camera.isUsingSkybox() && cam->Camera.GetSkybox() == res) cam->Camera.SetSkyBox(nullptr);
							break;
						}
						case ResourceContainer::Type::MATERIAL:
						{
							RE_CompMesh* mesh = dynamic_cast<RE_CompMesh*>(go);
							if (mesh && mesh->GetMaterial() == res) mesh->SetMaterial(nullptr);
							break;
						}
						case ResourceContainer::Type::PARTICLE_EMITTER:
						{

							RE_CompParticleEmitter* emitter = dynamic_cast<RE_CompParticleEmitter*>(go);
							if (emitter && emitter->GetEmitterResource() == res) emitter->SetEmitter(nullptr);
							break;
						}
						default: break;
						}
					}

					poolGORes->GetRootPtr()->ResetGOandChildsAABB();

					switch (goType)
					{
					case ResourceContainer::Type::SCENE:
					{
						RE_Scene* s = dynamic_cast<RE_Scene*>(resChange);
						s->Save(poolGORes);
						s->SaveMeta();
						break;
					}
					case ResourceContainer::Type::PREFAB:
					{
						RE_Prefab* p = dynamic_cast<RE_Prefab*>(resChange);
						p->Save(poolGORes, false);
						p->SaveMeta();
						break;
					}
					default: break;
					}
					RE_RENDER->PushThumnailRend(resToChange, true);
					UnUse(resToChange);
					DEL(poolGORes)
					RE_INPUT->ResumeEvents();
				}
			}
		}

		break;
	}
	case ResourceContainer::Type::PARTICLE_EMISSION:
	case ResourceContainer::Type::PARTICLE_RENDER: // Search on particle emitters and then apply changes on scene if exists
	{
		RE_ParticleEmitterBase* resChange = nullptr;
		for (auto resToChange : resourcesWillChange)
		{
			bool need_update_scene = false;

			resChange = dynamic_cast<RE_ParticleEmitterBase*>(RE_RES->At(resToChange));

			if (TotalReferenceCount(resToChange) > 0) need_update_scene = true;

			if (rType == ResourceContainer::Type::PARTICLE_EMISSION) resChange->ChangeEmissor(nullptr, nullptr);
			else resChange->ChangeRenderer(nullptr, nullptr);
			
			resChange->SaveMeta();

			if (need_update_scene)
			{
				eastl::stack<RE_Component*> comps = RE_SCENE->GetRootPtr()->GetAllChildsComponents(RE_Component::Type::PARTICLEEMITER);

				while (!comps.empty())
				{
					RE_CompParticleEmitter* emitter = dynamic_cast<RE_CompParticleEmitter*>(comps.top());
					comps.pop();

					if (emitter && emitter->GetEmitterResource() == resToChange)
					{
						emitter->UnUseResources();
						emitter->UseResources();
					}
				}
			}
		}
		break;
	}
	default: break;
	}

	if (resourceOnScene)
	{
		eastl::stack<RE_Component*> comps;
		
		switch (rType)
		{
		case ResourceContainer::Type::SKYBOX: comps = RE_SCENE->GetRootPtr()->GetAllChildsComponents(RE_Component::Type::CAMERA); break;
		case ResourceContainer::Type::MATERIAL: comps = RE_SCENE->GetRootPtr()->GetAllChildsComponents(RE_Component::Type::MESH); break;
		case ResourceContainer::Type::PARTICLE_EMITTER: comps = RE_SCENE->GetRootPtr()->GetAllChildsComponents(RE_Component::Type::PARTICLEEMITER); break;
		default: break;
		}
		
		while (!comps.empty())
		{
			RE_Component* go = comps.top();
			comps.pop();

			switch (rType)
			{
			case ResourceContainer::Type::SKYBOX:
			{
				RE_CompCamera* cam = dynamic_cast<RE_CompCamera*>(go);
				if (cam && cam->Camera.GetSkybox() == res) cam->Camera.SetSkyBox(nullptr);
				break;
			}
			case ResourceContainer::Type::MATERIAL:
			{
				RE_CompMesh* mesh = dynamic_cast<RE_CompMesh*>(go);
				if (mesh && mesh->GetMaterial() == res) mesh->SetMaterial(nullptr);
				break;
			}
			case ResourceContainer::Type::PARTICLE_EMITTER:
			{
				RE_CompParticleEmitter* emitter = dynamic_cast<RE_CompParticleEmitter*>(go);

				if (emitter && emitter->GetEmitterResource() == res) {
					emitter->UnUseResources();
					emitter->SetEmitter(nullptr);
					emitter->UseResources();
				}

				break;
			}
			default: break;
			}
		}

		RE_SCENE->HasChanges();
	}

	resourcesCounter.erase(res);
	resources.erase(res);

	if (rType != ResourceContainer::Type::SHADER &&
		rType != ResourceContainer::Type::PARTICLE_EMITTER &&
		rType != ResourceContainer::Type::PARTICLE_EMISSION &&
		rType != ResourceContainer::Type::PARTICLE_RENDER) RE_EDITOR->thumbnails->Delete(res);

	return resource;
}

eastl::vector<ResourceContainer*> RE_ResourceManager::GetResourcesByType(ResourceContainer::Type type) const
{
	eastl::vector<ResourceContainer*> ret;

	for (auto& resource : resources)
		if (resource.second->GetType() == type)
			ret.push_back(resource.second);

	return ret;
}

const char* RE_ResourceManager::IsReference(const char* md5, ResourceContainer::Type type)
{
	const char* ret = nullptr;
	if (type == ResourceContainer::Type::UNDEFINED)
	{
		for (auto& resource : resources)
			if (eastl::Compare(resource.second->GetMD5(), md5, 32) == 0)
				ret = resource.first;
	}
	else
	{
		eastl::vector<ResourceContainer*> tResources = GetResourcesByType(type);
		for (auto resource : tResources)
			if (eastl::Compare(resource->GetMD5(), md5, 32) == 0)
				ret = resource->GetMD5();
	}
	return ret;
}

const char * RE_ResourceManager::FindMD5ByMETAPath(const char * metaPath, ResourceContainer::Type type)
{
	const char* ret = nullptr;
	size_t sizemeta = 0;
	if (type == ResourceContainer::Type::UNDEFINED)
	{
		for (auto resource : resources)
			if ((sizemeta = eastl::CharStrlen(resource.second->GetMetaPath())) > 0
				&& eastl::Compare(resource.second->GetMetaPath(), metaPath, sizemeta) == 0)
				ret = resource.first;
	}
	else
	{
		eastl::vector<ResourceContainer*> tResources = GetResourcesByType(type);
		for (auto resource : tResources)
			if ((sizemeta = eastl::CharStrlen(resource->GetMetaPath())) > 0
				&& eastl::Compare(resource->GetMetaPath(), metaPath, sizemeta) == 0)
				ret = resource->GetMD5();
	}
	return ret;
}

const char* RE_ResourceManager::FindMD5ByLibraryPath(const char* libraryPath, ResourceContainer::Type type)
{
	const char* ret = nullptr;
	size_t sizelibrary = 0;
	if (type == ResourceContainer::Type::UNDEFINED)
	{
		for (auto resource : resources)
			if ((sizelibrary = eastl::CharStrlen(resource.second->GetLibraryPath())) > 0
				&& eastl::Compare(resource.second->GetLibraryPath(), libraryPath, sizelibrary) == 0)
				ret = resource.first;
	}
	else
	{
		eastl::vector<ResourceContainer*> tResources = GetResourcesByType(type);
		for (auto resource : tResources)
		{
			sizelibrary = eastl::CharStrlen(resource->GetLibraryPath());
			if (sizelibrary > 0 && eastl::Compare(resource->GetLibraryPath(), libraryPath, sizelibrary) == 0)
				ret = resource->GetMD5();
		}
	}
	return ret;
}

const char * RE_ResourceManager::FindMD5ByAssetsPath(const char * assetsPath, ResourceContainer::Type type)
{
	const char* ret = nullptr;
	size_t sizeassets = 0;
	if (type == ResourceContainer::Type::UNDEFINED)
	{
		for (auto resource : resources)
			if ((sizeassets = eastl::CharStrlen(resource.second->GetAssetPath())) > 0
				&& eastl::Compare(resource.second->GetAssetPath(), assetsPath, sizeassets) == 0)
				ret = resource.first;
	}
	else
	{
		eastl::vector<ResourceContainer*> tResources = GetResourcesByType(type);
		for (auto resource : tResources)
			if ((sizeassets = eastl::CharStrlen(resource->GetAssetPath())) > 0 && eastl::Compare(resource->GetAssetPath(), assetsPath, sizeassets) == 0)
				ret = resource->GetMD5();
	}
	return ret;
}

const char* RE_ResourceManager::CheckOrFindMeshOnLibrary(const char* librariPath)
{
	const char* meshMD5 = nullptr;

	meshMD5 = FindMD5ByLibraryPath(librariPath, ResourceContainer::Type::MESH);

	if (!meshMD5 && RE_FS->Exists(librariPath))
	{
		RE_Mesh* newMesh = new RE_Mesh();
		newMesh->SetLibraryPath(librariPath);
		newMesh->SetType(ResourceContainer::Type::MESH);
		meshMD5 = Reference(newMesh);
	}

	if (!meshMD5) RE_LOG_ERROR("Error finding library mesh:\n%s", librariPath);

	return meshMD5;
}

bool RE_ResourceManager::isNeededResoursesLoaded(const char* metaPath, ResourceContainer::Type type) const
{
	bool ret = false;

	ResourceContainer* newContainer = nullptr;
	switch (type) {
	case ResourceContainer::Type::SHADER:			 newContainer = static_cast<ResourceContainer*>(new RE_Shader(metaPath));				break;
	case ResourceContainer::Type::TEXTURE:			 newContainer = static_cast<ResourceContainer*>(new RE_Texture(metaPath));				break;
	case ResourceContainer::Type::PREFAB:			 newContainer = static_cast<ResourceContainer*>(new RE_Prefab(metaPath));				break;
	case ResourceContainer::Type::SKYBOX:			 newContainer = static_cast<ResourceContainer*>(new RE_SkyBox(metaPath));				break;
	case ResourceContainer::Type::MATERIAL:			 newContainer = static_cast<ResourceContainer*>(new RE_Material(metaPath));				break;
	case ResourceContainer::Type::MODEL:			 newContainer = static_cast<ResourceContainer*>(new RE_Model(metaPath));				break;
	case ResourceContainer::Type::SCENE:			 newContainer = static_cast<ResourceContainer*>(new RE_Scene(metaPath));				break;
	case ResourceContainer::Type::PARTICLE_EMITTER:  newContainer = static_cast<ResourceContainer*>(new RE_ParticleEmitterBase(metaPath));	break;
	case ResourceContainer::Type::PARTICLE_EMISSION: newContainer = static_cast<ResourceContainer*>(new RE_ParticleEmission(metaPath));		break;
	case ResourceContainer::Type::PARTICLE_RENDER:	 newContainer = static_cast<ResourceContainer*>(new RE_ParticleRender(metaPath));		break;
	}

	if (newContainer)
	{
		ret = newContainer->CheckResourcesReferenced();
		DEL(newContainer)
	}

	return ret;
}

void RE_ResourceManager::ThumbnailResources()
{
	RE_PROFILE(RE_ProfiledFunc::ThumbnailResources, RE_ProfiledClass::ResourcesManager);
	for (const auto& res : resources)
	{
		ResourceContainer::Type rT = res.second->GetType();
		if( rT == ResourceContainer::Type::SCENE ||
			rT == ResourceContainer::Type::PREFAB ||
			rT == ResourceContainer::Type::MODEL ||
			rT == ResourceContainer::Type::SKYBOX ||
			rT == ResourceContainer::Type::MATERIAL ||
			rT == ResourceContainer::Type::TEXTURE)
			RE_RENDER->PushThumnailRend(res.first);
	}
}

void RE_ResourceManager::PushParticleResource(const char* md5)
{
	resources_particles_reimport.push(md5);
}

void RE_ResourceManager::ProcessParticlesReimport()
{
	if (!resources_particles_reimport.empty())
	{
		eastl::vector<ResourceContainer*> emitters = GetResourcesByType(ResourceContainer::Type::PARTICLE_EMITTER);

		while (!resources_particles_reimport.empty())
		{
			const char* res = resources_particles_reimport.top();
			resources_particles_reimport.pop();

			for (auto res_container : emitters)
				dynamic_cast<RE_ParticleEmitterBase*>(res_container)->SomeResourceChanged(res);
		}
	}
}

const char* RE_ResourceManager::GetNameFromType(const ResourceContainer::Type type)
{
	switch (type)
	{
	case ResourceContainer::Type::TEXTURE: return "Texture";
	case ResourceContainer::Type::SCENE: return "Scene";
	case ResourceContainer::Type::MATERIAL: return "Material";
	case ResourceContainer::Type::MESH: return "Mesh";
	case ResourceContainer::Type::PREFAB: return "Prefab";
	case ResourceContainer::Type::SHADER: return "Shader";
	case ResourceContainer::Type::MODEL: return "Model";
	case ResourceContainer::Type::SKYBOX: return "Skybox";
	case ResourceContainer::Type::PARTICLE_EMITTER: return "Particle emitter";
	case ResourceContainer::Type::PARTICLE_EMISSION: return "Particle emission";
	case ResourceContainer::Type::PARTICLE_RENDER: return "Particle render";
	case ResourceContainer::Type::UNDEFINED: return "Undefined";
	default: return "Invalid";
	}
}

const char* RE_ResourceManager::ImportModel(const char* assetPath)
{
	RE_INPUT->PauseEvents();
	eastl::string path(assetPath);
	eastl::string filename = path.substr(path.find_last_of("/") + 1);
	eastl::string name = filename.substr(0, filename.find_last_of("."));
	eastl::string extension = filename.substr(filename.find_last_of(".") + 1);

	RE_Model* newModel = new RE_Model();
	newModel->SetName(name.c_str());
	newModel->SetAssetPath(assetPath);
	newModel->SetType(ResourceContainer::Type::MODEL);
	newModel->Import(false);
	newModel->SaveMeta();
	RE_INPUT->ResumeEvents();

	return Reference(newModel);
}
const char* RE_ResourceManager::ImportTexture(const char* assetPath)
{
	eastl::string path(assetPath);
	eastl::string filename = path.substr(path.find_last_of("/") + 1);
	eastl::string name = filename.substr(0, filename.find_last_of("."));
	eastl::string extension = filename.substr(filename.find_last_of(".") + 1);

	RE_Texture* newTexture = new RE_Texture();
	newTexture->SetName(name.c_str());
	newTexture->SetAssetPath(assetPath);
	newTexture->SetType(ResourceContainer::Type::TEXTURE);
	newTexture->Import(false);
	newTexture->SaveMeta();

	return Reference(newTexture);
}

const char* RE_ResourceManager::ImportMaterial(const char* assetPath)
{
	eastl::string path(assetPath);
	eastl::string filename = path.substr(path.find_last_of("/") + 1);
	eastl::string name = filename.substr(0, filename.find_last_of("."));
	eastl::string extension = filename.substr(filename.find_last_of(".") + 1);

	RE_Material* newMaterial = new RE_Material();
	newMaterial->SetName(name.c_str());
	newMaterial->SetAssetPath(assetPath);
	newMaterial->SetType(ResourceContainer::Type::MATERIAL);
	newMaterial->Import(false);
	newMaterial->SaveMeta();

	return Reference(newMaterial);
}

const char* RE_ResourceManager::ImportSkyBox(const char* assetPath)
{
	eastl::string path(assetPath);
	eastl::string filename = path.substr(path.find_last_of("/") + 1);
	eastl::string name = filename.substr(0, filename.find_last_of("."));
	eastl::string extension = filename.substr(filename.find_last_of(".") + 1);

	RE_SkyBox* newSkyBox = new RE_SkyBox();
	newSkyBox->SetName(name.c_str());
	newSkyBox->SetAssetPath(assetPath);
	newSkyBox->SetType(ResourceContainer::Type::SKYBOX);
	newSkyBox->Import(false);
	newSkyBox->SaveMeta();

	return Reference(newSkyBox);
}

const char* RE_ResourceManager::ImportPrefab(const char* assetPath)
{
	RE_INPUT->PauseEvents();
	eastl::string path(assetPath);
	eastl::string filename = path.substr(path.find_last_of("/") + 1);
	eastl::string name = filename.substr(0, filename.find_last_of("."));
	eastl::string extension = filename.substr(filename.find_last_of(".") + 1);

	RE_Prefab* newPrefab = new RE_Prefab();
	newPrefab->SetName(name.c_str());
	newPrefab->SetAssetPath(assetPath);
	newPrefab->SetType(ResourceContainer::Type::PREFAB);
	newPrefab->Import(false);
	newPrefab->SaveMeta();
	RE_INPUT->ResumeEvents();

	return Reference(newPrefab);
}

const char* RE_ResourceManager::ImportScene(const char* assetPath)
{
	RE_INPUT->PauseEvents();
	eastl::string path(assetPath);
	eastl::string filename = path.substr(path.find_last_of("/") + 1);
	eastl::string name = filename.substr(0, filename.find_last_of("."));
	eastl::string extension = filename.substr(filename.find_last_of(".") + 1);

	RE_Scene* newScene = new RE_Scene();
	newScene->SetName(name.c_str());
	newScene->SetAssetPath(assetPath);
	newScene->SetType(ResourceContainer::Type::SCENE);
	newScene->Import(false);
	newScene->SaveMeta();
	RE_INPUT->ResumeEvents();

	return Reference(newScene);
}

const char* RE_ResourceManager::ImportParticleEmissor(const char* assetPath)
{
	eastl::string path(assetPath);
	eastl::string filename = path.substr(path.find_last_of("/") + 1);
	eastl::string name = filename.substr(0, filename.find_last_of("."));
	eastl::string extension = filename.substr(filename.find_last_of(".") + 1);

	RE_ParticleEmission* new_emission = new RE_ParticleEmission();
	new_emission->SetName(name.c_str());
	new_emission->SetAssetPath(assetPath);
	new_emission->SetType(ResourceContainer::Type::PARTICLE_EMISSION);
	new_emission->Import(false);
	new_emission->SaveMeta();

	return Reference(new_emission);
}

const char* RE_ResourceManager::ImportParticleRender(const char* assetPath)
{
	eastl::string path(assetPath);
	eastl::string filename = path.substr(path.find_last_of("/") + 1);
	eastl::string name = filename.substr(0, filename.find_last_of("."));
	eastl::string extension = filename.substr(filename.find_last_of(".") + 1);

	RE_ParticleRender* new_emission = new RE_ParticleRender();
	new_emission->SetName(name.c_str());
	new_emission->SetAssetPath(assetPath);
	new_emission->SetType(ResourceContainer::Type::PARTICLE_RENDER);
	new_emission->Import(false);
	new_emission->SaveMeta();

	return Reference(new_emission);
}

unsigned int RE_ResourceManager::TotalReferenceCount(const char* resMD5) const
{
	return resourcesCounter.at(resMD5);
}