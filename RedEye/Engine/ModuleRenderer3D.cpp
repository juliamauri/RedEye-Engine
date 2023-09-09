#include "Event.h"
#include <MGL/Math/float4.h>
#include <EASTL/string.h>

#include "ModuleRenderer3D.h"

#include "RE_Memory.h"
#include "Application.h"
#include "ModuleWindow.h"
#include "ModuleInput.h"
#include "ModuleEditor.h"
#include "ModuleScene.h"
#include "ModulePhysics.h"
#include "RE_Profiler.h"
#include "RE_ConsoleLog.h"
#include "RE_FileSystem.h"
#include "RE_FileBuffer.h"
#include "RE_Config.h"
#include "RE_Json.h"
#include "RE_GLCache.h"
#include "RE_FBOManager.h"
#include "RE_ResourceManager.h"
#include "RE_InternalResources.h"
#include "RE_ShaderImporter.h"
#include "RE_ThumbnailManager.h"
#include "RE_CameraManager.h"
#include "RE_PrimitiveManager.h"
#include "RE_CompTransform.h"
#include "RE_CompCamera.h"
#include "RE_CompMesh.h"
#include "RE_CompPrimitive.h"
#include "RE_Shader.h"
#include "RE_SkyBox.h"
#include "RE_Scene.h"
#include "RE_Prefab.h"
#include "RE_Model.h"
#include "RE_Material.h"
#include "RE_DefaultShaders.h"

#include <MGL/Math/float3.h>
#include <MGL/Math/Quat.h>
#include <ImGuiWidgets/ImGuizmo/ImGuizmo.h>
#include <SDL2/SDL.h>
#include <GL/glew.h>
#include <gl/GL.h>
#include <EASTL/list.h>
#include <EASTL/stack.h>
#include <EASTL/array.h>

// OpenGL Output Debug
void GLAPIENTRY MessageCallback(
	GLenum source,
	GLenum type,
	GLuint id,
	GLenum severity,
	GLsizei length,
	const GLchar* message,
	const void* userParam)
{
	if (severity == GL_DEBUG_SEVERITY_NOTIFICATION || type == GL_DEBUG_TYPE_OTHER) return;

	eastl::string severityStr = "Severiry ";
	switch (severity) {
	case GL_DEBUG_SEVERITY_LOW: severityStr += "low"; break;
	case GL_DEBUG_SEVERITY_MEDIUM: severityStr += "medium"; break;
	case GL_DEBUG_SEVERITY_HIGH: severityStr += "high"; break;
	default: severityStr += "not specified."; break; }

	eastl::string typeStr = "Type ";
	switch (type) {
	case GL_DEBUG_TYPE_ERROR: typeStr += "GL ERROR"; break;
	case GL_DEBUG_TYPE_PORTABILITY: typeStr += "GL PORTABILITY"; break;
	case GL_DEBUG_TYPE_PERFORMANCE: typeStr += "GL PERFORMANCE"; break;
	case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: typeStr += "GL DEPRECATED"; break;
	default: typeStr += "GL UNDEFINED"; break; }

	if (type == GL_DEBUG_TYPE_ERROR) RE_LOG_ERROR("%s, %s, message = %s\n", (typeStr.c_str()), severityStr.c_str(), message);
	else RE_LOG_WARNING("%s, %s, message = %s\n", (typeStr.c_str()), severityStr.c_str(), message);
}

RenderView::LightMode ModuleRenderer3D::current_lighting = RenderView::LightMode::GL;
RE_CompCamera* ModuleRenderer3D::current_camera = nullptr;
unsigned int ModuleRenderer3D::current_fbo = 0;

ModuleRenderer3D::ModuleRenderer3D() : fbos(new RE_FBOManager()) {}
ModuleRenderer3D::~ModuleRenderer3D() { DEL(fbos) }

bool ModuleRenderer3D::Init()
{
	RE_PROFILE(RE_ProfiledFunc::Init, RE_ProfiledClass::ModuleRender);
	bool ret = false;
	RE_LOG("Initializing Module Renderer3D");
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
	
	if (RE_WINDOW)
	{
		RE_LOG_SECONDARY("Creating SDL GL Context");
		mainContext = SDL_GL_CreateContext(RE_WINDOW->GetWindow());
		if (ret = (mainContext != nullptr))
			RE_SOFT_NVS("OpenGL", reinterpret_cast<const char*>(glGetString(GL_VERSION)), "https://www.opengl.org/");
		else
			RE_LOG_ERROR("SDL could not create GL Context! SDL_Error: %s", SDL_GetError());
	}

	if (ret)
	{
		RE_LOG_SECONDARY("Initializing Glew");
		GLenum error = glewInit();
		if (ret = (error == GLEW_OK))
		{
			RE_SOFT_NVS("Glew", reinterpret_cast<const char*>(glewGetString(GLEW_VERSION)), "http://glew.sourceforge.net/");

			int test;
			glGetIntegerv(GL_MAX_VERTEX_UNIFORM_BLOCKS, &test);
			glGetIntegerv(GL_MAX_GEOMETRY_UNIFORM_BLOCKS, &test);
			glGetIntegerv(GL_MAX_FRAGMENT_UNIFORM_BLOCKS, &test);
			glGetIntegerv(GL_MAX_UNIFORM_BLOCK_SIZE, &test);
			glGetIntegerv(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT, &test);
			glGetIntegerv(GL_MAX_FRAGMENT_UNIFORM_COMPONENTS, &test);
			glGetIntegerv(GL_MAX_VERTEX_UNIFORM_COMPONENTS, &test);
			glGetIntegerv(GL_MAX_GEOMETRY_UNIFORM_COMPONENTS, &test);

			render_views.push_back(RenderView("Scene", { 0, 0 },
				static_cast<const ushort>(RenderView::Flag::FRUSTUM_CULLING) |
				static_cast<const ushort>(RenderView::Flag::OVERRIDE_CULLING) |
				static_cast<const ushort>(RenderView::Flag::OUTLINE_SELECTION) |
				static_cast<const ushort>(RenderView::Flag::SKYBOX) |
				static_cast<const ushort>(RenderView::Flag::BLENDED) |
				static_cast<const ushort>(RenderView::Flag::FACE_CULLING) |
				static_cast<const ushort>(RenderView::Flag::TEXTURE_2D) |
				static_cast<const ushort>(RenderView::Flag::COLOR_MATERIAL) |
				static_cast<const ushort>(RenderView::Flag::DEPTH_TEST),
				RenderView::LightMode::DISABLED));
			
			render_views.push_back(RenderView("Game", { 0, 0 },
				static_cast<const ushort>(RenderView::Flag::FRUSTUM_CULLING) |
				static_cast<const ushort>(RenderView::Flag::SKYBOX) |
				static_cast<const ushort>(RenderView::Flag::BLENDED) |
				static_cast<const ushort>(RenderView::Flag::FACE_CULLING) |
				static_cast<const ushort>(RenderView::Flag::TEXTURE_2D) |
				static_cast<const ushort>(RenderView::Flag::COLOR_MATERIAL) |
				static_cast<const ushort>(RenderView::Flag::DEPTH_TEST),
				RenderView::LightMode::DEFERRED));

			render_views.push_back(RenderView("Particle Editor", { 0, 0 },
				static_cast<const ushort>(RenderView::Flag::FACE_CULLING) |
				static_cast<const ushort>(RenderView::Flag::BLENDED) |
				static_cast<const ushort>(RenderView::Flag::TEXTURE_2D) |
				static_cast<const ushort>(RenderView::Flag::COLOR_MATERIAL) |
				static_cast<const ushort>(RenderView::Flag::DEPTH_TEST),
				RenderView::LightMode::DISABLED));

			//Thumbnail Configuration
			thumbnailView.clear_color = {0.f, 0.f, 0.f, 1.f};
			RE_INPUT->PauseEvents();
			thumbnailView.camera = new RE_CompCamera();
			thumbnailView.camera->SetParent(0ull);
			thumbnailView.camera->SetProperties();
			thumbnailView.camera->SetBounds(THUMBNAILSIZE, THUMBNAILSIZE);
			thumbnailView.camera->Update();
			RE_INPUT->ResumeEvents();
			
			RE_SCENE->primitives->CreateSphere(24, 24, mat_vao, mat_vbo, mat_ebo, mat_triangles);

			//OpenGL Debug Output
			glEnable(GL_DEBUG_OUTPUT);
			glDebugMessageCallback(MessageCallback, 0);

			glPolygonMode(GL_FRONT_AND_BACK, wireframe ? GL_LINE : GL_FILL);
			cullface ? glEnable(GL_CULL_FACE) : glDisable(GL_CULL_FACE);
			texture2d ? glEnable(GL_TEXTURE_2D) : glDisable(GL_TEXTURE_2D);
			color_material ? glEnable(GL_COLOR_MATERIAL) : glDisable(GL_COLOR_MATERIAL);
			depthtest ? glEnable(GL_DEPTH_TEST) : glDisable(GL_DEPTH_TEST);
			lighting ? glEnable(GL_LIGHTING) : glDisable(GL_LIGHTING);

			Load();
		}
		else RE_LOG_ERROR("Glew could not initialize! Glew_Error: %s", glewGetErrorString(error));
	}
	 
	return ret;
}

bool ModuleRenderer3D::Start()
{
	RE_PROFILE(RE_ProfiledFunc::Start, RE_ProfiledClass::ModuleRender);
	RE_LOG("Starting Module Renderer3D");
	thumbnailView.fbos = { fbos->CreateFBO(THUMBNAILSIZE, THUMBNAILSIZE),0 };

	render_views[static_cast<const ushort>(RenderView::Type::EDITOR)].fbos =
	{
		fbos->CreateFBO(1024, 768, 1, true, true),
		fbos->CreateDeferredFBO(1024, 768)
	};

	render_views[static_cast<const ushort>(RenderView::Type::GAME)].fbos =
	{
		fbos->CreateFBO(1024, 768, 1, true, true),
		fbos->CreateDeferredFBO(1024, 768)
	};

	render_views[static_cast<const ushort>(RenderView::Type::PARTICLE)].fbos =
	{
		fbos->CreateFBO(1024, 768, 1, true, false),
		fbos->CreateDeferredFBO(1024, 768)
	};

	return true;
}

void ModuleRenderer3D::PostUpdate()
{
	RE_PROFILE(RE_ProfiledFunc::PostUpdate, RE_ProfiledClass::ModuleRender);
	{
		RE_PROFILE(RE_ProfiledFunc::GetActiveShaders, RE_ProfiledClass::ModuleRender);
		// Setup Draws
		activeShaders = RE_RES->GetAllResourcesActiveByType(ResourceContainer::Type::SHADER);
	}
	{
		RE_PROFILE(RE_ProfiledFunc::DrawThumbnails, RE_ProfiledClass::ModuleRender);
		current_lighting = RenderView::LightMode::DISABLED;
		while (!render_queue.empty())
		{
			RenderQueue rend = render_queue.top();
			render_queue.pop();

			RE_ECS_Pool* poolGOThumbnail = nullptr;
			eastl::string path(THUMBNAILPATH);
			path += rend.resMD5;

			bool exist = RE_FS->Exists(path.c_str());
			if (rend.redo && exist)
			{
				RE_FileBuffer fileToDelete(path.c_str());
				fileToDelete.Delete();
				exist = false;
			}

			RE_GLCache::ChangeVAO(0);

			switch (rend.type)
			{
			case RenderType::GO:
			{
				RE_INPUT->PauseEvents();
				if (!exist)
				{
					ResourceContainer* res = RE_RES->At(rend.resMD5);
					RE_RES->Use(rend.resMD5);
					switch (res->GetType())
					{
					case ResourceContainer::Type::MODEL: poolGOThumbnail = dynamic_cast<RE_Model*>(res)->GetPool(); break;
					case ResourceContainer::Type::PREFAB: poolGOThumbnail = dynamic_cast<RE_Prefab*>(res)->GetPool(); break;
					case ResourceContainer::Type::SCENE: poolGOThumbnail = dynamic_cast<RE_Scene*>(res)->GetPool(); break;
					default: break;
					}
					if (poolGOThumbnail != nullptr)
					{
						poolGOThumbnail->UseResources();
						ThumbnailGameObject(poolGOThumbnail->GetRootPtr());
						poolGOThumbnail->UnUseResources();
						RE_RES->UnUse(rend.resMD5);
						RE_EDITOR->thumbnails->SaveTextureFromFBO(path.c_str());
						DEL(poolGOThumbnail)
					}
				}

				RE_EDITOR->thumbnails->Change(rend.resMD5, RE_EDITOR->thumbnails->LoadLibraryThumbnail(rend.resMD5));
				RE_INPUT->ResumeEvents();

				break;
			}
			case RenderType::MAT:
			{
				if (!exist)
				{
					ResourceContainer* res = RE_RES->At(rend.resMD5);
					RE_RES->Use(rend.resMD5);
					dynamic_cast<RE_Material*>(res)->UseResources();
					ThumbnailMaterial(dynamic_cast<RE_Material*>(res));
					dynamic_cast<RE_Material*>(res)->UnUseResources();
					RE_RES->UnUse(rend.resMD5);
					RE_EDITOR->thumbnails->SaveTextureFromFBO(path.c_str());
				}

				RE_EDITOR->thumbnails->Change(rend.resMD5, RE_EDITOR->thumbnails->LoadLibraryThumbnail(rend.resMD5));
				break;
			}
			case RenderType::TEX:
				RE_EDITOR->thumbnails->Change(rend.resMD5, RE_EDITOR->thumbnails->ThumbnailTexture(rend.resMD5));
				break;
			case RenderType::SKYBOX:
				if (!exist)
				{
					ResourceContainer* res = RE_RES->At(rend.resMD5);

					RE_RES->Use(rend.resMD5);
					ThumbnailSkyBox(dynamic_cast<RE_SkyBox*>(res));
					RE_RES->UnUse(rend.resMD5);
					RE_EDITOR->thumbnails->SaveTextureFromFBO(path.c_str());
				}
				RE_EDITOR->thumbnails->Change(rend.resMD5, RE_EDITOR->thumbnails->LoadLibraryThumbnail(rend.resMD5));
				break;
			}

		}}

	render_views[0].camera = RE_CameraManager::EditorCamera();
	render_views[1].camera = RE_CameraManager::MainCamera();
	render_views[2].camera = RE_CameraManager::ParticleEditorCamera();
	if (RE_EDITOR->EditorSceneNeedsRender()) PushSceneRend(render_views[0]);
	if (RE_EDITOR->GameSceneNeedsRender()) PushSceneRend(render_views[1]);

	while (!render_queue.empty()) {
		RenderQueue rend = render_queue.top();
		render_queue.pop();

		switch (rend.type)
		{
		case RenderType::SCENE:
			DrawScene(rend.renderview);
			break;
		}
	}

	if (RE_EDITOR->IsParticleEditorActive())
		DrawParticleEditor(render_views[2]);

	// Set Render to Window
	fbos->ChangeFBOBind(0, RE_WINDOW->GetWidth(), RE_WINDOW->GetHeight());

	// Draw Editor
	SetWireframe(false);
	RE_EDITOR->Draw();

	//Swap buffers
	SDL_GL_SwapWindow(RE_WINDOW->GetWindow());
}

void ModuleRenderer3D::CleanUp()
{
	RE_PROFILE(RE_ProfiledFunc::CleanUp, RE_ProfiledClass::ModuleRender);
	fbos->ClearAll();
	SDL_GL_DeleteContext(mainContext);
}

void ModuleRenderer3D::RecieveEvent(const Event & e)
{
	switch (e.type)
	{
	case RE_EventType::SET_VSYNC:
	{
		SetVSync(e.data1.AsBool());
		break;
	}
	case RE_EventType::SET_DEPTH_TEST:
	{
		SetDepthTest(e.data1.AsBool());
		break;
	}
	case RE_EventType::SET_FACE_CULLING:
	{
		SetFaceCulling(e.data1.AsBool());
		break;
	}
	case RE_EventType::SET_LIGHTNING:
	{
		SetLighting(e.data1.AsBool());
		break;
	}
	case RE_EventType::SET_TEXTURE_TWO_D:
	{
		SetTexture2D(e.data1.AsBool());
		break;
	}
	case RE_EventType::SET_COLOR_MATERIAL:
	{
		SetColorMaterial(e.data1.AsBool());
		break;
	}
	case RE_EventType::SET_WIRE_FRAME:
	{
		SetWireframe(e.data1.AsBool());
		break;
	}
	case RE_EventType::EDITORWINDOWCHANGED:
	{
		const int window_size[2] = { e.data1.AsInt(), e.data2.AsInt() };
		RE_CameraManager::EditorCamera()->SetBounds(static_cast<float>(window_size[0]), static_cast<float>(window_size[1]));
		ChangeFBOSize(window_size[0], window_size[1], RenderView::Type::EDITOR);
		break;
	}
	case RE_EventType::GAMEWINDOWCHANGED:
	{
		const int window_size[2] = { e.data1.AsInt(), e.data2.AsInt() };
		RE_SCENE->cams->OnWindowChangeSize(static_cast<float>(window_size[0]), static_cast<float>(window_size[1]));
		ChangeFBOSize(window_size[0], window_size[1], RenderView::Type::GAME);
		break;
	}
	case RE_EventType::PARTRICLEEDITORWINDOWCHANGED:
	{
		const int window_size[2] = { e.data1.AsInt(), e.data2.AsInt() };
		RE_CameraManager::ParticleEditorCamera()->SetBounds(static_cast<float>(window_size[0]), static_cast<float>(window_size[1]));
		ChangeFBOSize(window_size[0], window_size[1], RenderView::Type::PARTICLE);
		break;
	}
	}
}

void ModuleRenderer3D::DrawEditor()
{
	if (ImGui::Checkbox((vsync) ? "VSync Enabled" : "VSync Disabled", &vsync))
		SetVSync(vsync);

	if (ImGui::Checkbox(shareLightPass ? "Not share light pass" : "Share Light pass", &shareLightPass)) {
		if (shareLightPass)
			RE_RES->At(RE_InternalResources::GetParticleLightPassShader())->UnloadMemory();
		else
			dynamic_cast<RE_Shader*>(RE_RES->At(RE_InternalResources::GetParticleLightPassShader()))->SetAsInternal(LIGHTPASSVERTEXSHADER, PARTICLELIGHTPASSFRAGMENTSHADER);
	}

	ImGui::Separator();

	if (shareLightPass)
	{
		ImGui::Text("Total Lights: %u:203", lightsCount + particlelightsCount);
		ImGui::Text("From lights components: %u", lightsCount);
		ImGui::Text("From particles: %u", particlelightsCount);
	}
	else
	{
		ImGui::Text("Lights components: %u:203", lightsCount);
		ImGui::Text("Particle Lights: %u:508", particlelightsCount);
	}

	ImGui::Separator();

	for (auto& view : render_views) view.DrawEditor();
}

void ModuleRenderer3D::Load()
{
	RE_PROFILE(RE_ProfiledFunc::Load, RE_ProfiledClass::ModuleRender);
	RE_LOG_SECONDARY("Loading Render3D config values:");
	RE_Json* node = RE_FS->ConfigNode("Renderer3D");

	SetVSync(node->PullBool("vsync", true));
	RE_LOG_TERCIARY((vsync) ? "VSync enabled." : "VSync disabled");

	if (shareLightPass = node->PullBool("share_light_pass", false))
		RE_RES->At(RE_InternalResources::GetParticleLightPassShader())->UnloadMemory();

	// Render Views
	for (uint i = 0; i < render_views.size(); ++i)
		render_views[i].Load(node->PullJObject((render_views[i].name + " View").c_str()));

	DEL(node)
}

void ModuleRenderer3D::Save() const
{
	RE_PROFILE(RE_ProfiledFunc::Save, RE_ProfiledClass::ModuleRender);
	RE_Json* node = RE_FS->ConfigNode("Renderer3D");
	node->Push("vsync", vsync);

	node->Push("share_light_pass", shareLightPass);

	// Render Views
	for (uint i = 0; i < render_views.size(); ++i)
		render_views[i].Save(node->PushJObject((render_views[i].name + " View").c_str()));

	DEL(node)
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

RenderView::LightMode ModuleRenderer3D::GetLightMode()
{
	return current_lighting;
}

RE_CompCamera* ModuleRenderer3D::GetCamera()
{
	return current_camera;
}

void ModuleRenderer3D::ChangeFBOSize(int width, int height, RenderView::Type view)
{
	fbos->ChangeFBOSize(render_views[static_cast<const ushort>(view)].GetFBO(), width, height);
}

uintptr_t  ModuleRenderer3D::GetRenderViewTexture(RenderView::Type type) const
{
	auto id = static_cast<ushort>(type);
	return fbos->GetTextureID(
		render_views[id].GetFBO(),
		render_views[id].light == RenderView::LightMode::DEFERRED ? 4u : 0u);
}

unsigned int ModuleRenderer3D::GetDepthTexture() const
{
	RE_GLCache::ChangeTextureBind(0);
	return fbos->GetDepthTexture(current_fbo);
}

void ModuleRenderer3D::PushSceneRend(RenderView& rV)
{
	render_queue.push({ RenderType::SCENE, rV, nullptr});
}

void ModuleRenderer3D::PushThumnailRend(const char* md5, bool redo)
{
	switch (RE_RES->At(md5)->GetType())
	{
	case ResourceContainer::Type::SCENE:
	case ResourceContainer::Type::MODEL:
	case ResourceContainer::Type::PREFAB:	render_queue.push({ RenderType::GO,		thumbnailView, md5, redo }); break;
	case ResourceContainer::Type::MATERIAL:	render_queue.push({ RenderType::MAT,	thumbnailView, md5, redo }); break;
	case ResourceContainer::Type::TEXTURE:	render_queue.push({ RenderType::TEX,	thumbnailView, md5, redo }); break;
	case ResourceContainer::Type::SKYBOX:	render_queue.push({ RenderType::SKYBOX, thumbnailView, md5, redo }); break;
	default: break;
	}
}

void ModuleRenderer3D::DrawScene(const RenderView& render_view)
{
	RE_PROFILE(RE_ProfiledFunc::DrawScene, RE_ProfiledClass::ModuleRender);

	// Setup Frame Buffer
	current_lighting = render_view.light;
	current_fbo = render_view.GetFBO();
	current_camera = render_view.camera;

	fbos->ChangeFBOBind(current_fbo, fbos->GetWidth(current_fbo), fbos->GetHeight(current_fbo));
	fbos->ClearFBOBuffers(current_fbo, render_view.clear_color.ptr());

	// Setup Render Flags
	bool usingClipDistance = render_view.HasFlag(RenderView::Flag::CLIP_DISTANCE);
	SetWireframe(render_view.HasFlag(RenderView::Flag::WIREFRAME));
	SetFaceCulling(render_view.HasFlag(RenderView::Flag::FACE_CULLING));
	SetTexture2D(false);
	SetColorMaterial(render_view.HasFlag(RenderView::Flag::COLOR_MATERIAL));
	SetDepthTest(render_view.HasFlag(RenderView::Flag::DEPTH_TEST));
	SetClipDistance(usingClipDistance);

	// Upload Shader Uniforms
	for (auto sMD5 : activeShaders)
		dynamic_cast<RE_Shader*>(RE_RES->At(sMD5))->UploadMainUniforms(
			render_view.camera,
			static_cast<float>(fbos->GetHeight(current_fbo)),
			static_cast<float>(fbos->GetWidth(current_fbo)),
			usingClipDistance,
			render_view.clip_distance);

	// Frustum Culling
	eastl::stack<RE_Component*> comptsToDraw;
	if (render_view.HasFlag(RenderView::Flag::FRUSTUM_CULLING))
	{
		eastl::vector<const RE_GameObject*> objects;
		RE_SCENE->FustrumCulling(objects, render_view.HasFlag(RenderView::Flag::OVERRIDE_CULLING) ?
			RE_SCENE->cams->GetCullingFrustum() : render_view.camera->GetFrustum());

		for (auto& go : objects)
		{
			if (go->HasFlag(RE_GameObject::Flag::ACTIVE))
			{
				RE_Component* render_geo = go->GetRenderGeo();
				if (render_geo) comptsToDraw.push(render_geo);
			}
		}
	}
	else comptsToDraw = RE_SCENE->GetScenePool()->GetRootPtr()->GetAllChildsActiveRenderGeos();

	// Setup Lights
	eastl::vector<RE_Component*> scene_lights;
	eastl::vector<RE_Component*> particleS_lights;

	switch (render_view.light)
	{
	case RenderView::LightMode::DISABLED:
	{
		SetLighting(false);
		break;
	}
	case RenderView::LightMode::GL:
	{
		SetLighting(true);
		scene_lights = RE_SCENE->GetScenePool()->GetAllCompPtr(RE_Component::Type::LIGHT);
		eastl::vector<RE_Component*> tmp = RE_SCENE->GetScenePool()->GetAllCompPtr(RE_Component::Type::PARTICLEEMITER);
		for(auto ps: tmp)
			if (dynamic_cast<RE_CompParticleEmitter*>(ps)->HasLight())
				particleS_lights.push_back(ps);
		// TODO RUB: Bind GL Lights

		break;
	}
	case RenderView::LightMode::DIRECT:
	{
		SetLighting(false);
		scene_lights = RE_SCENE->GetScenePool()->GetAllCompPtr(RE_Component::Type::LIGHT);
		eastl::vector<RE_Component*> tmp = RE_SCENE->GetScenePool()->GetAllCompPtr(RE_Component::Type::PARTICLEEMITER);
		for (auto ps : tmp)
			if (dynamic_cast<RE_CompParticleEmitter*>(ps)->HasLight())
				particleS_lights.push_back(ps);
		// TODO RUB: Upload Light uniforms

		break;
	}
	case RenderView::LightMode::DEFERRED:
	{
		SetLighting(false);
		scene_lights = RE_SCENE->GetScenePool()->GetAllCompPtr(RE_Component::Type::LIGHT);
		eastl::vector<RE_Component*> tmp = RE_SCENE->GetScenePool()->GetAllCompPtr(RE_Component::Type::PARTICLEEMITER);
		for (auto ps : tmp)
			if (dynamic_cast<RE_CompParticleEmitter*>(ps)->HasLight())
				particleS_lights.push_back(ps);
		break;
	}
	}

	// Draw Scene
	eastl::stack<RE_Component*> drawAsLast;
	eastl::stack<RE_Component*> particleSystems;
	RE_Component* drawing = nullptr;
	while (!comptsToDraw.empty())
	{
		drawing = comptsToDraw.top();
		comptsToDraw.pop();

		bool blend = false;
		RE_Component::Type dT = drawing->GetType();
		if (dT == RE_Component::Type::MESH)
			 blend = dynamic_cast<RE_CompMesh*>(drawing)->isBlend();

		if (dT == RE_Component::Type::PARTICLEEMITER) particleSystems.push(drawing);
		else if (!blend && dT != RE_Component::Type::WATER) drawing->Draw();
		else drawAsLast.push(drawing);
	}

	// Deferred Light Pass
	if (render_view.light == RenderView::LightMode::DEFERRED)
	{
		// Particle System Draws
		if (!particleSystems.empty())
		{
			while (!particleSystems.empty())
			{
				particleSystems.top()->Draw();
				particleSystems.pop();
			}
		}

		// Draw Blended elements
		if (!drawAsLast.empty())
		{
			while (!drawAsLast.empty())
			{
				drawAsLast.top()->Draw();
				drawAsLast.pop();
			}
		}

		// Setup Shader
		unsigned int light_pass = dynamic_cast<RE_Shader*>(RE_RES->At(RE_InternalResources::GetLightPassShader()))->GetID();
		RE_GLCache::ChangeShader(light_pass);

		SetDepthTest(false);

		glMemoryBarrierByRegion(GL_FRAMEBUFFER_BARRIER_BIT);

		if (!shareLightPass) 
		{

			// Bind Textures
			static const eastl::string deferred_textures[4] = { "gPosition", "gNormal", "gAlbedo", "gSpec" };
			for (unsigned int count = 0; count < 4; ++count)
			{
				glActiveTexture(GL_TEXTURE0 + count);
				RE_ShaderImporter::setInt(light_pass, deferred_textures[count].c_str(), count);
				RE_GLCache::ChangeTextureBind(fbos->GetTextureID(current_fbo, count));
			}

			// Setup Light Uniforms
			unsigned int count = 0;

			eastl::string unif_name;
			for (auto l : scene_lights)
			{
				unif_name = "lights[" + eastl::to_string(count) + "].";
				dynamic_cast<RE_CompLight*>(l)->CallShaderUniforms(light_pass, unif_name.c_str());
				count++;
				if (count == 203) break;
			}
			lightsCount = count;

			unif_name = "count";
			RE_ShaderImporter::setInt(RE_ShaderImporter::getLocation(light_pass, unif_name.c_str()), count);

			// Render Lights
			DrawQuad();

			// Render Particle Lights
			RE_PROFILE(RE_ProfiledFunc::DrawParticlesLight, RE_ProfiledClass::ModuleRender);
			if (!particleS_lights.empty())
			{
				// Setup Shader
				unsigned int particlelight_pass = dynamic_cast<RE_Shader*>(RE_RES->At(RE_InternalResources::GetParticleLightPassShader()))->GetID();
				RE_GLCache::ChangeShader(particlelight_pass);

				glMemoryBarrierByRegion(GL_FRAMEBUFFER_BARRIER_BIT);

				// Bind Textures
				static const eastl::string pdeferred_textures[5] = { "gPosition", "gNormal", "gAlbedo", "gSpec", "gLighting" };
				for (unsigned int count = 0; count < 5; ++count)
				{
					glActiveTexture(GL_TEXTURE0 + count);
					RE_ShaderImporter::setInt(particlelight_pass, pdeferred_textures[count].c_str(), count);
					RE_GLCache::ChangeTextureBind(fbos->GetTextureID(current_fbo, count));
				}

				unsigned int pCount = 0;
				for (auto pS : particleS_lights)
					dynamic_cast<RE_CompParticleEmitter*>(pS)->CallLightShaderUniforms(particlelight_pass, "plights", pCount, 508, shareLightPass);

				particlelightsCount = pCount;

				unif_name = "pInfo.pCount";
				RE_ShaderImporter::setInt(RE_ShaderImporter::getLocation(particlelight_pass, unif_name.c_str()), pCount);

				// Render Lights
				DrawQuad();
			}
		}
		else
		{
			RE_PROFILE(RE_ProfiledFunc::DrawParticlesLight, RE_ProfiledClass::ModuleRender);

			// Bind Textures
			static const eastl::string deferred_textures[4] = { "gPosition", "gNormal", "gAlbedo", "gSpec" };
			for (unsigned int count = 0; count < 4; ++count)
			{
				glActiveTexture(GL_TEXTURE0 + count);
				RE_ShaderImporter::setInt(light_pass, deferred_textures[count].c_str(), count);
				RE_GLCache::ChangeTextureBind(fbos->GetTextureID(current_fbo, count));
			}

			// Setup Light Uniforms
			unsigned int count = 0;

			eastl::string unif_name;
			for (auto l : scene_lights)
			{
				unif_name = "lights[" + eastl::to_string(count) + "].";
				dynamic_cast<RE_CompLight*>(l)->CallShaderUniforms(light_pass, unif_name.c_str());
				count++;
				if (count == 203) break;
			}
			lightsCount = count;

			unif_name = "lights";
			for (auto pS : particleS_lights)
				dynamic_cast<RE_CompParticleEmitter*>(pS)->CallLightShaderUniforms(light_pass, unif_name.c_str(), count, 203, shareLightPass);

			particlelightsCount = static_cast<uint>(math::Clamp(static_cast<int>(count) - static_cast<int>(lightsCount), 0, 203));

			unif_name = "count";
			RE_ShaderImporter::setInt(RE_ShaderImporter::getLocation(light_pass, unif_name.c_str()), count);

			// Render Lights
			DrawQuad();
		}

		if (render_view.HasFlag(RenderView::Flag::DEPTH_TEST))
			SetDepthTest(true);

		if (render_view.HasFlag(RenderView::Flag::DEBUG_DRAW))
			DrawDebug(render_view);

		if (render_view.HasFlag(RenderView::Flag::SKYBOX) && render_view.camera->isUsingSkybox())
			DrawSkyBox();
	}
	else
	{
		eastl::stack<RE_Component*> drawParticleSystemAsLast;

		// Particle System Draws
		while (!particleSystems.empty())
		{
			RE_Component* toDraw = particleSystems.top();
			if (dynamic_cast<RE_CompParticleEmitter*>(toDraw)->isBlend()) drawParticleSystemAsLast.push(toDraw);
			else toDraw->Draw();
			particleSystems.pop();
		}

		// Draw Debug Geometry
		if (render_view.HasFlag(RenderView::Flag::DEBUG_DRAW))
			DrawDebug(render_view);

		// Draw Skybox
		if (render_view.HasFlag(RenderView::Flag::SKYBOX) && render_view.camera->isUsingSkybox())
			DrawSkyBox();

		// Draw Blended elements
		if (!drawAsLast.empty() || !drawParticleSystemAsLast.empty())
		{
			if (render_view.HasFlag(RenderView::Flag::BLENDED))
			{
				glEnable(GL_BLEND);
				glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			}

			while (!drawAsLast.empty())
			{
				drawAsLast.top()->Draw();
				drawAsLast.pop();
			}

			while (!drawParticleSystemAsLast.empty())
			{
				drawParticleSystemAsLast.top()->Draw();
				drawParticleSystemAsLast.pop();
			}

			if (render_view.HasFlag(RenderView::Flag::BLENDED)) glDisable(GL_BLEND);
		}
	}

	// Draw Stencil
	if (render_view.HasFlag(RenderView::Flag::OUTLINE_SELECTION))
	{
		GO_UID stencilGO = RE_EDITOR->GetSelected();
		if (stencilGO)
		{
			RE_GameObject* stencilPtr = RE_SCENE->GetGOPtr(stencilGO);
			DrawStencil(stencilPtr, stencilPtr->GetRenderGeo(), render_view.HasFlag(RenderView::Flag::DEPTH_TEST));
		}
	}
}

void ModuleRenderer3D::DrawDebug(const RenderView& render_view)
{
	RE_GLCache::ChangeShader(0);
	RE_GLCache::ChangeTextureBind(0);

	bool reset_light = lighting;
	SetLighting(false);
	SetTexture2D(false);

	RE_PHYSICS->DrawDebug(render_view.camera);
	RE_EDITOR->DrawDebug(render_view.camera);

	if (reset_light) SetLighting(true);
	SetTexture2D(render_view.HasFlag(RenderView::Flag::TEXTURE_2D));
}

void ModuleRenderer3D::DrawParticleEditor(RenderView& render_view)
{
	// Setup Frame Buffer
	current_lighting = render_view.light;
	current_fbo = render_view.GetFBO();
	current_camera = render_view.camera;

	fbos->ChangeFBOBind(current_fbo, fbos->GetWidth(current_fbo), fbos->GetHeight(current_fbo));
	fbos->ClearFBOBuffers(current_fbo, render_view.clear_color.ptr());

	// Setup Render Flags
	SetWireframe(render_view.HasFlag(RenderView::Flag::WIREFRAME));
	SetFaceCulling(render_view.HasFlag(RenderView::Flag::FACE_CULLING));
	SetTexture2D(false);
	SetColorMaterial(render_view.HasFlag(RenderView::Flag::COLOR_MATERIAL));
	SetDepthTest(render_view.HasFlag(RenderView::Flag::DEPTH_TEST));

	bool usingClipDistance = render_view.HasFlag(RenderView::Flag::CLIP_DISTANCE);
	SetClipDistance(usingClipDistance);

	// Upload Shader Uniforms
	for (auto sMD5 : activeShaders)
		dynamic_cast<RE_Shader*>(RE_RES->At(sMD5))->UploadMainUniforms(
			render_view.camera,
			static_cast<float>(fbos->GetHeight(current_fbo)),
			static_cast<float>(fbos->GetWidth(current_fbo)),
			usingClipDistance,
			render_view.clip_distance);

	const RE_ParticleEmitter* editting_simulation = RE_EDITOR->GetCurrentEditingParticleEmitter();
	if (editting_simulation == nullptr) return;

	// Deferred Light Pass
	if (render_view.light == RenderView::LightMode::DEFERRED)
	{
		// Particle System Draws
		RE_PHYSICS->DrawParticleEmitterSimulation(editting_simulation->id, { 0.0,0.0,0.0 }, { 0.0,1.0,0.0 });

		if (editting_simulation->light.type != RE_PR_Light::Type::NONE)
			DrawParticleLights(editting_simulation->id);

		if (render_view.HasFlag(RenderView::Flag::DEPTH_TEST))
			SetDepthTest(true);

		// Draw Debug Geometry
		if (render_view.HasFlag(RenderView::Flag::DEBUG_DRAW))
		{
			RE_GLCache::ChangeShader(0);
			RE_GLCache::ChangeTextureBind(0);

			bool reset_light = lighting;
			SetLighting(false);
			SetTexture2D(false);

			glMatrixMode(GL_PROJECTION);
			glLoadMatrixf(render_view.camera->GetProjectionPtr());
			glMatrixMode(GL_MODELVIEW);
			glLoadMatrixf((render_view.camera->GetView()).ptr());

			RE_PHYSICS->DebugDrawParticleEmitterSimulation(editting_simulation);

			if (reset_light) SetLighting(true);
			SetTexture2D(render_view.HasFlag(RenderView::Flag::TEXTURE_2D));
		}
	}
	else
	{
		bool drawParticleSystemAsLast = false;
		// Particle System Draws
		if (editting_simulation->opacity.type != RE_PR_Opacity::Type::NONE)
			drawParticleSystemAsLast = true;
		else
			RE_PHYSICS->DrawParticleEmitterSimulation(editting_simulation->id, { 0.0,0.0,0.0 }, { 0.0,1.0,0.0 });

		// Draw Debug Geometry
		if (render_view.HasFlag(RenderView::Flag::DEBUG_DRAW))
		{
			RE_GLCache::ChangeShader(0);
			RE_GLCache::ChangeTextureBind(0);

			bool reset_light = lighting;
			SetLighting(false);
			SetTexture2D(false);

			glMatrixMode(GL_PROJECTION);
			glLoadMatrixf(render_view.camera->GetProjectionPtr());
			glMatrixMode(GL_MODELVIEW);
			glLoadMatrixf((render_view.camera->GetView()).ptr());

			RE_PHYSICS->DebugDrawParticleEmitterSimulation(editting_simulation);

			if (reset_light) SetLighting(true);
			SetTexture2D(render_view.HasFlag(RenderView::Flag::TEXTURE_2D));
		}

		// Draw Blended elements
		if (drawParticleSystemAsLast)
		{
			if (render_view.flags & static_cast<const ushort>(RenderView::Flag::BLENDED))
			{
				glEnable(GL_BLEND);
				glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			}

			RE_PHYSICS->DrawParticleEmitterSimulation(editting_simulation->id, { 0.0,0.0,0.0 }, { 0.0,1.0,0.0 });

			if (render_view.HasFlag(RenderView::Flag::BLENDED)) glDisable(GL_BLEND);
		}
	}
}

void ModuleRenderer3D::DrawParticleLights(const uint sim_id)
{
	if (!shareLightPass)
	{
		// Setup Shader
		unsigned int particlelight_pass = dynamic_cast<RE_Shader*>(RE_RES->At(RE_InternalResources::GetParticleLightPassShader()))->GetID();
		RE_GLCache::ChangeShader(particlelight_pass);

		glMemoryBarrierByRegion(GL_FRAMEBUFFER_BARRIER_BIT);

		// Bind Textures
		static const eastl::string pdeferred_textures[5] = { "gPosition", "gNormal", "gAlbedo", "gSpec", "gLighting" };
		for (unsigned int count = 0; count < 5; ++count)
		{
			glActiveTexture(GL_TEXTURE0 + count);
			RE_ShaderImporter::setInt(particlelight_pass, pdeferred_textures[count].c_str(), count);
			RE_GLCache::ChangeTextureBind(fbos->GetTextureID(current_fbo, count));
		}

		unsigned int pCount = 0;
		RE_PHYSICS->CallParticleEmitterLightShaderUniforms(sim_id, { 0.0,0.0,0.0 }, particlelight_pass, "plights", pCount, 508, shareLightPass);

		eastl::string unif_name = "pInfo.pCount";
		RE_ShaderImporter::setInt(RE_ShaderImporter::getLocation(particlelight_pass, unif_name.c_str()), pCount);

		// Render Lights
		DrawQuad();
	}
	else
	{
		// Setup Shader
		unsigned int light_pass = dynamic_cast<RE_Shader*>(RE_RES->At(RE_InternalResources::GetLightPassShader()))->GetID();
		RE_GLCache::ChangeShader(light_pass);

		SetDepthTest(false);

		glMemoryBarrierByRegion(GL_FRAMEBUFFER_BARRIER_BIT);

		// Bind Textures
		static const eastl::string deferred_textures[4] = { "gPosition", "gNormal", "gAlbedo", "gSpec" };
		for (unsigned int count = 0; count < 4; ++count)
		{
			glActiveTexture(GL_TEXTURE0 + count);
			RE_ShaderImporter::setInt(light_pass, deferred_textures[count].c_str(), count);
			RE_GLCache::ChangeTextureBind(fbos->GetTextureID(current_fbo, count));
		}

		// Setup Light Uniforms
		unsigned int count = 0;
		RE_PHYSICS->CallParticleEmitterLightShaderUniforms(sim_id, { 0.0,0.0,0.0 }, light_pass, "lights", count, 203, shareLightPass);

		RE_ShaderImporter::setInt(RE_ShaderImporter::getLocation(light_pass, "count"), count);

		// Render Lights
		DrawQuad();
	}
}

void ModuleRenderer3D::DrawSkyBox()
{
	RE_PROFILE(RE_ProfiledFunc::DrawSkybox, RE_ProfiledClass::ModuleRender);
	RE_GLCache::ChangeTextureBind(0);

	uint skysphereshader = dynamic_cast<RE_Shader*>(RE_RES->At(RE_InternalResources::GetDefaultSkyBoxShader()))->GetID();
	RE_GLCache::ChangeShader(skysphereshader);
	RE_ShaderImporter::setInt(skysphereshader, "cubemap", 0);
	RE_ShaderImporter::setBool(skysphereshader, "deferred", (current_lighting == RenderView::LightMode::DEFERRED));

	glDepthFunc(GL_LEQUAL);
	RE_CameraManager::MainCamera()->DrawSkybox();
	glDepthFunc(GL_LESS); // set depth function back to default
}

void ModuleRenderer3D::DrawStencil(RE_GameObject* go, RE_Component* comp, bool has_depth_test)
{
	if (!comp) return;

	RE_PROFILE(RE_ProfiledFunc::DrawStencil, RE_ProfiledClass::ModuleRender);
	unsigned int vaoToStencil = 0;
	GLsizei triangleToStencil = 0;

	RE_Component::Type cT = comp->GetType();
	if (cT == RE_Component::Type::MESH)
	{
		RE_CompMesh* mesh_comp = dynamic_cast<RE_CompMesh*>(comp);
		vaoToStencil = mesh_comp->GetVAOMesh();
		triangleToStencil = static_cast<GLsizei>(mesh_comp->GetTriangleMesh());
	}
	else if (cT > RE_Component::Type::PRIMIVE_MIN && cT < RE_Component::Type::PRIMIVE_MAX)
	{
		RE_CompPrimitive* prim_comp = dynamic_cast<RE_CompPrimitive*>(comp);
		vaoToStencil = prim_comp->GetVAO();
		triangleToStencil = static_cast<GLsizei>(prim_comp->GetTriangleCount());
	}
	else if (cT == RE_Component::Type::WATER)
	{
		RE_CompWater* water_comp = dynamic_cast<RE_CompWater*>(comp);
		vaoToStencil = water_comp->GetVAO();
		triangleToStencil = static_cast<GLsizei>(water_comp->GetTriangles());
	}

	glEnable(GL_STENCIL_TEST);
	SetDepthTest(false);

	//Getting the scale shader and setting some values
	const char* scaleShader = RE_InternalResources::GetDefaultScaleShader();
	RE_Shader* sShader = dynamic_cast<RE_Shader*>(RE_RES->At(scaleShader));
	unsigned int shaderiD = sShader->GetID();
	RE_GLCache::ChangeShader(shaderiD);
	RE_GLCache::ChangeVAO(vaoToStencil);
	sShader->UploadModel(go->GetTransformPtr()->GetGlobalMatrixPtr());
	RE_ShaderImporter::setFloat(shaderiD, "useColor", 1.0);
	RE_ShaderImporter::setFloat(shaderiD, "useTexture", 0.0);
	RE_ShaderImporter::setFloat(shaderiD, "cdiffuse", { 1.0, 0.5, 0.0 });
	RE_ShaderImporter::setFloat(shaderiD, "opacity", 1.0f);
	RE_ShaderImporter::setFloat(shaderiD, "center", go->GetLocalBoundingBox().CenterPoint());

	//Prepare stencil for detect
	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE); //don't draw to color buffer
	glStencilFunc(GL_ALWAYS, 1, 0xFF); //mark to 1 where pass
	glStencilMask(0xFF);
	glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);

	//Draw scaled mesh 
	RE_ShaderImporter::setFloat(shaderiD, "scaleFactor", 0.5f / go->GetTransformPtr()->GetLocalScale().Length());

	glDrawElements(GL_TRIANGLES, triangleToStencil * 3, GL_UNSIGNED_INT, nullptr);

	glStencilFunc(GL_ALWAYS, 0, 0x00);//change stencil to draw 0
	//Draw normal mesh for empty the inside of stencil
	RE_ShaderImporter::setFloat(shaderiD, "scaleFactor", 0.0);
	glDrawElements(GL_TRIANGLES, triangleToStencil * 3, GL_UNSIGNED_INT, nullptr);

	//Turn on the draw and only draw where stencil buffer marks 1
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE); // Make sure we draw on the backbuffer again.
	glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP); // Make sure you will no longer (over)write stencil values, even if any test succeeds
	glStencilFunc(GL_EQUAL, 1, 0xFF); // Now we will only draw pixels where the corresponding stencil buffer value equals 1

	//Draw scaled mesh 
	RE_ShaderImporter::setFloat(shaderiD, "scaleFactor", 0.5f / go->GetTransformPtr()->GetLocalScale().Length());
	glDrawElements(GL_TRIANGLES, triangleToStencil * 3, GL_UNSIGNED_INT, nullptr);

	RE_GLCache::ChangeShader(0);

	glDisable(GL_STENCIL_TEST);
	if (has_depth_test) SetDepthTest(true);
}

void ModuleRenderer3D::ThumbnailGameObject(RE_GameObject* go)
{
	unsigned int c_fbo = thumbnailView.fbos.first;
	fbos->ChangeFBOBind(c_fbo, fbos->GetWidth(c_fbo), fbos->GetHeight(c_fbo));
	fbos->ClearFBOBuffers(c_fbo, thumbnailView.clear_color.ptr());

	go->ResetGOandChildsAABB();

	RE_CompCamera* internalCamera = thumbnailView.camera;

	internalCamera->SetFOV(math::RadToDeg(0.523599f));
	internalCamera->Update();
	internalCamera->GetTransform()->SetPosition({ 0.0f,1.0f,5.0f });
	internalCamera->Update();
	internalCamera->GetTransform()->SetRotation({ math::DegToRad(45.0f),0.0f,0.0f});
	internalCamera->Update();
	math::AABB box = go->GetGlobalBoundingBoxWithChilds();
	internalCamera->Focus(box.CenterPoint(), box.HalfSize().Length());
	internalCamera->Update();
	
	current_camera = internalCamera;

	for (auto sMD5 : activeShaders)
	{
		RE_Shader* shader = dynamic_cast<RE_Shader*>(RE_RES->At(sMD5));
		if (!shader->uniforms.empty())
			shader->UploadMainUniforms(internalCamera, THUMBNAILSIZE, THUMBNAILSIZE, false, {});
	}

	go->DrawChilds();
}

void ModuleRenderer3D::ThumbnailMaterial(RE_Material* mat)
{
	unsigned int c_fbo = thumbnailView.fbos.first;
	fbos->ChangeFBOBind(c_fbo, fbos->GetWidth(c_fbo), fbos->GetHeight(c_fbo));
	fbos->ClearFBOBuffers(c_fbo, thumbnailView.clear_color.ptr());

	RE_CompCamera* internalCamera = thumbnailView.camera;

	RE_INPUT->PauseEvents();
	internalCamera->SetFOV(math::RadToDeg(0.523599f));
	internalCamera->Update();
	internalCamera->GetTransform()->SetRotation({ 0.0f,0.0f, 0.0f });
	internalCamera->Update();
	//internalCamera->GetTransform()->SetPosition({ 0.0f, 0.0f, 0.0f});
	//internalCamera->Update();
	internalCamera->GetTransform()->SetPosition({ 0.0f ,0.0f, 5.0f });
	internalCamera->Update();
	RE_INPUT->ResumeEvents();

	for (auto sMD5 : activeShaders)
	{
		RE_Shader* shader = dynamic_cast<RE_Shader*>(RE_RES->At(sMD5));
		if (!shader->uniforms.empty())
			shader->UploadMainUniforms(internalCamera, THUMBNAILSIZE, THUMBNAILSIZE, false, {});
	}

	mat->UploadToShader(math::float4x4::identity.ptr(), false, true);

	RE_GLCache::ChangeVAO(mat_vao);
	glDrawElements(GL_TRIANGLES, mat_triangles * 3, GL_UNSIGNED_SHORT, 0);
	RE_GLCache::ChangeVAO(0);
	RE_GLCache::ChangeShader(0);
	RE_GLCache::ChangeTextureBind(0);
}

void ModuleRenderer3D::ThumbnailSkyBox(RE_SkyBox* skybox)
{
	unsigned int c_fbo = thumbnailView.fbos.first;
	fbos->ChangeFBOBind(c_fbo, fbos->GetWidth(c_fbo), fbos->GetHeight(c_fbo));
	fbos->ClearFBOBuffers(c_fbo, thumbnailView.clear_color.ptr());

	RE_CompCamera* internalCamera = thumbnailView.camera;

	RE_INPUT->PauseEvents();
	internalCamera->ForceFOV(125, 140);
	internalCamera->GetTransform()->SetRotation({ 0.0,0.0,0.0 });
	internalCamera->GetTransform()->SetPosition(math::vec(0.f, 0.f, 0.f));
	internalCamera->Update();
	RE_INPUT->ResumeEvents();

	for (auto sMD5 : activeShaders)
	{
		RE_Shader* shader = dynamic_cast<RE_Shader*>(RE_RES->At(sMD5));
		if (!shader->uniforms.empty())
			shader->UploadMainUniforms(internalCamera, THUMBNAILSIZE, THUMBNAILSIZE, false, {});
	}

	RE_GLCache::ChangeTextureBind(0);

	RE_Shader* skyboxShader = (RE_Shader*)RE_RES->At(RE_InternalResources::GetDefaultSkyBoxShader());
	uint skysphereshader = skyboxShader->GetID();
	RE_GLCache::ChangeShader(skysphereshader);
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
	RE_GLCache::ChangeVAO(quadVAO);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	RE_GLCache::ChangeVAO(0);
	RE_GLCache::ChangeShader(0);
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

math::float4 ModuleRenderer3D::GetRenderViewClearColor(RenderView::Type r_view) const
{
	return (r_view < RenderView::Type::OTHER) ? render_views[static_cast<const short>(r_view)].clear_color : math::float4(0.0f, 0.0f, 0.0f, 1.0f);
}

RenderView::LightMode ModuleRenderer3D::GetRenderViewLightMode(RenderView::Type r_view) const
{
	return (r_view < RenderView::Type::OTHER) ? render_views[static_cast<const short>(r_view)].light : RenderView::LightMode::DISABLED;
}

bool ModuleRenderer3D::GetRenderViewDebugDraw(RenderView::Type r_view) const
{
	return (r_view < RenderView::Type::OTHER) ? render_views[static_cast<const short>(r_view)].HasFlag(RenderView::Flag::DEBUG_DRAW) : false;
}

void ModuleRenderer3D::SetRenderViewDeferred(RenderView::Type r_view, bool using_deferred)
{
	if (r_view < RenderView::Type::OTHER) render_views[static_cast<const short>(r_view)].light =
		using_deferred ? RenderView::LightMode::DEFERRED : RenderView::LightMode::DISABLED;
}

void ModuleRenderer3D::SetRenderViewClearColor(RenderView::Type r_view, math::float4 clear_color)
{
	if (r_view < RenderView::Type::OTHER) render_views[static_cast<const short>(r_view)].clear_color = clear_color;
}

void ModuleRenderer3D::SetRenderViewDebugDraw(RenderView::Type r_view, bool debug_draw)
{
	if (r_view < RenderView::Type::OTHER)
	{
		if (debug_draw) render_views[static_cast<const short>(r_view)].AddFlag(RenderView::Flag::DEBUG_DRAW);
		else render_views[static_cast<const short>(r_view)].RemoveFlag(RenderView::Flag::DEBUG_DRAW);
	}
}