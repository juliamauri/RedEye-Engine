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
#include "FileSystem.h"
#include "RE_Camera.h"
#include "RE_Mesh.h"
#include "ModuleScene.h"

#pragma comment(lib, "Glew/lib/glew32.lib")
#pragma comment(lib, "opengl32.lib")

ModuleRenderer3D::ModuleRenderer3D(const char * name, bool start_enabled) : Module(name, start_enabled)
{}

ModuleRenderer3D::~ModuleRenderer3D()
{}

bool ModuleRenderer3D::Init(JSONNode * node)
{
	bool ret = false;

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

update_status ModuleRenderer3D::PreUpdate()
{
	update_status ret = UPDATE_CONTINUE;

	// Reset projection & view
	glMatrixMode(GL_PROJECTION); 
	glLoadMatrixf(App->editor->GetCamera()->GetProjection().ptr());
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	//Set background with a clear color
	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	return ret;
}

update_status ModuleRenderer3D::PostUpdate()
{
	update_status ret = UPDATE_CONTINUE;

	// Draw Scene
	if(wireframe) glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	if (App->scene != nullptr) App->scene->DrawScene();
	
	// Draw Editor
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	if(App->editor != nullptr) App->editor->Draw();

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
	}
}

bool ModuleRenderer3D::Load(JSONNode * node)
{
	bool ret = (node != nullptr);

	if (ret)
	{
		SetVSync(node->PullBool("vsync", vsync));
		SetDepthTest(node->PullBool("cullface", cullface));
		SetFaceCulling(node->PullBool("depthtest", depthtest));
		SetLighting(node->PullBool("lighting", lighting));
		SetTexture2D(node->PullBool("texture 2d", texture2d));
		SetColorMaterial(node->PullBool("color material", color_material));
		SetWireframe(node->PullBool("wireframe", wireframe));
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

void ModuleRenderer3D::SetVSync(const bool enable)
{
	SDL_GL_SetSwapInterval((vsync = enable) ? 1 : 0);
}

void ModuleRenderer3D::SetDepthTest(const bool enable)
{
	(cullface = enable) ? glEnable(GL_DEPTH_TEST) : glDisable(GL_DEPTH_TEST);
}

void ModuleRenderer3D::SetFaceCulling(const bool enable)
{
	(depthtest = enable) ? glEnable(GL_CULL_FACE) : glDisable(GL_CULL_FACE);
}

void ModuleRenderer3D::SetLighting(const bool enable)
{
	(lighting = enable) ? glEnable(GL_LIGHTING) : glDisable(GL_LIGHTING);
}

void ModuleRenderer3D::SetTexture2D(const bool enable)
{
	(texture2d = enable) ? glEnable(GL_TEXTURE_2D) : glDisable(GL_TEXTURE_2D);
}

void ModuleRenderer3D::SetColorMaterial(const bool enable)
{
	(color_material = enable) ? glEnable(GL_COLOR_MATERIAL) : glDisable(GL_COLOR_MATERIAL);
}

void ModuleRenderer3D::SetWireframe(const bool enable)
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
	glLoadMatrixf((App->editor->GetCamera()->GetView() * model).ptr());

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

void ModuleRenderer3D::ResetAspectRatio()
{
	glViewport(0, 0, App->window->GetWidth(), App->window->GetHeight());
	//camera->ResetFov();
}
