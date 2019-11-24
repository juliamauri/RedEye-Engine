#include "ModuleRenderer3D.h"

#include "SDL2/include/SDL.h"
#include "Glew/include/glew.h"
#include <gl/GL.h>

#include "OutputLog.h"
#include "Application.h"
#include "ModuleWindow.h"
#include "ModuleEditor.h"
#include "TimeManager.h"
#include "ModuleInput.h"
#include "RE_FileSystem.h"
#include "RE_CompCamera.h"
#include "RE_CameraManager.h"
#include "RE_CompTransform.h"
#include "ModuleScene.h"
#include "RE_ShaderImporter.h"
#include "RE_InternalResources.h"

#pragma comment(lib, "Glew/lib/glew32.lib")
#pragma comment(lib, "opengl32.lib")

ModuleRenderer3D::ModuleRenderer3D(const char * name, bool start_enabled) : Module(name, start_enabled)
{}

ModuleRenderer3D::~ModuleRenderer3D()
{}

bool ModuleRenderer3D::Init(JSONNode * node)
{
	bool ret = false;
	LOG_SECONDARY("Seting SDL/GL Attributes.");
	if (SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE) < 0)
		LOG_ERROR("SDL could not set GL Attributes: 'SDL_GL_CONTEXT_PROFILE_MASK: SDL_GL_CONTEXT_PROFILE_CORE'");
	if (SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1) < 0)
		LOG_ERROR("SDL could not set GL Attributes: 'SDL_GL_DOUBLEBUFFER: 1'");
	if (SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24) < 0)
		LOG_ERROR("SDL could not set GL Attributes: 'SDL_GL_DEPTH_SIZE: 24'");
	if (SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8) < 0)
		LOG_ERROR("SDL could not set GL Attributes: 'SDL_GL_STENCIL_SIZE: 8'");
	if (SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3) < 0)
		LOG_ERROR("SDL could not set GL Attributes: 'SDL_GL_CONTEXT_MAJOR_VERSION: 3'");
	if (SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1) < 0)
		LOG_ERROR("SDL could not set GL Attributes: 'SDL_GL_CONTEXT_MINOR_VERSION: 1'");
	
	if (App->window)
	{
		LOG_SECONDARY("Creating SDL GL Context");
		mainContext = SDL_GL_CreateContext(App->window->GetWindow());
		if (ret = (mainContext != nullptr))
			App->ReportSoftware("OpenGL", (char*)glGetString(GL_VERSION), "https://www.opengl.org/");
		else
			LOG_ERROR("SDL could not create GL Context! SDL_Error: %s", SDL_GetError());
	}

	if (ret)
	{
		LOG_SECONDARY("Initializing Glew");
		GLenum error = glewInit();
		if (ret = (error == GLEW_OK))
		{
			Load(node);
			App->ReportSoftware("Glew", (char*)glewGetString(GLEW_VERSION), "http://glew.sourceforge.net/");
		}
		else
		{
			LOG_ERROR("Glew could not initialize! Glew_Error: %s", glewGetErrorString(error));
		}
	}

	return ret;
}

bool ModuleRenderer3D::Start()
{
	sceneShader = App->internalResources->GetDefaultShader();
	skyboxShader = App->internalResources->GetSkyBoxShader();

	return true;
}

update_status ModuleRenderer3D::PreUpdate()
{
	OPTICK_CATEGORY("PreUpdate Renderer3D", Optick::Category::GameLogic);

	update_status ret = UPDATE_CONTINUE;

	// Reset background with a clear color
	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	return ret;
}

update_status ModuleRenderer3D::PostUpdate()
{
	OPTICK_CATEGORY("PostUpdate Renderer3D", Optick::Category::GameLogic);

	update_status ret = UPDATE_CONTINUE;

	OPTICK_CATEGORY("Culling", Optick::Category::Rendering);
	std::vector<const RE_GameObject*> objects;
	if (cull_scene)
	{
		math::Frustum frustum = App->cams->GetCullingFrustum();
		App->scene->FustrumCulling(objects, frustum);
	}

	OPTICK_CATEGORY("Scene Draw", Optick::Category::Rendering);

	// Prepare if using wireframe
	if(wireframe)
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	RE_CompCamera* current_camera = RE_CameraManager::CurrentCamera();
	current_camera->Update();

	// Load Shader Uniforms
	RE_ShaderImporter::use(sceneShader);
	RE_ShaderImporter::setFloat4x4(sceneShader, "view", current_camera->GetViewPtr());
	RE_ShaderImporter::setFloat4x4(sceneShader, "projection", current_camera->GetProjectionPtr());

	// Frustum Culling
	if (cull_scene)
		for (auto object : objects)
			object->DrawItselfOnly();
	else
		App->scene->GetRoot()->DrawWithChilds();

	// Draw Debug Geometry
	App->editor->DrawDebug(lighting);

	OPTICK_CATEGORY("SkyBox Draw", Optick::Category::Rendering);
	// draw skybox as last

	// Set shader and uniforms
	RE_ShaderImporter::use(skyboxShader);
	RE_ShaderImporter::setFloat4x4(skyboxShader, "view", current_camera->GetViewPtr());
	RE_ShaderImporter::setFloat4x4(skyboxShader, "projection", current_camera->GetProjectionPtr());
	RE_ShaderImporter::setInt(skyboxShader, "skybox", 0);

	// change depth function so depth test passes when values are equal to depth buffer's content
	glDepthFunc(GL_LEQUAL);
	
	// Render skybox cube
	glBindVertexArray(App->internalResources->GetSkyBoxVAO());
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, App->internalResources->GetSkyBoxTexturesID());
	glDrawArrays(GL_TRIANGLES, 0, 36);
	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
	glDepthFunc(GL_LESS); // set depth function back to default

	// Draw Editor
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	App->editor->Draw();

	//Swap buffers
	SDL_GL_SwapWindow(App->window->GetWindow());

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
	switch (e.GetType())
	{
	case WINDOW_SIZE_CHANGED:
	{
		WindowSizeChanged(e.GetData().AsInt(), e.GetDataNext().AsInt());
		break;
	}
	case SET_VSYNC:
	{
		SetVSync(e.GetData().AsBool());
		break;
	}
	case SET_DEPTH_TEST:
	{
		SetDepthTest(e.GetData().AsBool());
		break;
	}
	case SET_FACE_CULLING:
	{
		SetFaceCulling(e.GetData().AsBool());
		break;
	}
	case SET_LIGHTNING:
	{
		SetLighting(e.GetData().AsBool());
		break;
	}
	case SET_TEXTURE_TWO_D:
	{
		SetTexture2D(e.GetData().AsBool());
		break;
	}
	case SET_COLOR_MATERIAL:
	{
		SetColorMaterial(e.GetData().AsBool());
		break;
	}
	case SET_WIRE_FRAME:
	{
		SetWireframe(e.GetData().AsBool());
		break;
	}
	case CURRENT_CAM_VIEWPORT_CHANGED:
	{
		UpdateViewPort(e.GetData().AsInt(), e.GetDataNext().AsInt());
		break;
	}
	}
}

void ModuleRenderer3D::DrawEditor()
{
	if(ImGui::CollapsingHeader("Renderer 3D"))
	{
		if (ImGui::Checkbox((vsync) ? "Disable VSync" : "Enable VSync", &vsync))
			SetVSync(vsync);

		if (ImGui::Checkbox((cullface) ? "Disable Cull Face" : "Enable Cull Face", &cullface))
			SetDepthTest(cullface);

		if (ImGui::Checkbox((depthtest) ? "Disable Depht Test" : "Enable Depht Test", &depthtest))
			SetFaceCulling(depthtest);

		if (ImGui::Checkbox((lighting) ? "Disable Lighting" : "Enable Lighting", &lighting))
			SetLighting(lighting);

		if (ImGui::Checkbox((texture2d) ? "Disable Texture2D" : "Enable Texture2D", &texture2d))
			SetTexture2D(texture2d);

		if (ImGui::Checkbox((color_material) ? "Disable Color Material" : "Enable Color Material", &color_material))
			SetColorMaterial(color_material);

		if (ImGui::Checkbox((wireframe) ? "Disable Wireframe" : "Enable Wireframe", &wireframe))
			SetWireframe(wireframe);

		ImGui::Checkbox("Camera Frustum Culling", &cull_scene);
	}
}

bool ModuleRenderer3D::Load(JSONNode * node)
{
	bool ret = (node != nullptr);
	LOG_SECONDARY("Loading Render3D config values:");

	if (ret)
	{
		SetVSync(node->PullBool("vsync", vsync));
		LOG_TERCIARY((vsync)? "VSync enabled." : "VSync disabled");
		SetFaceCulling(node->PullBool("cullface", cullface));
		LOG_TERCIARY((cullface)? "CullFace enabled." : "CullFace disabled");
		SetDepthTest(node->PullBool("depthtest", depthtest));
		LOG_TERCIARY((depthtest)? "DepthTest enabled." : "DepthTest disabled");
		SetLighting(node->PullBool("lighting", lighting));
		LOG_TERCIARY((lighting)? "Lighting enabled." : "Lighting disabled");
		SetTexture2D(node->PullBool("texture 2d", texture2d));
		LOG_TERCIARY((texture2d)? "Textures enabled." : "Textures disabled");
		SetColorMaterial(node->PullBool("color material", color_material));
		LOG_TERCIARY((color_material)? "Color Material enabled." : "Color Material disabled");
		SetWireframe(node->PullBool("wireframe", wireframe));
		LOG_TERCIARY((wireframe)? "Wireframe enabled." : "Wireframe disabled");
	}

	return ret;
}

bool ModuleRenderer3D::Save(JSONNode * node) const
{
	bool ret = (node != nullptr);

	if (ret)
	{
		node->PushBool("vsync", vsync);
		node->PushBool("cullface", cullface);
		node->PushBool("depthtest", depthtest);
		node->PushBool("lighting", lighting);
		node->PushBool("texture 2d", texture2d);
		node->PushBool("color material", color_material);
		node->PushBool("wireframe", wireframe);
	}

	return ret;
}

void ModuleRenderer3D::SetVSync(bool enable)
{
	SDL_GL_SetSwapInterval((vsync = enable) ? 1 : 0);
}

void ModuleRenderer3D::SetDepthTest(bool enable)
{
	(cullface = enable) ? glEnable(GL_DEPTH_TEST) : glDisable(GL_DEPTH_TEST);
}

void ModuleRenderer3D::SetFaceCulling(bool enable)
{
	(depthtest = enable) ? glEnable(GL_CULL_FACE) : glDisable(GL_CULL_FACE);
}

void ModuleRenderer3D::SetLighting(bool enable)
{
	(lighting = enable) ? glEnable(GL_LIGHTING) : glDisable(GL_LIGHTING);
}

void ModuleRenderer3D::SetTexture2D(bool enable)
{
	(texture2d = enable) ? glEnable(GL_TEXTURE_2D) : glDisable(GL_TEXTURE_2D);
}

void ModuleRenderer3D::SetColorMaterial(bool enable)
{
	(color_material = enable) ? glEnable(GL_COLOR_MATERIAL) : glDisable(GL_COLOR_MATERIAL);
}

void ModuleRenderer3D::SetWireframe(bool enable)
{
	wireframe = enable;
}

unsigned int ModuleRenderer3D::GetMaxVertexAttributes()
{
	int nrAttributes;
	glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &nrAttributes);
	return nrAttributes;
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

void * ModuleRenderer3D::GetWindowContext() const
{
	return mainContext;
}

void ModuleRenderer3D::WindowSizeChanged(int width, int height)
{
	App->cams->OnWindowChangeSize(width, height);
}

void ModuleRenderer3D::UpdateViewPort(int width, int height) const
{
	int w, h;
	RE_CameraManager::CurrentCamera()->GetTargetWidthHeight(w, h);
	const int x = (width - w) / 2;
	const int y = (height - h) / 2;
	glViewport(x, y, w, h);
}


uint ModuleRenderer3D::GetShaderScene() const
{
	return sceneShader;
}

