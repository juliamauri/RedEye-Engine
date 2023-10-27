#include "ModuleRenderer3D.h"

#include "RE_Memory.h"
#include "RE_Profiler.h"
#include "RE_ConsoleLog.h"
#include "RE_Math.h"
#include "Event.h"
#include "Application.h"
#include "ModuleWindow.h"
#include "ModuleInput.h"
#include "ModuleEditor.h"
#include "ModuleScene.h"
#include "ModulePhysics.h"

#include "RE_FileSystem.h"
#include "RE_FileBuffer.h"
#include "RE_Config.h"
#include "RE_Json.h"

#include "RE_GLCache.h"
#include "RE_FBOManager.h"
#include "RE_ResourceManager.h"
#include "RE_InternalResources.h"
#include "RE_DefaultShaders.h"
#include "RE_ShaderImporter.h"
#include "RE_ThumbnailManager.h"
#include "RE_CameraManager.h"
#include "RE_PrimitiveManager.h"
#include "RE_ParticleManager.h"

#include "RenderedWindow.h"
#include "ParticleEmitterEditorWindow.h"

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

#include <MGL/Math/float3.h>
#include <MGL/Math/float4.h>
#include <MGL/Math/Quat.h>
#include <ImGuiWidgets/ImGuizmo/ImGuizmo.h>
#include <SDL2/SDL.h>
#include <GL/glew.h>
#include <gl/GL.h>
#include <EASTL/list.h>
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
	switch (severity)
	{
	case GL_DEBUG_SEVERITY_LOW: severityStr += "low"; break;
	case GL_DEBUG_SEVERITY_MEDIUM: severityStr += "medium"; break;
	case GL_DEBUG_SEVERITY_HIGH: severityStr += "high"; break;
	default: severityStr += "not specified."; break;
	}

	eastl::string typeStr = "Type ";
	switch (type)
	{
	case GL_DEBUG_TYPE_ERROR: typeStr += "GL ERROR"; break;
	case GL_DEBUG_TYPE_PORTABILITY: typeStr += "GL PORTABILITY"; break;
	case GL_DEBUG_TYPE_PERFORMANCE: typeStr += "GL PERFORMANCE"; break;
	case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: typeStr += "GL DEPRECATED"; break;
	default: typeStr += "GL UNDEFINED"; break;
	}

	if (type == GL_DEBUG_TYPE_ERROR) RE_LOG_ERROR("%s, %s, message = %s\n", (typeStr.c_str()), severityStr.c_str(), message);
	else RE_LOG_WARNING("%s, %s, message = %s\n", (typeStr.c_str()), severityStr.c_str(), message);
}

// Static values
void* ModuleRenderer3D::mainContext = nullptr;
ushort ModuleRenderer3D::flags = 0;
RenderView::LightMode ModuleRenderer3D::current_lighting = RenderView::LightMode::GL;
uint ModuleRenderer3D::current_fbo = 0;
eastl::vector<const char*> ModuleRenderer3D::activeShaders;

uint ModuleRenderer3D::lightsCount = 0;
uint ModuleRenderer3D::particlelightsCount = 0;

bool ModuleRenderer3D::Init()
{
	RE_LOG("Initializing Module Renderer3D");
	RE_PROFILE(RE_ProfiledFunc::Init, RE_ProfiledClass::ModuleRender)

	if (!InitializeGL())
	{
		RE_LOG_ERROR("Error Initializing GL");
		return false;
	}

	// Precompute debug circle for faster draws
	PrecomputeCircle();

	// Setup Empty Render Flags
	SetupFlags(0, true);
	Load();

	return true;
}

void ModuleRenderer3D::PostUpdate()
{
	RE_PROFILE(RE_ProfiledFunc::PostUpdate, RE_ProfiledClass::ModuleRender)
	
	PullActiveShaders();
	RE_ThumbnailManager::DrawThumbnails();

	// Render Window FBO's
	RE_EDITOR->RenderWindowFBOs();

	// Set Render to Window
	RE_FBOManager::ChangeFBOBind(0, RE_WINDOW->GetWidth(), RE_WINDOW->GetHeight());

	// Draw Editor
	RemoveFlag(RenderView::Flag::WIREFRAME);
	RE_EDITOR->DrawEditorWindows();

	//Swap buffers
	SDL_GL_SwapWindow(RE_WINDOW->GetWindow());
}

void ModuleRenderer3D::CleanUp()
{
	RE_PROFILE(RE_ProfiledFunc::CleanUp, RE_ProfiledClass::ModuleRender)
	RE_FBOManager::ClearAll();
	SDL_GL_DeleteContext(mainContext);
}

void ModuleRenderer3D::RecieveEvent(const Event & e)
{
	switch (e.type)
	{
	case RE_EventType::SET_WIRE_FRAME: SetupFlag(RenderView::Flag::WIREFRAME, e.data1.AsBool()); break;
	case RE_EventType::SET_FACE_CULLING: SetupFlag(RenderView::Flag::FACE_CULLING, e.data1.AsBool()); break;
	case RE_EventType::SET_TEXTURE_TWO_D: SetupFlag(RenderView::Flag::TEXTURE_2D, e.data1.AsBool()); break;
	case RE_EventType::SET_COLOR_MATERIAL: SetupFlag(RenderView::Flag::COLOR_MATERIAL, e.data1.AsBool()); break;
	case RE_EventType::SET_DEPTH_TEST: SetupFlag(RenderView::Flag::DEPTH_TEST, e.data1.AsBool()); break;
	case RE_EventType::SET_CLIP_DISTANCE: SetupFlag(RenderView::Flag::CLIP_DISTANCE, e.data1.AsBool()); break;
	case RE_EventType::SET_LIGHTNING: SetupFlag(RenderView::Flag::GL_LIGHT, e.data1.AsBool()); break;
	case RE_EventType::SET_VSYNC: SetupFlag(RenderView::Flag::VSYNC, e.data1.AsBool()); break;
	default: RE_LOG("Unused Event at Module Render"); break;
	}
}

void ModuleRenderer3D::DrawEditor()
{
	// Vertical Sync
	bool vsync = HasFlag(RenderView::Flag::VSYNC);
	if (ImGui::Checkbox(vsync ? "VSync Enabled" : "VSync Disabled", &vsync))
		SetupFlag(RenderView::Flag::VSYNC, vsync);

	// Shared Light Pass
	bool share_light_pass = HasFlag(RenderView::Flag::SHARE_LIGHT_PASS);
	if (ImGui::Checkbox(share_light_pass ? "Disable light pass" : "Share Light pass", &share_light_pass))
		SetupFlag(RenderView::Flag::SHARE_LIGHT_PASS, share_light_pass);

	ImGui::Separator();

	if (share_light_pass)
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

	// Debug Drawing
	ImGui::Separator();

	ImGui::DragFloat("Point Size", &point_size, 1.f, 0.f, 100.f);

	auto steps = static_cast<int>(circle_precompute.capacity());
	if (ImGui::DragInt("Circle Steps", &steps, 1.f, 0, 64))
		PrecomputeCircle(steps);
}

#pragma region Config Serialization

void ModuleRenderer3D::Load()
{
	RE_LOG_SECONDARY("Loading Render3D config values:");
	RE_PROFILE(RE_ProfiledFunc::Load, RE_ProfiledClass::ModuleRender)

	RE_Json* node = RE_FS->ConfigNode("Renderer3D");
	SetupFlag(RenderView::Flag::VSYNC, node->PullBool("vsync", true));
	SetupFlag(RenderView::Flag::SHARE_LIGHT_PASS, node->PullBool("share_light_pass", false));

	// Debug circle
	PrecomputeCircle(node->PullInt("CircleSteps", 12));

	DEL(node)
}

void ModuleRenderer3D::Save() const
{
	RE_PROFILE(RE_ProfiledFunc::Save, RE_ProfiledClass::ModuleRender)

	RE_Json* node = RE_FS->ConfigNode("Renderer3D");
	node->Push("vsync", HasFlag(RenderView::Flag::VSYNC));
	node->Push("share_light_pass", HasFlag(RenderView::Flag::SHARE_LIGHT_PASS));

	// Debug circle
	node->Push("CircleSteps", circle_precompute.size());

	DEL(node)
}

#pragma endregion

#pragma region Actions

void ModuleRenderer3D::LoadCameraMatrixes(const RE_Camera& camera)
{
	glMatrixMode(GL_PROJECTION);
	glLoadMatrixf(camera.GetProjection().Transposed().ptr());
	glMatrixMode(GL_MODELVIEW);
	glLoadMatrixf(camera.GetView().Transposed().ptr());
}

void ModuleRenderer3D::DrawScene(
	const RenderView& render_view,
	const RE_Camera& camera,
	eastl::stack<const RE_Component*>& drawables,
	eastl::vector<const RE_Component*> lights,
	eastl::vector<const RE_CompParticleEmitter*> particle_lights,
	RenderedWindow* toDebug) const
{
	RE_PROFILE(RE_ProfiledFunc::DrawScene, RE_ProfiledClass::ModuleRender)

	// Prep Renderer
	PrepareToRender(render_view, camera);

	// Get & Categorize Drawn Elements
	eastl::stack<const RE_Component*> geo;
	eastl::stack<const RE_Component*> blended_geo;
	eastl::stack<const RE_CompParticleEmitter*> particle_systems;
	eastl::stack<const RE_CompParticleEmitter*> blended_particle_systems;
	CategorizeDrawables(drawables, geo, blended_geo, particle_systems, blended_particle_systems);

	// Draw Geos
	while (!geo.empty())
	{
		geo.top()->Draw();
		geo.pop();
	}

	// Draw Particle Systems
	while (!particle_systems.empty())
	{
		particle_systems.top()->Draw();
		particle_systems.pop();
	}

	// Draw Elements based on LightMode
	switch (render_view.light_mode)
	{
	case RenderView::LightMode::DEFERRED:

		DrawSceneDeferred(
			render_view, camera,
			blended_geo, blended_particle_systems,
			lights, particle_lights);

		if (toDebug && render_view.HasFlag(RenderView::Flag::DEBUG_DRAW))
			DrawDebug(toDebug, render_view, camera);

		if (render_view.HasFlag(RenderView::Flag::SKYBOX) && camera.isUsingSkybox())
		{
			const char* skybox_md5 = camera.GetSkybox();
			auto skybox = dynamic_cast<const RE_SkyBox*>(RE_RES->At(skybox_md5 ? skybox_md5 : RE_InternalResources::GetDefaultSkyBox()));
			DrawSkyBox(skybox);
		}

		break;
	default:

		// Draw Debug Geometry
		if (toDebug && render_view.HasFlag(RenderView::Flag::DEBUG_DRAW))
			DrawDebug(toDebug, render_view, camera);

		// Draw Skybox
		if (render_view.HasFlag(RenderView::Flag::SKYBOX) && camera.isUsingSkybox())
		{
			const char* skybox_md5 = camera.GetSkybox();
			auto skybox = RE_RES->At(skybox_md5 ? skybox_md5 : RE_InternalResources::GetDefaultSkyBox());
			DrawSkyBox(skybox->As<const RE_SkyBox*>());
		}

		// Draw Blended elements
		if (!blended_geo.empty() || !blended_particle_systems.empty())
		{
			if (render_view.HasFlag(RenderView::Flag::BLENDED))
			{
				AddFlag(RenderView::Flag::BLENDED);
				glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			}

			while (!blended_geo.empty())
			{
				blended_geo.top()->Draw();
				blended_geo.pop();
			}

			while (!blended_particle_systems.empty())
			{
				blended_particle_systems.top()->Draw();
				blended_particle_systems.pop();
			}

			if (render_view.HasFlag(RenderView::Flag::BLENDED))
				RemoveFlag(RenderView::Flag::BLENDED);
		}

		break;
	}
}

void ModuleRenderer3D::DrawStencil(GO_UID stencilGO, bool has_depth_test) const
{
	if (!stencilGO) return;

	RE_GameObject* go = RE_SCENE->GetGOPtr(stencilGO);
	RE_Component* comp = go->GetRenderGeo();
	if (comp == nullptr) return;

	RE_PROFILE(RE_ProfiledFunc::DrawStencil, RE_ProfiledClass::ModuleRender)
		unsigned int vaoToStencil = 0;
	GLsizei triangleToStencil = 0;

	RE_Component::Type cT = comp->GetType();

	switch (cT)
	{
	case RE_Component::Type::MESH:
	{
		RE_CompMesh* mesh_comp = dynamic_cast<RE_CompMesh*>(comp);
		vaoToStencil = mesh_comp->GetVAOMesh();
		triangleToStencil = static_cast<GLsizei>(mesh_comp->GetTriangleMesh());
		break;
	}
	case RE_Component::Type::WATER:
	{
		RE_CompWater* water_comp = dynamic_cast<RE_CompWater*>(comp);
		vaoToStencil = water_comp->GetVAO();
		triangleToStencil = static_cast<GLsizei>(water_comp->GetTriangles());
		break;
	}
	default:

		if (cT > RE_Component::Type::PRIMIVE_MIN && cT < RE_Component::Type::PRIMIVE_MAX)
		{
			RE_CompPrimitive* prim_comp = dynamic_cast<RE_CompPrimitive*>(comp);
			vaoToStencil = prim_comp->GetVAO();
			triangleToStencil = static_cast<GLsizei>(prim_comp->GetTriangleCount());
		}
		break;
	}

	glEnable(GL_STENCIL_TEST);
	RemoveFlag(RenderView::Flag::DEPTH_TEST);

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
	if (has_depth_test) AddFlag(RenderView::Flag::DEPTH_TEST);
}

#pragma endregion

#pragma region Getters

uint ModuleRenderer3D::GetDepthTexture()
{
	RE_GLCache::ChangeTextureBind(0);
	return RE_FBOManager::GetDepthTexture(current_fbo);
}

const char* ModuleRenderer3D::GetGPURenderer() { return reinterpret_cast<const char*>(glGetString(GL_RENDERER)); }
const char* ModuleRenderer3D::GetGPUVendor() { return reinterpret_cast<const char*>(glGetString(GL_VENDOR)); }
unsigned int ModuleRenderer3D::GetMaxVertexAttributes()
{
	int nrAttributes;
	glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &nrAttributes);
	return nrAttributes;
}

#pragma endregion

bool ModuleRenderer3D::InitializeGL()
{
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

	RE_LOG_SECONDARY("Creating SDL GL Context");
	mainContext = SDL_GL_CreateContext(RE_WINDOW->GetWindow());
	if (!mainContext)
	{
		RE_LOG_ERROR("SDL could not create GL Context! SDL_Error: %s", SDL_GetError());
		return false;
	}
	RE_SOFT_NVS("OpenGL", reinterpret_cast<const char*>(glGetString(GL_VERSION)), "https://www.opengl.org/");

	RE_LOG_SECONDARY("Initializing Glew");
	GLenum error = glewInit();
	if (error != GLEW_OK)
	{
		RE_LOG_ERROR("Glew could not initialize! Glew_Error: %s", glewGetErrorString(error));
		return false;
	}
	RE_SOFT_NVS("Glew", reinterpret_cast<const char*>(glewGetString(GLEW_VERSION)), "http://glew.sourceforge.net/");

	int test;
	glGetIntegerv(GL_MAX_VERTEX_UNIFORM_BLOCKS, &test);
	RE_LOG_TERCIARY("GL Variable lookup: GL_MAX_VERTEX_UNIFORM_BLOCKS = %i", test);
	glGetIntegerv(GL_MAX_GEOMETRY_UNIFORM_BLOCKS, &test);
	RE_LOG_TERCIARY("GL Variable lookup: GL_MAX_GEOMETRY_UNIFORM_BLOCKS = %i", test);
	glGetIntegerv(GL_MAX_FRAGMENT_UNIFORM_BLOCKS, &test);
	RE_LOG_TERCIARY("GL Variable lookup: GL_MAX_FRAGMENT_UNIFORM_BLOCKS = %i", test);
	glGetIntegerv(GL_MAX_UNIFORM_BLOCK_SIZE, &test);
	RE_LOG_TERCIARY("GL Variable lookup: GL_MAX_UNIFORM_BLOCK_SIZE = %i", test);
	glGetIntegerv(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT, &test);
	RE_LOG_TERCIARY("GL Variable lookup: GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT = %i", test);
	glGetIntegerv(GL_MAX_FRAGMENT_UNIFORM_COMPONENTS, &test);
	RE_LOG_TERCIARY("GL Variable lookup: GL_MAX_FRAGMENT_UNIFORM_COMPONENTS = %i", test);
	glGetIntegerv(GL_MAX_VERTEX_UNIFORM_COMPONENTS, &test);
	RE_LOG_TERCIARY("GL Variable lookup: GL_MAX_VERTEX_UNIFORM_COMPONENTS = %i", test);
	glGetIntegerv(GL_MAX_GEOMETRY_UNIFORM_COMPONENTS, &test);
	RE_LOG_TERCIARY("GL Variable lookup: GL_MAX_GEOMETRY_UNIFORM_COMPONENTS = %i", test);

	//OpenGL Debug Output
	glEnable(GL_DEBUG_OUTPUT);
	glDebugMessageCallback(MessageCallback, 0);

	return true;
}

void ModuleRenderer3D::PrecomputeCircle(int steps)
{
	if (steps == circle_precompute.capacity())
		return;

	circle_precompute.clear();
	circle_precompute.set_capacity(steps);
	auto interval = RE_Math::pi_x2 / steps;
	for (float i = 0.f; i < RE_Math::pi_x2; i += interval)
		circle_precompute.push_back({ math::Sin(i), -math::Cos(i) });
}

#pragma region Render Flags

bool ModuleRenderer3D::HasFlag(RenderView::Flag flag)
{
	return flags & static_cast<ushort>(flag);
}

void ModuleRenderer3D::AddFlag(RenderView::Flag flag, bool force_state)
{
	if (!force_state && HasFlag(flag)) return;

	flags |= static_cast<ushort>(flag);

	switch (flag)
	{
		// GL Specs
	case RenderView::Flag::WIREFRAME: glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); break;
	case RenderView::Flag::FACE_CULLING: glEnable(GL_CULL_FACE); break;
	case RenderView::Flag::TEXTURE_2D: glEnable(GL_TEXTURE_2D); break;
	case RenderView::Flag::COLOR_MATERIAL: glEnable(GL_COLOR_MATERIAL); break;
	case RenderView::Flag::DEPTH_TEST: glEnable(GL_DEPTH_TEST); break;
	case RenderView::Flag::CLIP_DISTANCE: glEnable(GL_CLIP_DISTANCE0); break;
	case RenderView::Flag::GL_LIGHT: glEnable(GL_LIGHTING); break;

		// RE Utility
	case RenderView::Flag::FRUSTUM_CULLING: ; break;
	case RenderView::Flag::OVERRIDE_CULLING: ; break;
	case RenderView::Flag::OUTLINE_SELECTION: ; break;
	case RenderView::Flag::DEBUG_DRAW: ; break;
	case RenderView::Flag::SKYBOX: ; break;
	case RenderView::Flag::BLENDED: ; break;

		// Renderer
	case RenderView::Flag::VSYNC:
		SDL_GL_SetSwapInterval(1);
		RE_LOG_TERCIARY("VSync enabled.");
		break;
	case RenderView::Flag::SHARE_LIGHT_PASS:
		RE_RES->At(RE_InternalResources::GetParticleLightPassShader())->UnloadMemory();
		break;
	default: break;
	}
}

void ModuleRenderer3D::RemoveFlag(RenderView::Flag flag, bool force_state)
{
	if (!force_state && !HasFlag(flag)) return;
	
	auto f = static_cast<ushort>(flag);
	if (flags & f) flags -= f;

	switch (flag)
	{
		// GL Specs
	case RenderView::Flag::WIREFRAME: glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); break;
	case RenderView::Flag::FACE_CULLING: glDisable(GL_CULL_FACE); break;
	case RenderView::Flag::TEXTURE_2D: glDisable(GL_TEXTURE_2D); break;
	case RenderView::Flag::COLOR_MATERIAL: glDisable(GL_COLOR_MATERIAL); break;
	case RenderView::Flag::DEPTH_TEST: glDisable(GL_DEPTH_TEST); break;
	case RenderView::Flag::CLIP_DISTANCE: glDisable(GL_CLIP_DISTANCE0); break;
	case RenderView::Flag::GL_LIGHT: glDisable(GL_LIGHTING); break;

		// Renderer
	case RenderView::Flag::VSYNC:
		SDL_GL_SetSwapInterval(0);
		RE_LOG_TERCIARY("VSync disabled.");
		break;
	case RenderView::Flag::SHARE_LIGHT_PASS:
		dynamic_cast<RE_Shader*>(RE_RES->At(RE_InternalResources::GetParticleLightPassShader()))->SetAsInternal(LIGHTPASSVERTEXSHADER, PARTICLELIGHTPASSFRAGMENTSHADER);
		break;
	default: break;
	}
}

void ModuleRenderer3D::SetupFlag(RenderView::Flag flag, bool target_state, bool force_state)
{
	if (target_state) AddFlag(flag, force_state);
	else RemoveFlag(flag, force_state);
}

void ModuleRenderer3D::SetupFlags(ushort _flags, bool force_state)
{
	for (int i = 0; i < 15; i++)
	{
		ushort flag = 1 << 0;
		SetupFlag(static_cast<RenderView::Flag>(flag), _flags & flag, force_state);

		if (_flags & flag) AddFlag(static_cast<RenderView::Flag>(flag), force_state);
		else RemoveFlag(static_cast<RenderView::Flag>(flag), force_state);
	}

	flags = _flags;
}

#pragma endregion

void ModuleRenderer3D::PullActiveShaders() const
{
	RE_PROFILE(RE_ProfiledFunc::GetActiveShaders, RE_ProfiledClass::ModuleRender)
	activeShaders = RE_RES->GetAllResourcesActiveByType(ResourceContainer::Type::SHADER);
}

void ModuleRenderer3D::PrepareToRender(const RenderView& render_view, const RE_Camera& camera) const
{
	// Setup Frame Buffer
	current_lighting = render_view.light_mode;
	current_fbo = render_view.GetFBO();
	//current_camera = camera;

	RE_FBOManager::ChangeFBOBind(current_fbo, RE_FBOManager::GetWidth(current_fbo), RE_FBOManager::GetHeight(current_fbo));
	RE_FBOManager::ClearFBOBuffers(current_fbo, render_view.clear_color.ptr());

	// Setup Render Flags
	RemoveFlag(RenderView::Flag::TEXTURE_2D);
	SetupFlag(RenderView::Flag::WIREFRAME, render_view.HasFlag(RenderView::Flag::WIREFRAME));
	SetupFlag(RenderView::Flag::FACE_CULLING, render_view.HasFlag(RenderView::Flag::FACE_CULLING));
	SetupFlag(RenderView::Flag::COLOR_MATERIAL, render_view.HasFlag(RenderView::Flag::COLOR_MATERIAL));
	SetupFlag(RenderView::Flag::DEPTH_TEST, render_view.HasFlag(RenderView::Flag::DEPTH_TEST));

	bool usingClipDistance = render_view.HasFlag(RenderView::Flag::CLIP_DISTANCE);
	SetupFlag(RenderView::Flag::CLIP_DISTANCE, usingClipDistance);

	// Upload Shader Uniforms
	for (auto sMD5 : activeShaders)
		dynamic_cast<RE_Shader*>(RE_RES->At(sMD5))->UploadMainUniforms(
			camera,
			static_cast<float>(RE_FBOManager::GetHeight(current_fbo)),
			static_cast<float>(RE_FBOManager::GetWidth(current_fbo)),
			usingClipDistance,
			render_view.clip_distance);

	// Setup Lights
	/* switch (render_view.light) {
	case RenderView::LightMode::GL: break; // TODO RUB: Bind GL Lights
	case RenderView::LightMode::DIRECT: break; // TODO RUB: Upload Light uniforms
	default: break; }*/
	bool using_gl_lights = render_view.light_mode == RenderView::LightMode::GL;
	SetupFlag(RenderView::Flag::GL_LIGHT, using_gl_lights);
}

void ModuleRenderer3D::CategorizeDrawables(
	eastl::stack<const RE_Component*>& drawables,
	eastl::stack<const RE_Component*>& geo,
	eastl::stack<const RE_Component*>& blended_geo,
	eastl::stack<const RE_CompParticleEmitter*>& particle_systems,
	eastl::stack<const RE_CompParticleEmitter*>& blended_particle_systems) const
{
	while (!drawables.empty())
	{
		auto drawable = drawables.top();
		drawables.pop();

		bool blend = false;
		RE_Component::Type dT = drawable->GetType();

		switch (dT)
		{
		default: geo.push(drawable); break;
		case RE_Component::Type::MESH:
			if (drawable->As<const RE_CompMesh*>()->HasBlend()) blended_geo.push(drawable);
			else geo.push(drawable);
			break;
		case RE_Component::Type::WATER: blended_geo.push(drawable); break;
		case RE_Component::Type::PARTICLEEMITER:
			auto emitter = drawable->As<const RE_CompParticleEmitter*>();
			if (emitter->HasBlend()) blended_particle_systems.push(emitter);
			else particle_systems.push(emitter);
			break;
		}
	}
}

#pragma region Draws

#pragma region Main Draws

void ModuleRenderer3D::DrawSceneDeferred(
	const RenderView& render_view,
	const RE_Camera& camera,
	eastl::stack<const RE_Component*>& blended_geo,
	eastl::stack<const RE_CompParticleEmitter*>& blended_particle_systems,
	eastl::vector<const RE_Component*> lights,
	eastl::vector<const RE_CompParticleEmitter*> particle_lights) const
{
	// Setup Shader
	uint light_pass = dynamic_cast<RE_Shader*>(RE_RES->At(RE_InternalResources::GetLightPassShader()))->GetID();
	RE_GLCache::ChangeShader(light_pass);

	RemoveFlag(RenderView::Flag::DEPTH_TEST);

	glMemoryBarrierByRegion(GL_FRAMEBUFFER_BARRIER_BIT);
	
	// Bind Textures
	static const eastl::string texture_uniform_names[5] = { "gPosition", "gNormal", "gAlbedo", "gSpec", "gLighting" };
	for (uint count = 0; count < 4; count++)
	{
		glActiveTexture(GL_TEXTURE0 + count);
		RE_ShaderImporter::setInt(light_pass, texture_uniform_names[count].c_str(), count);
		RE_GLCache::ChangeTextureBind(RE_FBOManager::GetTextureID(current_fbo, count));
	}

	// Setup Light Uniforms
	unsigned int count = 0;
	for (auto light : lights)
	{
		light->As<const RE_CompLight*>()->CallShaderUniforms(
			light_pass,
			(eastl::string("lights[") + eastl::to_string(count) + "].").c_str());

		if (++count == 203) break;
	}
	lightsCount = count;

	if (!HasFlag(RenderView::Flag::SHARE_LIGHT_PASS))
	{
		RE_ShaderImporter::setInt(RE_ShaderImporter::getLocation(light_pass, "count"), count);

		// Render Lights
		DrawQuad();

		// Render Particle Lights
		RE_PROFILE(RE_ProfiledFunc::DrawParticlesLight, RE_ProfiledClass::ModuleRender)
		if (!particle_lights.empty())
		{
			// Setup Shader
			uint particlelight_pass = dynamic_cast<RE_Shader*>(RE_RES->At(RE_InternalResources::GetParticleLightPassShader()))->GetID();
			RE_GLCache::ChangeShader(particlelight_pass);

			glMemoryBarrierByRegion(GL_FRAMEBUFFER_BARRIER_BIT);

			// Bind Textures
			for (uint count = 0; count < 5; ++count)
			{
				glActiveTexture(GL_TEXTURE0 + count);
				RE_ShaderImporter::setInt(particlelight_pass, texture_uniform_names[count].c_str(), count);
				RE_GLCache::ChangeTextureBind(RE_FBOManager::GetTextureID(current_fbo, count));
			}

			uint pCount = 0;
			for (auto comp_emitter : particle_lights)
				RE_ParticleManager::CallLightShaderUniforms(
					comp_emitter->GetSimulationID(),
					comp_emitter->GetGlobalPosition(),
					particlelight_pass,
					"plights",
					pCount,
					508,
					HasFlag(RenderView::Flag::SHARE_LIGHT_PASS));

			particlelightsCount = pCount;
			RE_ShaderImporter::setInt(RE_ShaderImporter::getLocation(particlelight_pass, "pInfo.pCount"), pCount);

			// Render Lights
			DrawQuad();
		}
	}
	else
	{
		RE_PROFILE(RE_ProfiledFunc::DrawParticlesLight, RE_ProfiledClass::ModuleRender)

		for (auto comp_emitter : particle_lights)
			RE_ParticleManager::CallLightShaderUniforms(
				comp_emitter->GetSimulationID(),
				comp_emitter->GetGlobalPosition(),
				light_pass,
				"lights",
				count,
				203,
				HasFlag(RenderView::Flag::SHARE_LIGHT_PASS));

		particlelightsCount = static_cast<uint>(math::Clamp(static_cast<int>(count) - static_cast<int>(lightsCount), 0, 203));
		RE_ShaderImporter::setInt(RE_ShaderImporter::getLocation(light_pass, "count"), count);

		// Render Lights
		DrawQuad();
	}

	SetupFlag(RenderView::Flag::DEPTH_TEST, render_view.HasFlag(RenderView::Flag::DEPTH_TEST));
}

void ModuleRenderer3D::DrawDebug(
	const RenderedWindow* toDebug,
	const RenderView& render_view,
	const RE_Camera& camera) const
{
	RE_GLCache::ChangeShader(0);
	RE_GLCache::ChangeTextureBind(0);

	bool prev_light = HasFlag(RenderView::Flag::GL_LIGHT);
	RemoveFlag(RenderView::Flag::GL_LIGHT);
	RemoveFlag(RenderView::Flag::TEXTURE_2D);

	toDebug->DrawDebug();

	if (prev_light) AddFlag(RenderView::Flag::GL_LIGHT);
	SetupFlag(RenderView::Flag::TEXTURE_2D, render_view.HasFlag(RenderView::Flag::TEXTURE_2D));
}

void ModuleRenderer3D::DrawSkyBox(const RE_SkyBox* skybox) const
{
	if (skybox == nullptr) return;

	RE_PROFILE(RE_ProfiledFunc::DrawSkybox, RE_ProfiledClass::ModuleRender)
	RE_GLCache::ChangeTextureBind(0);

	uint skysphereshader = dynamic_cast<RE_Shader*>(RE_RES->At(RE_InternalResources::GetDefaultSkyBoxShader()))->GetID();
	RE_GLCache::ChangeShader(skysphereshader);
	RE_ShaderImporter::setInt(skysphereshader, "cubemap", 0);
	RE_ShaderImporter::setBool(skysphereshader, "deferred", (current_lighting == RenderView::LightMode::DEFERRED));

	glDepthFunc(GL_LEQUAL);
	skybox->DrawSkybox();
	glDepthFunc(GL_LESS); // set depth function back to default
}

#pragma endregion

#pragma region Direct Draws

void ModuleRenderer3D::DrawQuad() const
{
	// Setup Screen Quad
	static uint quadVAO = 0;
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
		uint quadVBO;
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

void ModuleRenderer3D::DirectDrawCube(math::vec position, math::vec color) const
{
	glColor3f(color.x, color.y, color.z);

	math::float4x4 model = math::float4x4::Translate(position.Neg());
	model.InverseTranspose();

	glMatrixMode(GL_MODELVIEW);
	glLoadMatrixf((RE_CameraManager::MainCamera()->Camera.GetView().Transposed() * model).ptr());

	glBegin(GL_TRIANGLES);
	glVertex3f(-1.0f, -1.0f, -1.0f);
	glVertex3f(1.0f, -1.0f, -1.0f);
	glVertex3f(1.0f, 1.0f, -1.0f);
	glVertex3f(1.0f, 1.0f, -1.0f);
	glVertex3f(-1.0f, 1.0f, -1.0f);
	glVertex3f(-1.0f, -1.0f, -1.0f);

	glVertex3f(-1.0f, -1.0f, 1.0f);
	glVertex3f(1.0f, -1.0f, 1.0f);
	glVertex3f(1.0f, 1.0f, 1.0f);
	glVertex3f(1.0f, 1.0f, 1.0f);
	glVertex3f(-1.0f, 1.0f, 1.0f);
	glVertex3f(-1.0f, -1.0f, 1.0f);

	glVertex3f(-1.0f, 1.0f, 1.0f);
	glVertex3f(-1.0f, 1.0f, -1.0f);
	glVertex3f(-1.0f, -1.0f, -1.0f);
	glVertex3f(-1.0f, -1.0f, -1.0f);
	glVertex3f(-1.0f, -1.0f, 1.0f);
	glVertex3f(-1.0f, 1.0f, 1.0f);

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
	glVertex3f(-1.0f, -1.0f, -1.0f);

	glVertex3f(-1.0f, 1.0f, -1.0f);
	glVertex3f(1.0f, 1.0f, -1.0f);
	glVertex3f(1.0f, 1.0f, 1.0f);
	glVertex3f(1.0f, 1.0f, 1.0f);
	glVertex3f(-1.0f, 1.0f, 1.0f);
	glVertex3f(-1.0f, 1.0f, -1.0f);
	glEnd();
}

void ModuleRenderer3D::DrawAASphere(const math::vec p_pos, const float radius) const
{
	eastl::vector<math::float2>::const_iterator i = circle_precompute.cbegin() + 1;
	math::vec previous = { p_pos.x + (circle_precompute.cbegin()->x * radius), p_pos.y + (circle_precompute.cbegin()->y * radius), p_pos.z };
	for (; i != circle_precompute.cend(); ++i) // Z
	{
		glVertex3fv(previous.ptr());
		previous = { p_pos.x + (i->x * radius), p_pos.y + (i->y * radius), p_pos.z };
		glVertex3fv(previous.ptr());
	}
	previous = { p_pos.x + (circle_precompute.cbegin()->x * radius), p_pos.y, p_pos.z + (circle_precompute.cbegin()->y * radius) };
	for (i = circle_precompute.cbegin() + 1; i != circle_precompute.cend(); ++i) // Y
	{
		glVertex3fv(previous.ptr());
		previous = { p_pos.x + (i->x * radius), p_pos.y, p_pos.z + (i->y * radius) };
		glVertex3fv(previous.ptr());
	}
	previous = { p_pos.x, p_pos.y + (circle_precompute.cbegin()->y * radius), p_pos.z + (circle_precompute.cbegin()->x * radius) };
	for (i = circle_precompute.cbegin() + 1; i != circle_precompute.cend(); ++i) // X
	{
		glVertex3fv(previous.ptr());
		previous = { p_pos.x, p_pos.y + (i->y * radius), p_pos.z + (i->x * radius) };
		glVertex3fv(previous.ptr());
	}
}

#pragma endregion

#pragma region Particle Editor Draws

void ModuleRenderer3D::DrawParticleEditor(RenderView& render_view, const RE_Camera& camera) const
{
	auto emitter_id = RE_EDITOR->GetParticleEmitterEditorWindow()->GetEdittingParticleEmitter();
	auto emitter = RE_ParticleManager::GetEmitter(emitter_id);
	if (!emitter) return;

	PrepareToRender(render_view, camera);

	switch (render_view.light_mode)
	{
	case RenderView::LightMode::DEFERRED:

		RE_ParticleManager::DrawSimulation(emitter_id, camera);
		if (emitter->HasLight()) DrawParticleLights(emitter_id);

		SetupFlag(RenderView::Flag::DEPTH_TEST, render_view.HasFlag(RenderView::Flag::DEPTH_TEST));

		DrawParticleEditorDebug(render_view, emitter, camera);

		break;

	default:
	{
		bool draw_last = emitter->opacity.HasOpacity();
		if (!draw_last) RE_ParticleManager::DrawSimulation(emitter_id, camera);

		DrawParticleEditorDebug(render_view, emitter, camera);

		if (draw_last) // Draw Blended elements
		{
			if (render_view.HasFlag(RenderView::Flag::BLENDED))
			{
				AddFlag(RenderView::Flag::BLENDED);
				glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			}

			RE_ParticleManager::DrawSimulation(emitter_id, camera);

			SetupFlag(RenderView::Flag::DEPTH_TEST, render_view.HasFlag(RenderView::Flag::DEPTH_TEST));
			if (render_view.HasFlag(RenderView::Flag::BLENDED)) RemoveFlag(RenderView::Flag::BLENDED);
		}

		break;
	}
	}
}

void ModuleRenderer3D::DrawParticleEditorDebug(
	const RenderView& render_view,
	const RE_ParticleEmitter* emitter,
	const RE_Camera& camera) const
{
	if (!render_view.HasFlag(RenderView::Flag::DEBUG_DRAW))
		return;

	RE_GLCache::ChangeShader(0);
	RE_GLCache::ChangeTextureBind(0);

	bool reset_light = render_view.HasFlag(RenderView::Flag::GL_LIGHT);
	RemoveFlag(RenderView::Flag::GL_LIGHT);
	RemoveFlag(RenderView::Flag::TEXTURE_2D);

	LoadCameraMatrixes(camera);
	DebugDrawParticleEmitter(*emitter);

	if (reset_light) AddFlag(RenderView::Flag::GL_LIGHT);
	SetupFlag(RenderView::Flag::TEXTURE_2D, render_view.HasFlag(RenderView::Flag::TEXTURE_2D));
}

void ModuleRenderer3D::DrawParticleLights(P_UID sim_id) const
{
	static const eastl::string deferred_textures[5] = { "gPosition", "gNormal", "gAlbedo", "gSpec", "gLighting" };
	bool shared_light_pass = HasFlag(RenderView::Flag::SHARE_LIGHT_PASS);
	
	uint shader_pass;
	uint texture_count;
	eastl::string shader_pass_name;
	eastl::string count_var;
	uint max_lights;

	if (!shared_light_pass)
	{
		shader_pass = dynamic_cast<RE_Shader*>(RE_RES->At(RE_InternalResources::GetParticleLightPassShader()))->GetID();
		texture_count = 5;
		shader_pass_name = "plights";
		count_var = "pInfo.pCount";
		max_lights = 508;
	}
	else
	{
		shader_pass = dynamic_cast<RE_Shader*>(RE_RES->At(RE_InternalResources::GetLightPassShader()))->GetID();
		texture_count = 4;
		shader_pass_name = "lights";
		count_var = "count";
		max_lights = 203;
		RemoveFlag(RenderView::Flag::DEPTH_TEST);
	}

	// Setup Shader
	RE_GLCache::ChangeShader(shader_pass);
	glMemoryBarrierByRegion(GL_FRAMEBUFFER_BARRIER_BIT);

	// Bind Textures
	for (uint count = 0; count < texture_count; ++count)
	{
		glActiveTexture(GL_TEXTURE0 + count);
		RE_ShaderImporter::setInt(shader_pass, deferred_textures[count].c_str(), count);
		RE_GLCache::ChangeTextureBind(RE_FBOManager::GetTextureID(current_fbo, count));
	}

	// Setup Light Uniforms
	uint count = 0;
	RE_ParticleManager::CallLightShaderUniforms(
		sim_id,
		{ 0.0,0.0,0.0 },
		shader_pass,
		shader_pass_name.c_str(),
		count,
		max_lights,
		shared_light_pass);

	RE_ShaderImporter::setInt(RE_ShaderImporter::getLocation(shader_pass, count_var.c_str()), count);

	// Render Lights
	DrawQuad();
}

void ModuleRenderer3D::DebugDrawParticleEmitter(const RE_ParticleEmitter& sim) const
{
	if (sim.initial_pos.IsShaped() || sim.boundary.HasBoundary())
	{
		glBegin(GL_LINES);

		// Render Spawn Shape
		glColor4f(1.0f, 0.27f, 0.f, 1.f); // orange
		switch (sim.initial_pos.type) {
		case RE_EmissionShape::Type::CIRCLE:
		{
			math::Circle c = sim.initial_pos.geo.circle;
			c.pos += sim.parent_pos;
			math::vec previous = c.GetPoint(0.f);
			auto interval = static_cast<float>(circle_precompute.size());
			for (float i = interval; i < RE_Math::pi_x2; i += interval)
			{
				glVertex3f(previous.x, previous.y, previous.z);
				previous = c.GetPoint(i);
				glVertex3f(previous.x, previous.y, previous.z);
			}
			break;
		}
		case RE_EmissionShape::Type::RING:
		{
			math::Circle c = sim.initial_pos.geo.ring.first;
			c.pos += sim.parent_pos;
			c.r += sim.initial_pos.geo.ring.second;
			math::vec previous = c.GetPoint(0.f);
			auto interval = static_cast<float>(circle_precompute.size());
			for (float i = interval; i < RE_Math::pi_x2; i += interval)
			{
				glVertex3f(previous.x, previous.y, previous.z);
				previous = c.GetPoint(i);
				glVertex3f(previous.x, previous.y, previous.z);
			}

			c.r -= 2.f * sim.initial_pos.geo.ring.second;
			previous = c.GetPoint(0.f);
			for (float i = interval; i < RE_Math::pi_x2; i += interval)
			{
				glVertex3f(previous.x, previous.y, previous.z);
				previous = c.GetPoint(i);
				glVertex3f(previous.x, previous.y, previous.z);
			}
			break;
		}
		case RE_EmissionShape::Type::AABB:
		{
			for (int i = 0; i < 12; i++)
			{
				glVertex3fv((sim.initial_pos.geo.box.Edge(i).a + sim.parent_pos).ptr());
				glVertex3fv((sim.initial_pos.geo.box.Edge(i).b + sim.parent_pos).ptr());
			}

			break;
		}
		case RE_EmissionShape::Type::SPHERE:
		{
			DrawAASphere(sim.parent_pos + sim.initial_pos.geo.sphere.pos, sim.initial_pos.geo.sphere.r);
			break;
		}
		case RE_EmissionShape::Type::HOLLOW_SPHERE:
		{
			DrawAASphere(sim.parent_pos + sim.initial_pos.geo.hollow_sphere.first.pos, sim.initial_pos.geo.hollow_sphere.first.r - sim.initial_pos.geo.hollow_sphere.second);
			DrawAASphere(sim.parent_pos + sim.initial_pos.geo.hollow_sphere.first.pos, sim.initial_pos.geo.hollow_sphere.first.r + sim.initial_pos.geo.hollow_sphere.second);
			break;
		}
		default: break;
		}

		// Render Boundary
		glColor4f(1.f, 0.84f, 0.0f, 1.f); // gold
		switch (sim.boundary.type) {
		case RE_EmissionBoundary::Type::PLANE:
		{
			const float interval = RE_Math::pi_x2 / circle_precompute.size();
			for (float j = 1.f; j < 6.f; ++j)
			{
				const math::Circle c = sim.boundary.geo.plane.GenerateCircle(sim.parent_pos, j * j);
				math::vec previous = c.GetPoint(0.f);
				for (float i = interval; i < RE_Math::pi_x2; i += interval)
				{
					glVertex3f(previous.x, previous.y, previous.z);
					previous = c.GetPoint(i);
					glVertex3f(previous.x, previous.y, previous.z);
				}
			}

			break;
		}
		case RE_EmissionBoundary::Type::SPHERE:
		{
			DrawAASphere(sim.parent_pos + sim.boundary.geo.sphere.pos, sim.boundary.geo.sphere.r);
			break;
		}
		case RE_EmissionBoundary::Type::AABB:
		{
			for (int i = 0; i < 12; i++)
			{
				glVertex3fv((sim.parent_pos + sim.boundary.geo.box.Edge(i).a).ptr());
				glVertex3fv((sim.parent_pos + sim.boundary.geo.box.Edge(i).b).ptr());
			}

			break;
		}
		default: break;
		}

		// Render Shape Collider
		if (sim.collider.type == RE_EmissionCollider::Type::SPHERE)
		{
			glColor4f(0.1f, 0.8f, 0.1f, 1.f); // light green
			for (const auto& p : sim.particle_pool)
				DrawAASphere(sim.local_space ? sim.parent_pos + p.position : p.position, p.col_radius);
		}

		glEnd();
	}

	// Render Point Collider
	if (sim.collider.type == RE_EmissionCollider::Type::POINT)
	{
		glPointSize(point_size);
		glBegin(GL_POINTS);
		glColor4f(0.1f, 0.8f, 0.1f, 1.f); // light green

		if (sim.local_space)
			for (const auto& p : sim.particle_pool)
				glVertex3fv((sim.parent_pos + p.position).ptr());
		else
			for (const auto& p : sim.particle_pool)
				glVertex3fv(p.position.ptr());

		glPointSize(1.f);
		glEnd();
	}
}

#pragma endregion

#pragma endregion
