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

RE_ResourceManager::RE_ResourceManager()
{
	model_importer = new RE_ModelImporter();
	shader_importer = new RE_ShaderImporter();
	internalResources = new RE_InternalResources();
}

RE_ResourceManager::~RE_ResourceManager()
{
	DEL(internalResources);
	DEL(shader_importer);
	DEL(model_importer);
}

void RE_ResourceManager::Init()
{
	RE_PROFILE(RE_ProfiledFunc::Init, RE_ProfiledClass::ResourcesManager);
	RE_LOG_SEPARATOR("Initializing Resources");

	RE_TextureImporter::Init();
	shader_importer->Init();
	internalResources->Init();

	// Fetch Assets
	RE_FS->ReadAssetChanges(0, true);
	RE_AUDIO->ReadBanksChanges();
}

void RE_ResourceManager::Clear()
{
	RE_PROFILE(RE_ProfiledFunc::Clear, RE_ProfiledClass::ResourcesManager);
	internalResources->Clear();
	shader_importer->Clear();

	while (!resources.empty())
	{
		DEL(resources.begin()->second);
		resources.erase(resources.begin());
	}
}

void RE_ResourceManager::RecieveEvent(const Event& e)
{
	if (e.type == RE_EventType::RESOURCE_CHANGED)
	{
		ResourceContainer* res = resources.at(e.data1.AsCharP());
		if (res->GetType() == ResourceType::SHADER)
		{
			eastl::vector<ResourceContainer*> materials = GetResourcesByType(ResourceType::MATERIAL);
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

	do {
		resourcesSelected.pop();
	} while (all && !resourcesSelected.empty());
}

ResourceContainer* RE_ResourceManager::At(const char* md5) const { return resources.at(md5); }

const char* RE_ResourceManager::ReferenceByMeta(const char* metaPath, ResourceType type)
{
	ResourceContainer* newContainer = nullptr;
	switch (type) {
	case ResourceType::SHADER:				newContainer = static_cast<ResourceContainer*>(new RE_Shader(metaPath));				break;
	case ResourceType::TEXTURE:				newContainer = static_cast<ResourceContainer*>(new RE_Texture(metaPath));				break;
	case ResourceType::PREFAB:				newContainer = static_cast<ResourceContainer*>(new RE_Prefab(metaPath));				break;
	case ResourceType::SKYBOX:				newContainer = static_cast<ResourceContainer*>(new RE_SkyBox(metaPath));				break;
	case ResourceType::MATERIAL:			newContainer = static_cast<ResourceContainer*>(new RE_Material(metaPath));				break;
	case ResourceType::MODEL:				newContainer = static_cast<ResourceContainer*>(new RE_Model(metaPath));					break;
	case ResourceType::SCENE:				newContainer = static_cast<ResourceContainer*>(new RE_Scene(metaPath));					break; 
	case ResourceType::PARTICLE_EMITTER:	newContainer = static_cast<ResourceContainer*>(new RE_ParticleEmitterBase(metaPath));	break; 
	case ResourceType::PARTICLE_EMISSION:	newContainer = static_cast<ResourceContainer*>(new RE_ParticleEmission(metaPath));		break; 
	case ResourceType::PARTICLE_RENDER:		newContainer = static_cast<ResourceContainer*>(new RE_ParticleRender(metaPath));		break;  }

	const char* retMD5 = nullptr;
	if (newContainer)
	{
		newContainer->LoadMeta();
		retMD5 = Reference(newContainer);
	}

	return retMD5;
}

unsigned int RE_ResourceManager::TotalReferences() const { return resources.size(); }

eastl::vector<const char*> RE_ResourceManager::GetAllResourcesActiveByType(ResourceType resT)
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
	eastl::vector<ResourceContainer*> temp_resources = GetResourcesByType(ResourceType::SHADER);

	RE_Shader* shader = nullptr;
	for (auto res : temp_resources)
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
	ResourceType rType = resource->GetType();

	switch (rType)
	{
	case ResourceType::SHADER: //search on materials
	case ResourceType::TEXTURE: //search on materials. no scenes will be afected
	{
		temp_resources = GetResourcesByType(ResourceType::MATERIAL);
		RE_Material* mat = nullptr;
		for (auto resource : temp_resources)
		{
			mat = dynamic_cast<RE_Material*>(resource);
			if (rType == ResourceType::SHADER)
			{
				if (mat->ExitsOnShader(res)) ret.push_back(resource->GetMD5());
			}
			else if (mat->ExistsOnTexture(res)) ret.push_back(resource->GetMD5());
		}

		break;
	}
	case ResourceType::PARTICLE_EMISSION:
	case ResourceType::PARTICLE_RENDER:
	{
		temp_resources = GetResourcesByType(ResourceType::PARTICLE_EMITTER);
		RE_ParticleEmitterBase* emitter = nullptr;
		for (auto resource : temp_resources)
		{
			emitter = dynamic_cast<RE_ParticleEmitterBase*>(resource);

			if (emitter->Contains(res)) ret.push_back(resource->GetMD5());

		}
		break;
	}
	case ResourceType::SKYBOX: //search on cameras from scenes or prefabs
	case ResourceType::MATERIAL: //search on scenes, prefabs and models(models advise you need to reimport)
	case ResourceType::PARTICLE_EMITTER: //search on scenes and prefabs
	{
		temp_resources = GetResourcesByType(ResourceType::SCENE);
		temp = GetResourcesByType(ResourceType::PREFAB);
		temp_resources.insert(temp_resources.end(), temp.begin(), temp.end());
		if (rType == ResourceType::MATERIAL)
		{
			temp = GetResourcesByType(ResourceType::MODEL);
			temp_resources.insert(temp_resources.end(), temp.begin(), temp.end());
		}

		RE_INPUT->PauseEvents();
		for (auto resource : temp_resources)
		{
			RE_ECS_Pool* poolGORes = nullptr;
			Use(resource->GetMD5());
			switch (resource->GetType()) {
			case ResourceType::SCENE: poolGORes = dynamic_cast<RE_Scene*>(resource)->GetPool(); break;
			case ResourceType::PREFAB: poolGORes = dynamic_cast<RE_Prefab*>(resource)->GetPool(); break;
			case ResourceType::MODEL: poolGORes = dynamic_cast<RE_Model*>(resource)->GetPool(); break;
			}

			eastl::stack<RE_Component*> comps;
			if (poolGORes != nullptr)
			{
				switch (rType)
				{
				case ResourceType::SKYBOX:			 comps = RE_SCENE->GetRootPtr()->GetAllChildsComponents(C_CAMERA); break;
				case ResourceType::MATERIAL:		 comps = RE_SCENE->GetRootPtr()->GetAllChildsComponents(C_MESH); break;
				case ResourceType::PARTICLE_EMITTER: comps = RE_SCENE->GetRootPtr()->GetAllChildsComponents(C_PARTICLEEMITER); break;
				default: break;
				}

				bool skip = false;
				while (!comps.empty() && !skip)
				{
					switch (rType)
					{
					case ResourceType::SKYBOX: break;
					case ResourceType::MATERIAL:
					{
						RE_CompMesh* mesh = dynamic_cast<RE_CompMesh*>(comps.top());
						if (mesh && mesh->GetMaterial() == res)
						{
							ret.push_back(resource->GetMD5());
							skip = true;
						}
						break;
					}
					case ResourceType::PARTICLE_EMITTER:
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
			RE_INPUT->ResumeEvents();
			break;
		}
	}
	default: break;
	}
	
	return ret;
}

ResourceContainer* RE_ResourceManager::DeleteResource(const char* res, eastl::vector<const char*> resourcesWillChange, bool resourceOnScene)
{
	ResourceContainer* resource = resources.at(res);
	ResourceType rType = resource->GetType();

	if (TotalReferenceCount(res) > 0) resource->UnloadMemory();

	switch (rType)
	{
	case ResourceType::SHADER: //search on materials.
	case ResourceType::TEXTURE: //search on materials. No will afect on scene because the textures are saved on meta
	{
		RE_Material* resChange = nullptr;
		for (auto resToChange : resourcesWillChange)
		{
			(resChange = dynamic_cast<RE_Material*>(RE_RES->At(resToChange)))->LoadInMemory();
			if(rType == ResourceType::SHADER) resChange->DeleteShader();
			else resChange->DeleteTexture(res);
			resChange->UnloadMemory();
			RE_RENDER->PushThumnailRend(resToChange, true);
		}
		break;
	}
	case ResourceType::SKYBOX: //search on cameras from scenes or prefabs
	case ResourceType::MATERIAL: //search on scenes, prefabs and models(models advise you need to reimport)
	case ResourceType::PARTICLE_EMITTER://search on particle emitters from scenes or prefabs
	{
		for (auto resToChange : resourcesWillChange)
		{
			ResourceContainer* resChange = RE_RES->At(resToChange);
			ResourceType goType = resChange->GetType();

			if (resourceOnScene && resToChange == RE_SCENE->GetCurrentScene())
				continue;

			if (goType != ResourceType::MODEL)
			{
				RE_INPUT->PauseEvents();

				RE_ECS_Pool* poolGORes = nullptr;
				Use(resToChange);

				switch (goType)
				{
				case ResourceType::SCENE: poolGORes = dynamic_cast<RE_Scene*>(resChange)->GetPool(); break;
				case ResourceType::PREFAB: poolGORes = dynamic_cast<RE_Prefab*>(resChange)->GetPool(); break;
				default: break;
				}

				if (poolGORes != nullptr)
				{
					eastl::stack<RE_Component*> comps = poolGORes->GetRootPtr()->GetAllChildsComponents(
						(rType == ResourceType::MATERIAL) ? C_MESH : C_CAMERA);

					while (!comps.empty())
					{
						RE_Component* go = comps.top();
						comps.pop();

						switch (rType)
						{
						case ResourceType::SKYBOX:
						{
							RE_CompCamera* cam = dynamic_cast<RE_CompCamera*>(go);
							if (cam && cam->isUsingSkybox() && cam->GetSkybox() == res) cam->SetSkyBox(nullptr);
							break;
						}
						case ResourceType::MATERIAL:
						{
							RE_CompMesh* mesh = dynamic_cast<RE_CompMesh*>(go);
							if (mesh && mesh->GetMaterial() == res) mesh->SetMaterial(nullptr);
							break;
						}
						case ResourceType::PARTICLE_EMITTER:
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
					case ResourceType::SCENE:
					{
						RE_Scene* s = dynamic_cast<RE_Scene*>(resChange);
						s->Save(poolGORes);
						s->SaveMeta();
						break;
					}
					case ResourceType::PREFAB:
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
					DEL(poolGORes);
					RE_INPUT->ResumeEvents();
				}
			}
		}

		break;
	}
	//Search on particle emitters and then apply changes on scene if exists
	case ResourceType::PARTICLE_EMISSION:
	case ResourceType::PARTICLE_RENDER:
	{
		RE_ParticleEmitterBase* resChange = nullptr;
		for (auto resToChange : resourcesWillChange)
		{
			bool need_update_scene = false;

			resChange = dynamic_cast<RE_ParticleEmitterBase*>(RE_RES->At(resToChange));

			if (TotalReferenceCount(resToChange) > 0) need_update_scene = true;

			if (rType == ResourceType::PARTICLE_EMISSION) resChange->ChangeEmissor(nullptr, nullptr);
			else resChange->ChangeRenderer(nullptr, nullptr);
			
			resChange->SaveMeta();

			if (need_update_scene) {
				eastl::stack<RE_Component*> comps = RE_SCENE->GetRootPtr()->GetAllChildsComponents(C_PARTICLEEMITER);

				while (!comps.empty())
				{
					RE_CompParticleEmitter* emitter = dynamic_cast<RE_CompParticleEmitter*>(comps.top());
					comps.pop();

					if (emitter)
					{
						if (emitter->GetEmitterResource() == resToChange)
						{
							emitter->UnUseResources();
							emitter->UseResources();
						}
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
		case ResourceType::SKYBOX: comps = RE_SCENE->GetRootPtr()->GetAllChildsComponents(C_CAMERA); break;
		case ResourceType::MATERIAL: comps = RE_SCENE->GetRootPtr()->GetAllChildsComponents(C_MESH); break;
		case ResourceType::PARTICLE_EMITTER: comps = RE_SCENE->GetRootPtr()->GetAllChildsComponents(C_PARTICLEEMITER); break;
		default: break;
		}
		
		while (!comps.empty())
		{
			RE_Component* go = comps.top();
			comps.pop();

			switch (rType)
			{
			case ResourceType::SKYBOX:
			{
				RE_CompCamera* cam = dynamic_cast<RE_CompCamera*>(go);
				if (cam && cam->GetSkybox() == res) cam->SetSkyBox(nullptr);
				break;
			}
			case ResourceType::MATERIAL:
			{
				RE_CompMesh* mesh = dynamic_cast<RE_CompMesh*>(go);
				if (mesh && mesh->GetMaterial() == res) mesh->SetMaterial(nullptr);
				break;
			}
			case ResourceType::PARTICLE_EMITTER:
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

	if (rType != ResourceType::SHADER &&
		rType != ResourceType::PARTICLE_EMITTER &&
		rType != ResourceType::PARTICLE_EMISSION &&
		rType != ResourceType::PARTICLE_RENDER)
		RE_EDITOR->thumbnails->Delete(res);

	return resource;
}

eastl::vector<ResourceContainer*> RE_ResourceManager::GetResourcesByType(ResourceType type)
{
	eastl::vector<ResourceContainer*> ret;

	for (auto resource : resources)
		if (resource.second->GetType() == type)
			ret.push_back(resource.second);

	return ret;
}

const char* RE_ResourceManager::IsReference(const char* md5, ResourceType type)
{
	const char* ret = nullptr;
	if (type == ResourceType::UNDEFINED)
	{
		for (auto resource : resources)
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

const char * RE_ResourceManager::FindMD5ByMETAPath(const char * metaPath, ResourceType type)
{
	const char* ret = nullptr;
	size_t sizemeta = 0;
	if (type == ResourceType::UNDEFINED)
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

const char* RE_ResourceManager::FindMD5ByLibraryPath(const char* libraryPath, ResourceType type)
{
	const char* ret = nullptr;
	size_t sizelibrary = 0;
	if (type == ResourceType::UNDEFINED)
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

const char * RE_ResourceManager::FindMD5ByAssetsPath(const char * assetsPath, ResourceType type)
{
	const char* ret = nullptr;
	size_t sizeassets = 0;
	if (type == ResourceType::UNDEFINED)
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

	meshMD5 = FindMD5ByLibraryPath(librariPath, ResourceType::MESH);

	if (!meshMD5)
	{
		if (RE_FS->Exists(librariPath))
		{
			RE_Mesh* newMesh = new RE_Mesh();
			newMesh->SetLibraryPath(librariPath);
			newMesh->SetType(ResourceType::MESH);
			meshMD5 = Reference(newMesh);
		}
	}

	if (!meshMD5) RE_LOG_ERROR("Error finding library mesh:\n%s", librariPath);

	return meshMD5;
}

bool RE_ResourceManager::isNeededResoursesLoaded(const char* metaPath, ResourceType type) const
{
	bool ret = false;

	ResourceContainer* newContainer = nullptr;
	switch (type) {
	case ResourceType::SHADER:				newContainer = static_cast<ResourceContainer*>(new RE_Shader(metaPath));				break;
	case ResourceType::TEXTURE:				newContainer = static_cast<ResourceContainer*>(new RE_Texture(metaPath));				break;
	case ResourceType::PREFAB:				newContainer = static_cast<ResourceContainer*>(new RE_Prefab(metaPath));				break;
	case ResourceType::SKYBOX:				newContainer = static_cast<ResourceContainer*>(new RE_SkyBox(metaPath));				break;
	case ResourceType::MATERIAL:			newContainer = static_cast<ResourceContainer*>(new RE_Material(metaPath));				break;
	case ResourceType::MODEL:				newContainer = static_cast<ResourceContainer*>(new RE_Model(metaPath));					break;
	case ResourceType::SCENE:				newContainer = static_cast<ResourceContainer*>(new RE_Scene(metaPath));					break;
	case ResourceType::PARTICLE_EMITTER:	newContainer = static_cast<ResourceContainer*>(new RE_ParticleEmitterBase(metaPath));	break;
	case ResourceType::PARTICLE_EMISSION:	newContainer = static_cast<ResourceContainer*>(new RE_ParticleEmission(metaPath));		break;
	case ResourceType::PARTICLE_RENDER:		newContainer = static_cast<ResourceContainer*>(new RE_ParticleRender(metaPath));		break;
	}

	if(newContainer)
	{
		ret = newContainer->isNeededResourcesReferenced();
		DEL(newContainer);
	}

	return ret;
}

void RE_ResourceManager::ThumbnailResources()
{
	RE_PROFILE(RE_ProfiledFunc::ThumbnailResources, RE_ProfiledClass::ResourcesManager);
	for (auto res : resources)
	{
		ResourceType rT = res.second->GetType();
		if(
			rT == ResourceType::SCENE ||
			rT == ResourceType::PREFAB ||
			rT == ResourceType::MODEL ||
			rT == ResourceType::SKYBOX ||
			rT == ResourceType::MATERIAL ||
			rT == ResourceType::TEXTURE) RE_RENDER->PushThumnailRend(res.first);
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
		eastl::vector<ResourceContainer*> emitters = GetResourcesByType(ResourceType::PARTICLE_EMITTER);

		while (!resources_particles_reimport.empty())
		{
			const char* res = resources_particles_reimport.top();
			resources_particles_reimport.pop();

			for (auto res_container : emitters)
				dynamic_cast<RE_ParticleEmitterBase*>(res_container)->SomeResourceChanged(res);
		}
	}
}

const char* RE_ResourceManager::GetNameFromType(const ResourceType type)
{
	switch (type) {
	case ResourceType::TEXTURE: return "Texture";
	case ResourceType::SCENE: return "Scene";
	case ResourceType::MATERIAL: return "Material";
	case ResourceType::MESH: return "Mesh";
	case ResourceType::PREFAB: return "Prefab";
	case ResourceType::SHADER: return "Shader";
	case ResourceType::MODEL: return "Model";
	case ResourceType::SKYBOX: return "Skybox";
	case ResourceType::PARTICLE_EMITTER: return "Particle emitter";
	case ResourceType::PARTICLE_EMISSION: return "Particle emission";
	case ResourceType::PARTICLE_RENDER: return "Particle render";
	case ResourceType::UNDEFINED: return "Undefined";
	default: return "Invalid"; }
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
	newModel->SetType(ResourceType::MODEL);
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
	newTexture->SetType(ResourceType::TEXTURE);
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
	newMaterial->SetType(ResourceType::MATERIAL);
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
	newSkyBox->SetType(ResourceType::SKYBOX);
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
	newPrefab->SetType(ResourceType::PREFAB);
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
	newScene->SetType(ResourceType::SCENE);
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
	new_emission->SetType(ResourceType::PARTICLE_EMISSION);
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
	new_emission->SetType(ResourceType::PARTICLE_RENDER);
	new_emission->Import(false);
	new_emission->SaveMeta();

	return Reference(new_emission);
}

unsigned int RE_ResourceManager::TotalReferenceCount(const char* resMD5) const
{
	return resourcesCounter.at(resMD5);
}