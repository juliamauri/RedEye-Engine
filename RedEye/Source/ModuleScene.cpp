#include "ModuleScene.h"

#include "Application.h"
#include "ModuleRenderer3D.h"
#include "ModuleEditor.h"
#include "ModuleInput.h"
#include "FileSystem.h"
#include "OutputLog.h"
#include "ResourceManager.h"
#include "RE_Material.h"
#include "RE_TextureImporter.h"
#include "ShaderManager.h"
#include "RE_GameObject.h"
#include "RE_Prefab.h"
#include "RE_CompTransform.h"
#include "RE_CompMesh.h"
#include "RE_CompCamera.h"
#include "RE_CompParticleEmiter.h"
#include "RE_ModelImporter.h"
#include "TimeManager.h"
#include "RE_PrimitiveManager.h"
#include "md5.h"
#include <gl/GL.h>
#include <string>

ModuleScene::ModuleScene(const char* name, bool start_enabled) : Module(name, start_enabled)
{
	all_aabb_color = math::vec(0.f, 225.f, 0.f);
	sel_aabb_color = math::vec(255.f, 255.f, 0.f);
}

ModuleScene::~ModuleScene()
{}

bool ModuleScene::Start()
{
	bool ret = true;

	//Loading Shaders
	if (App->shaders)
	{
		ret = App->shaders->Load("texture", &modelloading);
		if (!ret)
			LOG("%s\n", App->shaders->GetShaderError());
	}

	// Load scene
	std::string path_scene("Assets/Scenes/");
	path_scene += GetName();
	path_scene += ".re";
	Config scene_file(path_scene.c_str(), App->fs->GetZipPath());
	if (scene_file.Load())
	{
		// Load saved scene
		JSONNode* node = scene_file.GetRootNode("Game Objects");
		root = node->FillGO();
		DEL(node);
	}
	else
	{
		// Load default FBX
		RE_Prefab* newModel = App->modelImporter->LoadModelFromAssets("Assets/Meshes/BakerHouse/BakerHouse.fbx");
		//App->meshes->LoadMeshOnGameObject(root, "BakerHouse/BakerHouse.fbx");

		root = new RE_GameObject("root");
		root->AddChild(newModel->GetRoot());
		DEL(newModel);

		// Add camera
		(new RE_GameObject("Main Camera", GUID_NULL, root))->AddComponent(C_CAMERA);
	}

	root->AddComponent(C_PLANE);

	// Setup AABB
	root->ResetBoundingBoxFromChilds();
	aabb_need_reset = false;

	// FOCUS CAMERA
	if (!root->GetChilds().empty()) {
		App->scene->SetSelected(root->GetChilds().begin()._Ptr->_Myval);
		App->editor->FocusSelected();
	}

	// Quadtree
	//quad_tree.Build(root);

	// Checkers
	int value;
	int IMAGE_ROWS = 264;
	int IMAGE_COLS = 264;
	GLubyte imageData[264][264][3];
	for (int row = 0; row < IMAGE_ROWS; row++) {
		for (int col = 0; col < IMAGE_COLS; col++) {
			// Each cell is 8x8, value is 0 or 255 (black or white)
			value = (((row & 0x8) == 0) ^ ((col & 0x8) == 0)) * 255;
			imageData[row][col][0] = (GLubyte)value;
			imageData[row][col][1] = (GLubyte)value;
			imageData[row][col][2] = (GLubyte)value;
		}
	}

	glGenTextures(1, &checkers_texture);
	glBindTexture(GL_TEXTURE_2D, checkers_texture);
	glTexImage2D(GL_TEXTURE_2D, 0, 3, IMAGE_COLS, IMAGE_ROWS, 0, GL_RGB, GL_UNSIGNED_BYTE, imageData);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glBindTexture(GL_TEXTURE_2D, 0);

	return ret;
}

update_status ModuleScene::Update()
{
	root->Update();

	return UPDATE_CONTINUE;
}

bool ModuleScene::CleanUp()
{
	Serialize();

	DEL(root);
	glDeleteTextures(1, &checkers_texture);

	if (smoke_particle)
		DEL(smoke_particle);

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
	if(selected != nullptr) selected->GetParent()->AddChild(new RE_GameObject(*selected));
}

void ModuleScene::CreateCube()
{
	RE_GameObject* cube_go = AddGO("Sphere", root);
	cube_go->AddComponent(App->primitives->CreateCube(cube_go));
}

void ModuleScene::CreateSphere()
{
	RE_GameObject* sphere_go = AddGO("Sphere", root);
	sphere_go->AddComponent(App->primitives->CreateSphere(sphere_go));
}

void ModuleScene::DrawEditor()
{
	if (ImGui::CollapsingHeader(GetName()))
	{
		// AABB Controls
		ImGui::Text("Last AABB Reset took: %f", aabb_reset_time);
		if (aabb_need_reset && ImGui::Button("Reset All AABB"))
		{
			root->ResetBoundingBoxFromChilds();
			aabb_need_reset = false;
		}

		// AABB All
		ImGui::Checkbox("Draw All AABB", &draw_all_aabb);
		if (draw_all_aabb)
		{
			// Color 
			float p[3] = { all_aabb_color.x, all_aabb_color.y, all_aabb_color.z };
			if (ImGui::DragFloat3("Color All", p, 0.1f, 0.f, 255.f, "%.2f"))
				all_aabb_color = math::vec(p[0], p[1], p[2]);

			// Width
			ImGui::DragFloat("Width All", &all_aabb_width, 01.f, 0.1f, 100.f, "%.1f");
		}

		// AABB Selected
		ImGui::Checkbox("Draw Selected AABB", &draw_selected_aabb);
		if (draw_selected_aabb)
		{
			// Color 
			float p[3] = { sel_aabb_color.x, sel_aabb_color.y, sel_aabb_color.z };
			if (ImGui::DragFloat3("Color Selected", p, 0.1f, 0.f, 255.f, "%.2f"))
				sel_aabb_color = math::vec(p[0], p[1], p[2]);

			// Width
			ImGui::DragFloat("Width Selected", &sel_aabb_width, 01.f, 0.1f, 100.f, "%.1f");
		}

		ImGui::Checkbox("Focus on Select", &focus_on_select);

		//ImGui::Checkbox("Draw QuadTree", &draw_quad_tree);
	}
}

void ModuleScene::DrawScene()
{
	// Draw Bounding Boxes
	if (draw_all_aabb)
		root->DrawAllAABB(all_aabb_color, all_aabb_width);

	if (draw_selected_aabb && selected != nullptr && selected != root)
		selected->DrawGlobalAABB(sel_aabb_color, sel_aabb_width);

	/*/ Draw Quadtree
	if (draw_quad_tree)
		quad_tree.Draw();*/

	// Load Shader Uniforms
	ShaderManager::use(modelloading);
	ShaderManager::setFloat4x4(modelloading, "view", App->editor->GetCamera()->GetViewPtr());
	ShaderManager::setFloat4x4(modelloading, "projection", App->editor->GetCamera()->GetProjectionPtr());

	ShaderManager::use(App->primitives->shaderPrimitive);
	ShaderManager::setFloat4x4(App->primitives->shaderPrimitive, "view", App->editor->GetCamera()->GetViewPtr());
	ShaderManager::setFloat4x4(App->primitives->shaderPrimitive, "projection", App->editor->GetCamera()->GetProjectionPtr());

	/*/ Frustum Culling
	std::vector<RE_GameObject*> objects;
	quad_tree.CollectIntersections(objects, App->editor->GetCamera()->GetFrustum());
	drawn_go = objects.size();
	for (auto object : objects) object->Draw(false);*/

	root->Draw();
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
		App->editor->FocusSelected();
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
		math::float4 camera_pos = math::float4(App->editor->GetCamera()->GetTransform()->GetGlobalPosition(), 0.f);

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
	root->Serialize(node);

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

	RE_GameObject* toAdd = nullptr;
	if (App->fs->Exists(fbxOnLibrary.c_str())) {
		
		Config fbxPrefab(fbxOnLibrary.c_str(), App->fs->GetZipPath());
		if (fbxPrefab.Load()) {
			JSONNode* node = fbxPrefab.GetRootNode("Game Objects");
			toAdd = node->FillGO();
			DEL(node);
		}
	}
	else
	{
		RE_Prefab* toLoad = App->modelImporter->LoadModelFromAssets(fbxPath);
		toAdd = toLoad->GetRoot();
		DEL(toLoad);
	}

	if (toAdd) {
		if (root) DEL(root);
		selected = nullptr;
		root = new RE_GameObject("root");
		root->AddChild(toAdd);

		root->ResetBoundingBoxFromChilds();
		aabb_need_reset = false;
		// FOCUS CAMERA ON DROPPED GEOMETRY
		App->scene->SetSelected(toAdd);
		App->editor->FocusSelected();
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

		const char* materialMD5 = selectedMesh->GetMaterial();
		if (materialMD5) {
			RE_Material* selectedMaterial = (RE_Material*)App->resources->At(materialMD5);
			if (selectedMaterial) {
				if (selectedMaterial->tDiffuse.empty())
					selectedMaterial->tDiffuse.push_back(textureResource);
				else
					selectedMaterial->tDiffuse[0] = textureResource;
			}
		}
	}
}

void ModuleScene::SceneModified()
{
	aabb_need_reset = true;
}
