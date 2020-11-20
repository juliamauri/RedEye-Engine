#include "ModuleRenderer3D.h"

#include "Application.h"
#include "RE_FileSystem.h"
#include "ModuleWindow.h"
#include "ModuleInput.h"
#include "ModuleEditor.h"
#include "ModuleScene.h"
#include "RE_InternalResources.h"
#include "RE_ResourceManager.h"
#include "RE_ShaderImporter.h"
#include "RE_ThumbnailManager.h"

#include "RE_Shader.h"
#include "RE_SkyBox.h"
#include "RE_Scene.h"
#include "RE_Prefab.h"
#include "RE_Model.h"
#include "RE_Material.h"

#include "RE_CompCamera.h"
#include "RE_CompMesh.h"
#include "RE_CompPrimitive.h"
#include "RE_CameraManager.h"
#include "RE_CompTransform.h"

#include "OutputLog.h"
#include "RE_TimeManager.h"
#include "RE_GLCacheManager.h"
#include "RE_FBOManager.h"

#include <EASTL/list.h>
#include <EASTL/stack.h>

#include "SDL2/include/SDL.h"
#include "Glew/include/glew.h"
#include <gl/GL.h>

#include "MathGeoLib/include/Math/float3.h"
#include "MathGeoLib/include/Math/Quat.h"
#include "ImGuizmo/ImGuizmo.h"

#pragma comment(lib, "Glew/lib/glew32.lib")
#pragma comment(lib, "opengl32.lib")

RE_FBOManager ModuleRenderer3D::fbomanager;
LightMode ModuleRenderer3D::current_lighting = LIGHT_GL;
unsigned int ModuleRenderer3D::current_fbo = 0;

const char* RenderView::labels[12] = {
						"Fustrum Culling", "Override Culling", "Outline Selection", "Debug Draw", "Skybox", "Blending",
						"Wireframe", "Face Culling", "Texture 2D", "Color Material", "Depth Testing", "Clip Distance"};

ModuleRenderer3D::ModuleRenderer3D(const char * name, bool start_enabled) : Module(name, start_enabled) {}
ModuleRenderer3D::~ModuleRenderer3D() {}

bool ModuleRenderer3D::Init(JSONNode * node)
{
	bool ret = false;
	RE_LOG_SECONDARY("Seting SDL/GL Attributes.");
	if (SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE) < 0)
		RE_LOG_ERROR("SDL could not set GL Attributes: 'SDL_GL_CONTEXT_PROFILE_MASK: SDL_GL_CONTEXT_PROFILE_CORE'");
	if (SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1) < 0)
		RE_LOG_ERROR("SDL could not set GL Attributes: 'SDL_GL_DOUBLEBUFFER: 1'");
	if (SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24) < 0)
		RE_LOG_ERROR("SDL could not set GL Attributes: 'SDL_GL_DEPTH_SIZE: 24'");
	if (SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8) < 0)
		RE_LOG_ERROR("SDL could not set GL Attributes: 'SDL_GL_STENCIL_SIZE: 8'");
	if (SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3) < 0)
		RE_LOG_ERROR("SDL could not set GL Attributes: 'SDL_GL_CONTEXT_MAJOR_VERSION: 3'");
	if (SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1) < 0)
		RE_LOG_ERROR("SDL could not set GL Attributes: 'SDL_GL_CONTEXT_MINOR_VERSION: 1'");
	
	if (App::window)
	{
		RE_LOG_SECONDARY("Creating SDL GL Context");
		mainContext = SDL_GL_CreateContext(App::window->GetWindow());
		if (ret = (mainContext != nullptr))
			App::ReportSoftware("OpenGL", reinterpret_cast<const char*>(glGetString(GL_VERSION)), "https://www.opengl.org/");
		else
			RE_LOG_ERROR("SDL could not create GL Context! SDL_Error: %s", SDL_GetError());
	}

	if (ret)
	{
		RE_LOG_SECONDARY("Initializing Glew");
		GLenum error = glewInit();
		if (ret = (error == GLEW_OK))
		{
			render_views.push_back(RenderView("Scene", { 0, 0 },
				FRUSTUM_CULLING | OVERRIDE_CULLING | OUTLINE_SELECTION /*| DEBUG_DRAW*/ | SKYBOX | BLENDED |
				FACE_CULLING | TEXTURE_2D | COLOR_MATERIAL | DEPTH_TEST,
				LIGHT_DEFERRED));
			
			render_views.push_back(RenderView("Game", { 0, 0 },
				FRUSTUM_CULLING | SKYBOX | BLENDED |
				FACE_CULLING | TEXTURE_2D | COLOR_MATERIAL | DEPTH_TEST,
				LIGHT_DEFERRED));

			//Thumbnail Configuration
			thumbnailView.fbos = { RE_FBOManager::CreateFBO(THUMBNAILSIZE, THUMBNAILSIZE),0 };
			thumbnailView.clear_color = {0.f, 0.f, 0.f, 1.f};
			Event::PauseEvents();
			thumbnailView.camera = new RE_CompCamera();
			thumbnailView.camera->SetParent(0ull);
			thumbnailView.camera->SetProperties();
			thumbnailView.camera->SetBounds(THUMBNAILSIZE, THUMBNAILSIZE);
			thumbnailView.camera->Update();
			Event::ResumeEvents();

			RE_PrimitiveManager::PlatonicData meshInfo = App::primitives.CreateSphere(24, 24);
			mat_vao = meshInfo.vao;
			mat_vbo = meshInfo.vbo;
			mat_ebo = meshInfo.ebo;
			mat_triangles = meshInfo.triangles;

			//OpenGL Debug Output
			glEnable(GL_DEBUG_OUTPUT);
			glDebugMessageCallback(MessageCallback, 0);

			glPolygonMode(GL_FRONT_AND_BACK, wireframe ? GL_LINE : GL_FILL);
			cullface ? glEnable(GL_CULL_FACE) : glDisable(GL_CULL_FACE);
			texture2d ? glEnable(GL_TEXTURE_2D) : glDisable(GL_TEXTURE_2D);
			color_material ? glEnable(GL_COLOR_MATERIAL) : glDisable(GL_COLOR_MATERIAL);
			depthtest ? glEnable(GL_DEPTH_TEST) : glDisable(GL_DEPTH_TEST);
			lighting ? glEnable(GL_LIGHTING) : glDisable(GL_LIGHTING);


			Load(node);
			App::ReportSoftware("Glew", reinterpret_cast<const char*>(glewGetString(GLEW_VERSION)), "http://glew.sourceforge.net/");
		}
		else
		{
			RE_LOG_ERROR("Glew could not initialize! Glew_Error: %s", glewGetErrorString(error));
		}
	}

	return ret;
}

bool ModuleRenderer3D::Start()
{
	render_views[VIEW_EDITOR].fbos = {
		RE_FBOManager::CreateFBO(1024, 768, 1, true, false),
		RE_FBOManager::CreateDeferredFBO(1024, 768) };

	render_views[VIEW_GAME].fbos = {
		RE_FBOManager::CreateFBO(1024, 768),
		RE_FBOManager::CreateDeferredFBO(1024, 768) };

	return true;
}

update_status ModuleRenderer3D::PreUpdate()
{
	OPTICK_CATEGORY("PreUpdate Renderer3D", Optick::Category::GameLogic);

	update_status ret = UPDATE_CONTINUE;

	return ret;
}

update_status ModuleRenderer3D::PostUpdate()
{
	OPTICK_CATEGORY("PostUpdate Renderer3D", Optick::Category::GameLogic);
	update_status ret = UPDATE_CONTINUE;


	// Setup Draws
	activeShaders = App::resources->GetAllResourcesActiveByType(Resource_Type::R_SHADER);

	current_lighting = LightMode::LIGHT_DISABLED;


	while (!rendQueue.empty()) {
		RenderQueue rend = rendQueue.top();
		rendQueue.pop();

		RE_GOManager* poolGOThumbnail = nullptr;
		eastl::string path(THUMBNAILPATH);
		path += rend.resMD5;

		bool exist = App::fs->Exists(path.c_str());
		if (rend.redo && exist) {
			RE_FileIO fileToDelete(path.c_str(), App::fs->GetZipPath());
			fileToDelete.Delete();
			exist = false;
		}

		switch (rend.type)
		{
		case RenderType::T_R_GO:
		{
			Event::PauseEvents();
			if (!exist) {
				ResourceContainer* res = App::resources->At(rend.resMD5);
				App::resources->Use(rend.resMD5);
				switch (res->GetType()) {
				case Resource_Type::R_MODEL: poolGOThumbnail = dynamic_cast<RE_Model*>(res)->GetPool(); break;
				case Resource_Type::R_PREFAB: poolGOThumbnail = dynamic_cast<RE_Prefab*>(res)->GetPool(); break;
				case Resource_Type::R_SCENE: poolGOThumbnail = dynamic_cast<RE_Scene*>(res)->GetPool(); break;
				}
				poolGOThumbnail->UseResources();
				ThumbnailGameObject(poolGOThumbnail->GetRootPtr());
				poolGOThumbnail->UnUseResources();
				App::resources->UnUse(rend.resMD5);
				App::thumbnail->SaveTextureFromFBO(path.c_str());
				DEL(poolGOThumbnail);
			}

			App::thumbnail->Change(rend.resMD5, App::thumbnail->LoadLibraryThumbnail(rend.resMD5));
			Event::ResumeEvents();

			break;
		}
		case RenderType::T_R_MAT:
		{
			if (!exist)
			{
				ResourceContainer* res = App::resources->At(rend.resMD5);
				App::resources->Use(rend.resMD5);
				ThumbnailMaterial(dynamic_cast<RE_Material*>(res));
				App::resources->UnUse(rend.resMD5);
				App::thumbnail->SaveTextureFromFBO(path.c_str());
			}

			App::thumbnail->Change(rend.resMD5, App::thumbnail->LoadLibraryThumbnail(rend.resMD5));
			break;
		}
		case RenderType::T_R_TEX:
			App::thumbnail->Change(rend.resMD5,App::thumbnail->ThumbnailTexture(rend.resMD5));
			break;
		case RenderType::T_R_SKYBOX:
			if (!exist)
			{
				ResourceContainer* res = App::resources->At(rend.resMD5);
				App::resources->Use(rend.resMD5);
				ThumbnailSkyBox(dynamic_cast<RE_SkyBox*>(res));
				App::resources->UnUse(rend.resMD5);
				App::thumbnail->SaveTextureFromFBO(path.c_str());
			}
			App::thumbnail->Change(rend.resMD5, App::thumbnail->LoadLibraryThumbnail(rend.resMD5));
			break;
		}

	}

	render_views[0].camera = RE_CameraManager::EditorCamera();
	render_views[1].camera = RE_CameraManager::MainCamera();
	PushSceneRend(render_views[1]);
	PushSceneRend(render_views[0]);

	while (!rendQueue.empty()) {
		RenderQueue rend = rendQueue.top();
		rendQueue.pop();

		switch (rend.type)
		{
		case RenderType::R_R_SCENE:
			DrawScene(rend.renderview);
			break;
		}
	}

	// Set Render to Window
	RE_FBOManager::ChangeFBOBind(0, App::window->GetWidth(), App::window->GetHeight());

	// Draw Editor
	SetWireframe(false);
	App::editor->Draw();

	//Swap buffers
	SDL_GL_SwapWindow(App::window->GetWindow());

	return ret;
}

bool ModuleRenderer3D::CleanUp()
{
	//Delete context
	SDL_GL_DeleteContext(mainContext);

	return true;
}

void ModuleRenderer3D::RecieveEvent(const Event & e)
{
	switch (e.type)
	{
	case SET_VSYNC:
	{
		SetVSync(e.data1.AsBool());
		break;
	}
	case SET_DEPTH_TEST:
	{
		SetDepthTest(e.data1.AsBool());
		break;
	}
	case SET_FACE_CULLING:
	{
		SetFaceCulling(e.data1.AsBool());
		break;
	}
	case SET_LIGHTNING:
	{
		SetLighting(e.data1.AsBool());
		break;
	}
	case SET_TEXTURE_TWO_D:
	{
		SetTexture2D(e.data1.AsBool());
		break;
	}
	case SET_COLOR_MATERIAL:
	{
		SetColorMaterial(e.data1.AsBool());
		break;
	}
	case SET_WIRE_FRAME:
	{
		SetWireframe(e.data1.AsBool());
		break;
	}
	case EDITORWINDOWCHANGED:
	{
		const int window_size[2] = { e.data1.AsInt(), e.data2.AsInt() };
		RE_CameraManager::EditorCamera()->SetBounds(static_cast<float>(window_size[0]), static_cast<float>(window_size[1]));
		ChangeFBOSize(window_size[0], window_size[1], true);
		break;
	}
	case GAMEWINDOWCHANGED:
	{
		const int window_size[2] = { e.data1.AsInt(), e.data2.AsInt() };
		App::cams.OnWindowChangeSize(static_cast<float>(window_size[0]), static_cast<float>(window_size[1]));
		ChangeFBOSize(window_size[0], window_size[1]);
		break;
	}
	}
}

void ModuleRenderer3D::DrawEditor()
{
	if(ImGui::CollapsingHeader("Renderer 3D"))
	{
		if (ImGui::Checkbox((vsync) ? "VSync Enabled" : "VSync Disabled", &vsync))
			SetVSync(vsync);

		for (unsigned int i = 0; i < render_views.size(); ++i)
		{
			if (ImGui::TreeNodeEx((render_views[i].name + " View").c_str(), ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_OpenOnArrow))
			{
				int light = render_views[i].light;
				ImGui::PushID(eastl::string("light" + eastl::to_string(i)).c_str());
				if (ImGui::Combo("Light Mode", &light, "LIGHT_DISABLED\0LIGHT_GL\0LIGHT_DIRECT\0LIGHT_DEFERRED"))
					render_views[i].light = LightMode(light);
				ImGui::PopID();

				for (short count = 0; count < 12; ++count)
				{
					bool temp = (render_views[i].flags & (1 << count));
					ImGui::PushID(eastl::string(RenderView::labels[count] + eastl::to_string(i)).c_str());
					if (ImGui::Checkbox(RenderView::labels[count], &temp))
						temp ? render_views[i].flags |= (1 << count) : render_views[i].flags -= (1 << count);
					ImGui::PopID();
				}

				ImGui::TreePop();
			}
		}
	}
}

bool ModuleRenderer3D::Load(JSONNode * node)
{
	bool ret = (node != nullptr);
	RE_LOG_SECONDARY("Loading Render3D config values:");

	if (ret)
	{
		SetVSync(node->PullBool("vsync", vsync));
		RE_LOG_TERCIARY((vsync)? "VSync enabled." : "VSync disabled");

		//TODO RUB: Load render view loading
		/*for (int i = 0; i < render_views.size(); ++i)
		{
			render_views[i].name = node->PullString((eastl::string("Render view ") + eastl::to_string(i)).c_str(), render_views[i].name.c_str());
			render_views[i].light = LightMode(node->PullInt(eastl::string(render_views[i].name + " - lightmode").c_str(), render_views[i].light));

			render_views[i].flags = 0;
			for (int i2 = 0; i2 < 11; ++i2)
				if (node->PullBool(eastl::string(render_views[i].name + " - " + RenderView::labels[i2]).c_str(), render_views[i].flags & (1 << i2)))
					render_views[i].flags &= (1 << i2);
		}*/
	}

	return ret;
}

bool ModuleRenderer3D::Save(JSONNode * node) const
{
	bool ret = (node != nullptr);

	if (ret)
	{
		node->PushBool("vsync", vsync);
		for (unsigned int i = 0; i < render_views.size(); ++i)
		{
			node->PushString((eastl::string("Render view ") + eastl::to_string(i)).c_str(), render_views[i].name.c_str());
			node->PushInt(eastl::string(render_views[i].name + " - lightmode").c_str(), int(render_views[i].light));

			for (int i2 = 0; i2 < 11; ++i2)
				node->PushBool(eastl::string(render_views[i].name + " - " + RenderView:: labels[i2]).c_str(), render_views[i].flags & (1 << i2));
		}
	}

	return ret;
}

void ModuleRenderer3D::SetVSync(bool enable)
{
	SDL_GL_SetSwapInterval((vsync = enable) ? 1 : 0);
}

void* ModuleRenderer3D::GetWindowContext() const
{
	return mainContext;
}

unsigned int ModuleRenderer3D::GetMaxVertexAttributes()
{
	int nrAttributes;
	glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &nrAttributes);
	return nrAttributes;
}

const char* ModuleRenderer3D::GetGPURenderer() const
{
	return reinterpret_cast<const char*>(glGetString(GL_RENDERER));
}

const char* ModuleRenderer3D::GetGPUVendor() const
{
	return reinterpret_cast<const char*>(glGetString(GL_VENDOR));
}

const LightMode ModuleRenderer3D::GetLightMode()
{
	return current_lighting;
}

void ModuleRenderer3D::ChangeFBOSize(int width, int height, bool isEditor)
{
	RE_FBOManager::ChangeFBOSize(render_views[!isEditor].GetFBO(), width, height);
}

unsigned int ModuleRenderer3D::GetRenderedEditorSceneTexture() const
{
	return RE_FBOManager::GetTextureID(render_views[0].GetFBO(), 4 * (render_views[0].light == LIGHT_DEFERRED));
}

unsigned int ModuleRenderer3D::GetDepthTexture() const
{
	RE_GLCacheManager::ChangeTextureBind(0);
	return RE_FBOManager::GetDepthTexture(current_fbo);
}

unsigned int ModuleRenderer3D::GetRenderedGameSceneTexture() const
{
	return RE_FBOManager::GetTextureID(render_views[1].GetFBO(), 4 * (render_views[1].light == LIGHT_DEFERRED));
}

void ModuleRenderer3D::PushSceneRend(RenderView& rV)
{
	rendQueue.push({ RenderType::R_R_SCENE, rV, nullptr});
}

void ModuleRenderer3D::PushThumnailRend(const char* md5, bool redo)
{
	RenderType t = RenderType::T_R_GO;
	Resource_Type rType = App::resources->At(md5)->GetType();
	switch (rType)
	{
	case R_SCENE:
	case R_MODEL:
	case R_PREFAB:
		t = RenderType::T_R_GO;
		rendQueue.push({ t, thumbnailView, md5, redo });
		break;
	case R_MATERIAL:
		t = RenderType::T_R_MAT;
		rendQueue.push({ t, thumbnailView, md5, redo });
		break;
	case R_TEXTURE:
		t = RenderType::T_R_TEX;
		rendQueue.push({ t, thumbnailView, md5, redo });
		break;
	case R_SKYBOX:
		t = RenderType::T_R_SKYBOX;
		rendQueue.push({ t, thumbnailView, md5, redo });
		break;
	}
}

void ModuleRenderer3D::DrawScene(const RenderView& render_view)
{
	OPTICK_CATEGORY(render_view.name.c_str(), Optick::Category::Rendering);

	// Setup Frame Buffer
	current_lighting = render_view.light;
	current_fbo = render_view.GetFBO();

	RE_FBOManager::ChangeFBOBind(current_fbo, RE_FBOManager::GetWidth(current_fbo), RE_FBOManager::GetHeight(current_fbo));
	RE_FBOManager::ClearFBOBuffers(current_fbo, render_view.clear_color.ptr());

	// Setup Render Flags
	SetWireframe(render_view.flags & WIREFRAME);
	SetFaceCulling(render_view.flags & FACE_CULLING);
	SetTexture2D(false);
	SetColorMaterial(render_view.flags & COLOR_MATERIAL);
	SetDepthTest(render_view.flags & DEPTH_TEST);
	bool usingClipDistance;
	SetClipDistance(usingClipDistance = render_view.flags & CLIP_DISTANCE);

	// Upload Shader Uniforms
	for (auto sMD5 : activeShaders)
		static_cast<RE_Shader*>(App::resources->At(sMD5))->UploadMainUniforms(
			render_view.camera,
			static_cast<float>(RE_FBOManager::GetHeight(current_fbo)),
			static_cast<float>(RE_FBOManager::GetWidth(current_fbo)),
			usingClipDistance,
			render_view.clip_distance);

	// Frustum Culling
	eastl::stack<RE_Component*> comptsToDraw;
	if (render_view.flags & FRUSTUM_CULLING)
	{
		eastl::vector<const RE_GameObject*> objects;
		App::scene->FustrumCulling(objects, render_view.flags & OVERRIDE_CULLING ?
			App::cams.GetCullingFrustum() : render_view.camera->GetFrustum());

		for (const RE_GameObject* object : objects)
			if (object->IsActive()) {
				RE_Component* goRend = object->GetRenderGeo();
				if(goRend) comptsToDraw.push(goRend);
			}
	}
	else
		comptsToDraw = App::scene->GetScenePool()->GetRootPtr()->GetAllChildsActiveRenderGeos();

	// Setup Lights
	eastl::vector<RE_Component*> scene_lights;
	switch (render_view.light)
	{
	case LIGHT_DISABLED:
	{
		SetLighting(false);
		break;
	}
	case LIGHT_GL:
	{
		SetLighting(true);
		scene_lights = App::scene->GetScenePool()->GetAllCompPtr(C_LIGHT);

		// TODO RUB: Bind GL Lights

		break;
	}
	case LIGHT_DIRECT:
	{
		SetLighting(false);
		scene_lights = App::scene->GetScenePool()->GetAllCompPtr(C_LIGHT);

		// TODO RUB: Upload Light uniforms

		break;
	}
	case LIGHT_DEFERRED:
	{
		SetLighting(false);
		scene_lights = App::scene->GetScenePool()->GetAllCompPtr(C_LIGHT);
		break;
	}
	}

	// Draw Scene
	eastl::stack<RE_Component*> drawAsLast;
	RE_Component* drawing = nullptr;
	while (!comptsToDraw.empty())
	{
		drawing = comptsToDraw.top();
		bool blend = false;

		if (drawing->GetType() == C_MESH)
			 blend = dynamic_cast<RE_CompMesh*>(drawing)->isBlend();

		if (!blend) drawing->Draw();
		else drawAsLast.push(drawing);

		comptsToDraw.pop();
	}

	// Deferred Light Pass
	if (render_view.light == LIGHT_DEFERRED)
	{
		// Setup Shader
		unsigned int light_pass = dynamic_cast<RE_Shader*>(App::resources->At(App::internalResources.GetLightPassShader()))->GetID();
		RE_GLCacheManager::ChangeShader(light_pass);

		SetDepthTest(false);

		glMemoryBarrierByRegion(GL_FRAMEBUFFER_BARRIER_BIT);

		// Bind Textures
		static const eastl::string deferred_textures[4] = { "gPosition", "gNormal", "gAlbedo", "gSpec" };
		for (unsigned int count = 0; count < 4; ++count)
		{
			glActiveTexture(GL_TEXTURE0 + count);
			RE_ShaderImporter::setInt(light_pass, deferred_textures[count].c_str(), count);
			RE_GLCacheManager::ChangeTextureBind(RE_FBOManager::GetTextureID(current_fbo, count));
		}

		// Setup Light Uniforms
		unsigned int count = 0;

		eastl::string unif_name;
		for (auto l : scene_lights) {
			unif_name = "lights[" + eastl::to_string(count) + "].";
			dynamic_cast<RE_CompLight*>(l)->CallShaderUniforms(light_pass, unif_name.c_str());
			count++;
		}

		for (count; count < 64; count++) {
			unif_name = "lights[" + eastl::to_string(count) + "].";
			RE_ShaderImporter::setFloat(RE_ShaderImporter::getLocation(light_pass, (unif_name + "type").c_str()), -1.0f);
		}

		// Render Lights
		DrawQuad();

		if (render_view.flags & DEPTH_TEST)
			SetDepthTest(true);
	}

	// Draw Debug Geometry
	if (render_view.flags & DEBUG_DRAW)
	{
		RE_GLCacheManager::ChangeShader(0);
		RE_GLCacheManager::ChangeTextureBind(0);

		bool reset_light = lighting;
		SetLighting(false);
		SetTexture2D(false);

		App::editor->DrawDebug(render_view.camera);

		if (reset_light) SetLighting(true);
		SetTexture2D(render_view.flags & TEXTURE_2D);
	}

	// Draw Skybox
	if (render_view.flags & SKYBOX && render_view.camera->isUsingSkybox())
	{
		OPTICK_CATEGORY("SkyBox Draw", Optick::Category::Rendering);

		RE_GLCacheManager::ChangeTextureBind(0);

		uint skysphereshader = static_cast<RE_Shader*>(App::resources->At(App::internalResources.GetDefaultSkyBoxShader()))->GetID();
		RE_GLCacheManager::ChangeShader(skysphereshader);
		RE_ShaderImporter::setInt(skysphereshader, "cubemap", 0);

		glDepthFunc(GL_LEQUAL);
		RE_CameraManager::MainCamera()->DrawSkybox();
		glDepthFunc(GL_LESS); // set depth function back to default
	}

	// Draw Blended elements
	if (render_view.flags & BLENDED && !drawAsLast.empty())
	{
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		while (!drawAsLast.empty())
		{
			drawAsLast.top()->Draw();
			drawAsLast.pop();
		}

		glDisable(GL_BLEND);
	}

	// Draw Stencil
	/*if (render_view.flags & OUTLINE_SELECTION)
	{
		RE_GameObject* stencilGO = App::editor->GetSelected();
		if (stencilGO != nullptr)
		{
			eastl::stack<RE_Component*> stackComponents = stencilGO->GetDrawableComponentsItselfOnly();
			if (!stackComponents.empty())
			{
				OPTICK_CATEGORY("Stencil Draw", Optick::Category::Rendering);
				eastl::stack<unsigned int> vaoToStencil;
				eastl::stack<unsigned int> triangleToStencil;
				eastl::stack<RE_Component*> stackTemp;
				while (!stackComponents.empty())
				{
					RE_Component* dC = stackComponents.top();
					ComponentType cT = dC->GetType();
					if (cT == ComponentType::C_MESH)
					{
						RE_CompMesh* mesh_comp = dynamic_cast<RE_CompMesh*>(dC);
						vaoToStencil.push(mesh_comp->GetVAOMesh());
						triangleToStencil.push(mesh_comp->GetTriangleMesh());
						stackTemp.push(dC);
					}
					else if (cT > ComponentType::C_PRIMIVE_MIN && cT < ComponentType::C_PRIMIVE_MAX)
					{
						RE_CompPrimitive* prim_comp = dynamic_cast<RE_CompPrimitive*>(dC);
						vaoToStencil.push(prim_comp->GetVAO());
						triangleToStencil.push(prim_comp->GetTriangleCount());
						stackTemp.push(dC);
					}
					stackComponents.pop();
				}

				glEnable(GL_STENCIL_TEST);
				SetDepthTest(false);

				while (!vaoToStencil.empty())
				{
					//Getting the scale shader and setting some values
					const char* scaleShader = App::internalResources.GetDefaultScaleShader();
					RE_Shader* sShader = dynamic_cast<RE_Shader*>(App::resources->At(scaleShader));
					unsigned int shaderiD = sShader->GetID();
					RE_GLCacheManager::ChangeShader(shaderiD);
					RE_GLCacheManager::ChangeVAO(vaoToStencil.top());
					sShader->UploadModel(stencilGO->GetTransformPtr()->GetGlobalMatrixPtr());
					RE_ShaderImporter::setFloat(shaderiD, "useColor", 1.0);
					RE_ShaderImporter::setFloat(shaderiD, "useTexture", 0.0);
					RE_ShaderImporter::setFloat(shaderiD, "cdiffuse", { 1.0, 0.5, 0.0 });
					RE_ShaderImporter::setFloat(shaderiD, "center", (stackTemp.top()->GetType() == ComponentType::C_MESH) ? dynamic_cast<RE_CompMesh*>(stackTemp.top())->GetAABB().CenterPoint() : math::vec::zero);

					//Prepare stencil for detect
					glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE); //don't draw to color buffer
					glStencilFunc(GL_ALWAYS, 1, 0xFF); //mark to 1 where pass
					glStencilMask(0xFF);
					glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);

					//Draw scaled mesh 
					RE_ShaderImporter::setFloat(shaderiD, "scaleFactor", 0.5f / stencilGO->GetTransformPtr()->GetLocalScale().Length());
					stackComponents.push(stackTemp.top());
					stackTemp.pop();
					glDrawElements(GL_TRIANGLES, triangleToStencil.top() * 3, GL_UNSIGNED_INT, nullptr);

					glStencilFunc(GL_ALWAYS, 0, 0x00);//change stencil to draw 0
					//Draw normal mesh for empty the inside of stencil
					RE_ShaderImporter::setFloat(shaderiD, "scaleFactor", 0.0);
					glDrawElements(GL_TRIANGLES, triangleToStencil.top() * 3, GL_UNSIGNED_INT, nullptr);

					//Turn on the draw and only draw where stencil buffer marks 1
					glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE); // Make sure we draw on the backbuffer again.
					glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP); // Make sure you will no longer (over)write stencil values, even if any test succeeds
					glStencilFunc(GL_EQUAL, 1, 0xFF); // Now we will only draw pixels where the corresponding stencil buffer value equals 1

					//Draw scaled mesh 
					RE_ShaderImporter::setFloat(shaderiD, "scaleFactor", 0.5f / stencilGO->GetTransformPtr()->GetLocalScale().Length());
					glDrawElements(GL_TRIANGLES, triangleToStencil.top() * 3, GL_UNSIGNED_INT, nullptr);

					vaoToStencil.pop();
					triangleToStencil.pop();
				}

				glDisable(GL_STENCIL_TEST);
				if (render_view.flags & DEPTH_TEST)
					SetDepthTest(true);
			}
		}
	}*/
}

void ModuleRenderer3D::ThumbnailGameObject(RE_GameObject* go)
{
	unsigned int c_fbo = thumbnailView.fbos.first;
	RE_FBOManager::ChangeFBOBind(c_fbo, RE_FBOManager::GetWidth(c_fbo), RE_FBOManager::GetHeight(c_fbo));
	RE_FBOManager::ClearFBOBuffers(c_fbo, thumbnailView.clear_color.ptr());

	go->ResetGOandChildsAABB();

	RE_CompCamera* internalCamera = thumbnailView.camera;

	internalCamera->SetFOV(math::RadToDeg(0.523599f));
	internalCamera->Update();
	internalCamera->Focus(go->GetGlobalBoundingBox().CenterPoint());
	internalCamera->Update();

	for (auto sMD5 : activeShaders) (dynamic_cast<RE_Shader*>(App::resources->At(sMD5)))->UploadMainUniforms(internalCamera, THUMBNAILSIZE, THUMBNAILSIZE, false, {});

	go->DrawChilds();
}

void ModuleRenderer3D::ThumbnailMaterial(RE_Material* mat)
{
	unsigned int c_fbo = thumbnailView.fbos.first;
	RE_FBOManager::ChangeFBOBind(c_fbo, RE_FBOManager::GetWidth(c_fbo), RE_FBOManager::GetHeight(c_fbo));
	RE_FBOManager::ClearFBOBuffers(c_fbo, thumbnailView.clear_color.ptr());

	RE_CompCamera* internalCamera = thumbnailView.camera;

	Event::PauseEvents();
	internalCamera->SetFOV(math::RadToDeg(0.523599f));
	internalCamera->Update();
	internalCamera->LocalRotate(0, 1);
	internalCamera->Update();
	Event::ResumeEvents();

	for (auto sMD5 : activeShaders)
		dynamic_cast<RE_Shader*>(App::resources->At(sMD5))->UploadMainUniforms(internalCamera, THUMBNAILSIZE, THUMBNAILSIZE, false, {});

	mat->UploadToShader(math::float4x4::identity.ptr(), false, true);

	RE_GLCacheManager::ChangeVAO(mat_vao);
	glDrawElements(GL_TRIANGLES, mat_triangles * 3, GL_UNSIGNED_SHORT, 0);
	RE_GLCacheManager::ChangeVAO(0);
	RE_GLCacheManager::ChangeShader(0);
}

void ModuleRenderer3D::ThumbnailSkyBox(RE_SkyBox* skybox)
{
	unsigned int c_fbo = thumbnailView.fbos.first;
	RE_FBOManager::ChangeFBOBind(c_fbo, RE_FBOManager::GetWidth(c_fbo), RE_FBOManager::GetHeight(c_fbo));
	RE_FBOManager::ClearFBOBuffers(c_fbo, thumbnailView.clear_color.ptr());

	RE_CompCamera* internalCamera = thumbnailView.camera;

	Event::PauseEvents();
	internalCamera->ForceFOV(125, 140);
	internalCamera->GetTransform()->SetRotation({ 0.0,0.0,0.0 });
	internalCamera->GetTransform()->SetPosition(math::vec(0.f, 0.f, 0.f));
	internalCamera->Update();
	Event::ResumeEvents();

	for (auto sMD5 : activeShaders)
		dynamic_cast<RE_Shader*>(App::resources->At(sMD5))->UploadMainUniforms(internalCamera, THUMBNAILSIZE, THUMBNAILSIZE, false, {});

	RE_GLCacheManager::ChangeTextureBind(0);

	RE_Shader* skyboxShader = (RE_Shader*)App::resources->At(App::internalResources.GetDefaultSkyBoxShader());
	uint skysphereshader = skyboxShader->GetID();
	RE_GLCacheManager::ChangeShader(skysphereshader);
	RE_ShaderImporter::setInt(skysphereshader, "cubemap", 0);
	skybox->DrawSkybox();
}

inline void ModuleRenderer3D::SetWireframe(bool enable)
{
	if (wireframe != enable)
		glPolygonMode(GL_FRONT_AND_BACK, (wireframe = enable) ? GL_LINE : GL_FILL);
}

inline void ModuleRenderer3D::SetFaceCulling(bool enable)
{
	if (cullface != enable)
		(cullface = enable) ? glEnable(GL_CULL_FACE) : glDisable(GL_CULL_FACE);
}

inline void ModuleRenderer3D::SetTexture2D(bool enable)
{
	if (texture2d != enable)
		(texture2d = enable) ? glEnable(GL_TEXTURE_2D) : glDisable(GL_TEXTURE_2D);
}

inline void ModuleRenderer3D::SetColorMaterial(bool enable)
{
	if (color_material != enable)
		(color_material = enable) ? glEnable(GL_COLOR_MATERIAL) : glDisable(GL_COLOR_MATERIAL);
}

inline void ModuleRenderer3D::SetDepthTest(bool enable)
{
	if (depthtest != enable)
		(depthtest = enable) ? glEnable(GL_DEPTH_TEST) : glDisable(GL_DEPTH_TEST);
}

inline void ModuleRenderer3D::SetLighting(bool enable)
{
	if (lighting != enable)
		(lighting = enable) ? glEnable(GL_LIGHTING) : glDisable(GL_LIGHTING);
}

inline void ModuleRenderer3D::SetClipDistance(bool enable)
{
	if (clip_distance != enable)
		(clip_distance = enable) ? glEnable(GL_CLIP_DISTANCE0) : glDisable(GL_CLIP_DISTANCE0);
}

void ModuleRenderer3D::DrawQuad()
{
	// Setup Screen Quad
	static unsigned int quadVAO = 0;
	if (quadVAO == 0)
	{
		float quadVertices[] = {
			// positions        // texture Coords
			-1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
			-1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
			 1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
			 1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
		};
		// setup plane VAO
		unsigned int quadVBO;
		glGenVertexArrays(1, &quadVAO);
		glGenBuffers(1, &quadVBO);
		glBindVertexArray(quadVAO);
		glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	}

	// Render Quad
	RE_GLCacheManager::ChangeVAO(quadVAO);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	RE_GLCacheManager::ChangeVAO(0);
	RE_GLCacheManager::ChangeShader(0);
}

void ModuleRenderer3D::DirectDrawCube(math::vec position, math::vec color)
{
	glColor3f(color.x, color.y, color.z);

	math::float4x4 model = math::float4x4::Translate(position.Neg());
	model.InverseTranspose();

	glMatrixMode(GL_MODELVIEW);
	glLoadMatrixf((RE_CameraManager::CurrentCamera()->GetView() * model).ptr());

	glBegin(GL_TRIANGLES);
	glVertex3f(-1.0f, -1.0f, -1.0f);
	glVertex3f(1.0f, -1.0f, -1.0f);
	glVertex3f(1.0f, 1.0f, -1.0f);
	glVertex3f(1.0f, 1.0f, -1.0f);
	glVertex3f(- 1.0f, 1.0f, -1.0f);
	glVertex3f(- 1.0f, -1.0f, -1.0f);

	glVertex3f(- 1.0f, -1.0f, 1.0f);
	glVertex3f(1.0f, -1.0f, 1.0f);
	glVertex3f(1.0f, 1.0f, 1.0f);
	glVertex3f(1.0f, 1.0f, 1.0f);
	glVertex3f(- 1.0f, 1.0f, 1.0f);
	glVertex3f(- 1.0f, -1.0f, 1.0f);

	glVertex3f(-1.0f, 1.0f, 1.0f);
	glVertex3f(-1.0f, 1.0f, -1.0f);
	glVertex3f(-1.0f, -1.0f, -1.0f);
	glVertex3f(-1.0f, -1.0f, -1.0f);
	glVertex3f(-1.0f, -1.0f, 1.0f);
	glVertex3f(- 1.0f, 1.0f, 1.0f);

	glVertex3f(1.0f, 1.0f, 1.0f);
	glVertex3f(1.0f, 1.0f, -1.0f);
	glVertex3f(1.0f, -1.0f, -1.0f);
	glVertex3f(1.0f, -1.0f, -1.0f);
	glVertex3f(1.0f, -1.0f, 1.0f);
	glVertex3f(1.0f, 1.0f, 1.0f);

	glVertex3f(-1.0f, -1.0f, -1.0f);
	glVertex3f(1.0f, -1.0f, -1.0f);
	glVertex3f(1.0f, -1.0f, 1.0f);
	glVertex3f(1.0f, -1.0f, 1.0f);
	glVertex3f(-1.0f, -1.0f, 1.0f);
	glVertex3f(- 1.0f, -1.0f, -1.0f);

	glVertex3f(-1.0f, 1.0f, -1.0f);
	glVertex3f(1.0f, 1.0f, -1.0f);
	glVertex3f(1.0f, 1.0f, 1.0f);
	glVertex3f(1.0f, 1.0f, 1.0f);
	glVertex3f(-1.0f, 1.0f, 1.0f);
	glVertex3f(- 1.0f, 1.0f, -1.0f);
	glEnd();
}

RenderView::RenderView(eastl::string name, eastl::pair<unsigned int, unsigned int> fbos, short flags, LightMode light, math::float4 clipDistance) :
	name(name), fbos(fbos), flags(flags), light(light)
{
	clear_color = math::float4::one;
	clip_distance = clipDistance;
}

const unsigned int RenderView::GetFBO() const
{
	return light != LIGHT_DEFERRED ? fbos.first : fbos.second;
}