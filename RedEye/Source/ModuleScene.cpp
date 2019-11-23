#include "ModuleScene.h"

#include "Application.h"
#include "ModuleRenderer3D.h"
#include "ModuleInput.h"

#include "EditorWindows.h"
#include "FileSystem.h"
#include "RE_PrimitiveManager.h"
#include "RE_ResourceManager.h"
#include "RE_CameraManager.h"
#include "RE_ShaderImporter.h"
#include "RE_ModelImporter.h"
#include "RE_TextureImporter.h"

#include "RE_Material.h"
#include "RE_Prefab.h"

#include "RE_GameObject.h"
#include "RE_CompMesh.h"
#include "RE_CompTransform.h"
#include "RE_CompCamera.h"
#include "RE_CompPrimitive.h" 

#include "OutputLog.h"
#include "RE_HandleErrors.h"
#include "TimeManager.h"

#include "md5.h"
#include "Glew/include/glew.h"
#include <gl/GL.h>
#include <string>

#include "SDL2\include\SDL.h"


#define DEFAULTMODEL "Assets/Meshes/BakerHouse/BakerHouse.fbx"

ModuleScene::ModuleScene(const char* name, bool start_enabled) : Module(name, start_enabled)
{
	all_aabb_color = math::vec(0.f, 1.f, 0.f);
	sel_aabb_color = math::vec(1.f, 1.f, 1.f);
	quad_tree_color = math::vec(1.f, 1.f, 0.f);
	frustum_color = math::vec(0.f, 1.f, 1.f);
}

ModuleScene::~ModuleScene()
{}

bool ModuleScene::Init(JSONNode * node)
{
	defaultModel = node->PullString("defaultModel", DEFAULTMODEL);
	return true;
}

bool ModuleScene::Start()
{
	bool ret = true;

	// Load scene
	Timer timer;
	std::string path_scene("Assets/Scenes/");
	path_scene += GetName();
	path_scene += ".re";
	bool loadDefaultFBX = false;
	Config scene_file(path_scene.c_str(), App->fs->GetZipPath());
	if (scene_file.Load())
	{
		App->handlerrors->StartHandling();

		LOG("Importing scene from own format:");
		// Load saved scene
		JSONNode* node = scene_file.GetRootNode("Game Objects");
		RE_GameObject* loadedDO = node->FillGO();

		if (loadedDO) {
			root = loadedDO;
			DEL(node);
		}
		else {
			LOG_ERROR("Own serialized scene can't be loaded");
			loadDefaultFBX = true;
		}

		LOG("Time imported: %u ms", timer.Read());
	}
	else
		loadDefaultFBX = true;
	
	if(loadDefaultFBX)
	{
		App->handlerrors->StartHandling();

		// Load default FBX
		LOG("Importing scene from default asset model:");
		RE_Prefab* newModel = App->modelImporter->LoadModelFromAssets(defaultModel.c_str());
		if (newModel) {
			root = new RE_GameObject("root");
			root->AddChild(newModel->GetRoot());
			DEL(newModel);
		}
		else {
			LOG_ERROR("Cannot be lodadded the default model.\nAssetsPath: %s", defaultModel.c_str());
		}
	}

	// Error Handling
	App->handlerrors->StopHandling();
	if (App->handlerrors->AnyErrorHandled()) {
		App->handlerrors->ActivatePopUp();
	}

	// Grid Plane
	root->AddComponent(C_PLANE);

	// Render Camera Management
	App->cams->RecallCameras(root);
	if (!RE_CameraManager::HasMainCamera())
		CreateCamera();

	// Setup AABB
	root->TransformModified();
	root->ResetBoundingBoxFromChilds();
	static_gos_modified = false;

	// FOCUS CAMERA
	if (!root->GetChilds().empty()) {
		App->scene->SetSelected(root->GetChilds().begin()._Ptr->_Myval);
		RE_CameraManager::CurrentCamera()->Focus(selected);
	}

	// Quadtree
	quad_tree.Build(root);

	return ret;
}

update_status ModuleScene::Update()
{
	OPTICK_CATEGORY("Update Scene", Optick::Category::GameLogic);

	root->Update();

	return UPDATE_CONTINUE;
}

bool ModuleScene::CleanUp()
{
	Serialize();

	DEL(root);

	return true;
}

void ModuleScene::OnPlay()
{
	root->OnPlay();
}

void ModuleScene::OnPause()
{
	root->OnPause();
}

void ModuleScene::OnStop()
{
	root->OnStop();
}

void ModuleScene::RecieveEvent(const Event& e)
{
}

RE_GameObject * ModuleScene::GetRoot() const
{
	return root;
}

RE_GameObject * ModuleScene::AddGO(const char * name, RE_GameObject * parent)
{
	return new RE_GameObject(name, GUID_NULL, parent ? parent : root);
}

void ModuleScene::AddGoToRoot(RE_GameObject * toAdd)
{
	root->AddChild(toAdd);
}

void ModuleScene::DuplicateSelectedObject()
{
	if(selected != nullptr)
		selected->GetParent()->AddChild(new RE_GameObject(*selected));
}

void ModuleScene::CreateCube()
{
	RE_GameObject* cube_go = AddGO("Cube", root);
	cube_go->AddComponent(App->primitives->CreateCube(cube_go));
}

void ModuleScene::CreateSphere()
{
	RE_GameObject* sphere_go = AddGO("Sphere", root);
	sphere_go->AddComponent(App->primitives->CreateSphere(sphere_go));
}

void ModuleScene::CreateCamera()
{
	RE_GameObject* cam_go = AddGO("Camera", root);
	cam_go->AddCompCamera();
}

void ModuleScene::DrawEditor()
{
	if (ImGui::CollapsingHeader(GetName()))
	{
		// AABB Controls
		ImGui::Text("Last AABB Reset took: %f", aabb_reset_time);
		if (static_gos_modified && ImGui::Button("Reset All AABB"))
		{
			root->ResetBoundingBoxFromChilds();
			static_gos_modified = false;
		}

		// AABB All
		ImGui::Checkbox("Draw All AABB", &draw_all_aabb);
		if (draw_all_aabb)
		{
			float p[3] = { all_aabb_color.x, all_aabb_color.y, all_aabb_color.z };
			if (ImGui::ColorEdit3("Color All", p))
				all_aabb_color = math::vec(p[0], p[1], p[2]);
		}

		// AABB Selected
		ImGui::Checkbox("Draw Selected AABB", &draw_selected_aabb);
		if (draw_selected_aabb)
		{
			float p[3] = { sel_aabb_color.x, sel_aabb_color.y, sel_aabb_color.z };
			if (ImGui::ColorEdit3("Color Selected", p))
				sel_aabb_color = math::vec(p[0], p[1], p[2]);
		}

		// Camera Fustrums
		ImGui::Checkbox("Draw Camera Fustrums", &draw_cameras);
		if (draw_cameras)
		{
			float p[3] = { frustum_color.x, frustum_color.y, frustum_color.z };
			if (ImGui::ColorEdit3("Color Fustrum", p))
				frustum_color = math::vec(p[0], p[1], p[2]);
		}

		ImGui::Checkbox("Focus on Select", &focus_on_select);
		ImGui::Checkbox("Draw QuadTree", &draw_quad_tree);
	}
}

void ModuleScene::DrawDebug() const
{
	OPTICK_CATEGORY("Scene Debug Draw", Optick::Category::Debug);

	// Draw Bounding Boxes
	if (draw_all_aabb || draw_selected_aabb || draw_quad_tree)
	{
		RE_ShaderImporter::use(0);
		bool resetLight = App->renderer3d->GetLighting();

		if (resetLight)
			glDisable(GL_LIGHTING);

		glMatrixMode(GL_PROJECTION);
		glLoadMatrixf(RE_CameraManager::CurrentCamera()->GetProjectionPtr());
		glMatrixMode(GL_MODELVIEW);
		glLoadMatrixf((RE_CameraManager::CurrentCamera()->GetView()).ptr());
		glBegin(GL_LINES);

		if (draw_all_aabb)
		{
			glColor3f(all_aabb_color.x, all_aabb_color.y, all_aabb_color.z);
			root->DrawAllAABB();
		}

		if (draw_selected_aabb && selected != nullptr && selected != root)
		{
			glColor3f(sel_aabb_color.x, sel_aabb_color.y, sel_aabb_color.z);
			selected->DrawGlobalAABB();
		}

		if (draw_quad_tree)
		{
			glColor3f(quad_tree_color.x, quad_tree_color.y, quad_tree_color.z);
			quad_tree.Draw();
		}

		if (draw_cameras)
		{
			glColor3f(frustum_color.x, frustum_color.y, frustum_color.z);
			for(auto cam : App->cams->GetCameras())
				cam->DrawFrustum();
		}

		glEnd();

		if (resetLight)
			glEnable(GL_LIGHTING);
	}
}

void ModuleScene::DrawHeriarchy()
{
	if (root != nullptr)
	{
		for (auto child : root->GetChilds())
			child->DrawHeriarchy();
	}
}

// DRAW SELECTED GO
void ModuleScene::DrawFocusedProperties()
{
	if (selected != nullptr && selected != root)
		selected->DrawProperties();
}

void ModuleScene::SetSelected(RE_GameObject * select)
{
	selected = (select != nullptr) ? select : root;

	if (focus_on_select)
		RE_CameraManager::CurrentCamera()->Focus(selected);
}

RE_GameObject * ModuleScene::GetSelected() const
{
	return selected;
}

void ModuleScene::RayCastSelect(math::Ray & ray)
{
	std::vector<RE_GameObject*> objects;
	//quad_tree.CollectIntersections(objects, ray);
	if (!objects.empty())
	{
		float closest_distance = -1.f;
		RE_GameObject* new_selection = nullptr;
		math::float4 camera_pos = math::float4(RE_CameraManager::CurrentCamera()->GetTransform()->GetGlobalPosition(), 0.f);

		for (auto object : objects)
		{
			RE_CompMesh* comp_mesh = object->GetMesh();
			if (comp_mesh != nullptr)
			{
				/*math::vec pos =	comp_mesh->GetClosestTriangleIntersectPos(ray, App->editor->GetCamera());
				float res_distance = math::Distance3(math::float4(pos, 0.f), camera_pos);
				if (closest_distance < 0.f || closest_distance > res_distance)
				{
					new_selection = object;
					closest_distance = res_distance;
				}*/
			}
		}
	}
}

void ModuleScene::Serialize()
{
	char* buffer = nullptr;

	std::string path_scene("Assets/Scenes/");
	path_scene += GetName();
	path_scene += ".re";

	Config scene_file(path_scene.c_str(), App->fs->GetZipPath());

	JSONNode* node = scene_file.GetRootNode("Game Objects");

	node->SetArray();
	root->SerializeJson(node);

	DEL(node);

	scene_file.Save();
}

void ModuleScene::LoadFBXOnScene(const char * fbxPath)
{
	std::string path(fbxPath);
	std::string fileName = path.substr(path.find_last_of("/") + 1);
	fileName = fileName.substr(0, fileName.find_last_of("."));

	std::string fbxOnLibrary("Library/Scenes/");
	fbxOnLibrary += fileName + ".efab";
	bool reloadFBX = false;
	RE_GameObject* toAdd = nullptr;
	if (App->fs->Exists(fbxOnLibrary.c_str())) {
		LOG_SECONDARY("Internal prefab of fbx exits. Loading from it.\nPath: %s", fbxOnLibrary.c_str());
		Config fbxPrefab(fbxOnLibrary.c_str(), App->fs->GetZipPath());
		if (fbxPrefab.Load()) {
			JSONNode* node = fbxPrefab.GetRootNode("Game Objects");
			toAdd = node->FillGO();
			DEL(node);
		}
		else {
			LOG_WARNING("Can't open internal prefab, reload from .fbx");
			reloadFBX = true;
		}
	}
	else
		reloadFBX = true;
	
	if(reloadFBX)
	{
		LOG_SECONDARY("Loading fbx on scene: %s", fbxPath);
		RE_Prefab* toLoad = App->modelImporter->LoadModelFromAssets(fbxPath);
		if (toLoad) {
			toAdd = toLoad->GetRoot();
			DEL(toLoad);
		}
		else {
			LOG_ERROR("Can't load the .fbx.\nAssetsPath: %s", fbxPath);
		}

	}

	if (toAdd) {
		if (root) DEL(root);
		selected = nullptr;
		root = new RE_GameObject("root");
		root->AddChild(toAdd);

		root->TransformModified();
		root->ResetBoundingBoxFromChilds();
		static_gos_modified = false;
		// FOCUS CAMERA ON DROPPED GEOMETRY
		SetSelected(toAdd);
		RE_CameraManager::CurrentCamera()->Focus(selected);
	}
	else
		LOG_ERROR("Error to load dropped fbx");
}

void ModuleScene::LoadTextureOnSelectedGO(const char * texturePath)
{
	RE_CompMesh* selectedMesh = nullptr;
	if (selected && (selectedMesh = (RE_CompMesh*)selected->GetComponent(ComponentType::C_MESH)) != nullptr ) {
		std::string path(texturePath);
		std::string fileName = path.substr(path.find_last_of("/") + 1);
		fileName = fileName.substr(0, fileName.find_last_of("."));

		std::string md5Generated = md5(texturePath);
		const char* textureResource = App->resources->CheckFileLoaded(texturePath, md5Generated.c_str(), Resource_Type::R_TEXTURE);

		bool createMaterial = false;
		std::string filePath("Assets/Materials/");
		filePath += fileName;
		filePath += ".pupil";
		const char* materialMD5 = selectedMesh->GetMaterial();
		if (!materialMD5) {
			LOG_SECONDARY("Material don't exists on mesh creating a new one: %s", filePath.c_str());
			if (!App->fs->Exists(filePath.c_str())) {
				createMaterial = true;
			}
			else {
				Config materialToLoad(filePath.c_str(), App->fs->GetZipPath());
				if (materialToLoad.Load()) {
					materialMD5 = App->resources->CheckFileLoaded(filePath.c_str(), materialToLoad.GetMd5().c_str(), Resource_Type::R_MATERIAL);
					selectedMesh->SetMaterial(materialMD5);
				}
				else {
					std::string newFile("Assets/Materials/");
					newFile += fileName;
					uint count = 0;

					do {
						count++;
						filePath = newFile;
						filePath += " ";
						filePath += std::to_string(count);
						filePath += ".pupil";
					} while (!App->fs->Exists(filePath.c_str()));

					createMaterial = true;
				}
			}

			if (createMaterial) {
				LOG_SECONDARY("Creating new material: %s", filePath.c_str());
				RE_Material* newMaterial = new RE_Material();

				newMaterial->tDiffuse.push_back(textureResource);

				((ResourceContainer*)newMaterial)->SetName(fileName.c_str());
				((ResourceContainer*)newMaterial)->SetAssetPath(filePath.c_str());
				((ResourceContainer*)newMaterial)->SetType(Resource_Type::R_MATERIAL);
				newMaterial->Save();

				materialMD5 = App->resources->Reference((ResourceContainer*)newMaterial);
				selectedMesh->SetMaterial(materialMD5);
			}
		} 
		else {
			LOG_SECONDARY("Material on mesh found, changing texture and saving.");

			RE_Material* selectedMaterial = (RE_Material*)App->resources->At(materialMD5);
			if (selectedMaterial) {
				if (selectedMaterial->tDiffuse.empty())
					selectedMaterial->tDiffuse.push_back(textureResource);
				else
					selectedMaterial->tDiffuse[0] = textureResource;

				selectedMaterial->Save();
			}
		}
	}
	else
	{
		LOG_ERROR("Selected GameObject don't have mesh");
	}
}

void ModuleScene::StaticTransformed()
{
	static_gos_modified = true;
}

bool ModuleScene::DrawingSelAABB() const
{
	return draw_selected_aabb;
}

const QTree* ModuleScene::GetQuadTree() const
{
	return &quad_tree;
}
