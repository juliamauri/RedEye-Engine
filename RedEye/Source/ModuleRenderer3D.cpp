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
#include "RE_CompMesh.h"
#include "ModuleScene.h"

#pragma comment(lib, "Glew/lib/glew32.lib")
#pragma comment(lib, "opengl32.lib")

ModuleRenderer3D::ModuleRenderer3D(const char * name, bool start_enabled) : Module(name, start_enabled)
{}

ModuleRenderer3D::~ModuleRenderer3D()
{}

bool ModuleRenderer3D::Init(JSONNode * config_module)
{
	bool ret = true;

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE); 
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1); 
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24); 
	SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8); 
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3); 
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
	
	LOG_SECONDARY("Creating SDL GL Context");
	mainContext = SDL_GL_CreateContext(App->window->GetWindow());
	if (ret = (mainContext != nullptr))
		App->ReportSoftware("OpenGL", (char*)glGetString(GL_VERSION), "https://www.opengl.org/");
	else
		LOG_ERROR("SDL could not create GL Context! SDL_Error: %s", SDL_GetError());

	LOG_SECONDARY("Initializing Glew");
	GLenum error = glewInit();
	if (ret = (error == GLEW_OK))
		App->ReportSoftware("Glew", (char*)glewGetString(GLEW_VERSION), "http://glew.sourceforge.net/");
	else
		LOG_ERROR("Glew could not initialize! Glew_Error: %s", glewGetErrorString(error));

	glEnable(GL_TEXTURE_2D);
	glEnable(GL_DEPTH_TEST);
	//glEnable(GL_CULL_FACE);

	enableVSync(vsync = config_module->PullBool("vsync", false));
	enableVSync(cullface = config_module->PullBool("cullface", false));
	enableVSync(depthtest = config_module->PullBool("depthtest", true));
	enableVSync(lighting = config_module->PullBool("lighting", false));

	camera = new RE_Camera();

	return ret;
}

update_status ModuleRenderer3D::PreUpdate()
{
	update_status ret = UPDATE_CONTINUE;

	//Set background with a clear color
	glMatrixMode(GL_PROJECTION); 
	glLoadMatrixf(camera->GetProjection().ptr());
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	float cameraSpeed = 2.5f;
	if (App->input->GetKey(SDL_SCANCODE_LSHIFT) == KEY_REPEAT)
		cameraSpeed *= 2.0f;
	cameraSpeed *= App->time->GetDeltaTime();

	if (App->input->GetKey(SDL_SCANCODE_W) == KEY_REPEAT)
		camera->Move(Camera_Movement::FORWARD, cameraSpeed);
	if (App->input->GetKey(SDL_SCANCODE_S) == KEY_REPEAT)
		camera->Move(Camera_Movement::BACKWARD, cameraSpeed);
	if (App->input->GetKey(SDL_SCANCODE_A) == KEY_REPEAT)
		camera->Move(Camera_Movement::LEFT, cameraSpeed);
	if (App->input->GetKey(SDL_SCANCODE_D) == KEY_REPEAT)
		camera->Move(Camera_Movement::RIGHT, cameraSpeed);

	float p = 0, y = 0;

	if (App->input->GetKey(SDL_SCANCODE_UP) == KEY_REPEAT)
		p = 1.0f;
	if (App->input->GetKey(SDL_SCANCODE_DOWN) == KEY_REPEAT)
		p = -1.0f;
	if (App->input->GetKey(SDL_SCANCODE_RIGHT) == KEY_REPEAT)
		y = -1.0f;
	if (App->input->GetKey(SDL_SCANCODE_LEFT) == KEY_REPEAT)
		y = 1.0F;

	const MouseData* mouse = App->input->GetMouse();
	if(App->input->GetKey(SDL_SCANCODE_LALT) == KEY_REPEAT && mouse->GetButton(1) == KEY_REPEAT)
		camera->RotateWMouse(-mouse->mouse_x_motion, mouse->mouse_y_motion);

	camera->ZoomMouse(mouse->mouse_wheel_motion);

	/*
	
	if (mouse->GetButton(1) == KEY_REPEAT)
	{
		lastx = newx;
		lasty = newy;

		// get mouse coordinates from Windows
		newx = mouse->mouse_x;
		newy = mouse->mouse_y;

		// these lines limit the camera's range
		if (newy < 200)
			newy = 200;
		if (newy > 450)
			newy = 450;

		if ((newx - lastx) > 0)             // mouse moved to the right
			yaw += 3.0f;
		else if ((newx - lastx) < 0)     // mouse moved to the left
			yaw -= 3.0f;

		camera->SetEulerAngle(yaw, newy);

		
		if (firstMouse)
		{
			lastx = mouse->mouse_x;
			lasty = mouse->mouse_y;
			firstMouse = false;
		}

		float xoffset = mouse->mouse_x - lastx;
		float yoffset = lasty - mouse->mouse_y;
		lastx = mouse->mouse_x;
		lasty = mouse->mouse_y;

		float sensitivity = 0.05;
		xoffset *= sensitivity;
		yoffset *= sensitivity;

		yaw += xoffset;
		pitch += yoffset;

		camera->SetEulerAngle(yoffset, xoffset);
	}
	*/
	//camera->Update();

	return ret;
}

update_status ModuleRenderer3D::PostUpdate()
{
	update_status ret = UPDATE_CONTINUE;

	if(isLine)
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	
	App->scene->DrawScene();

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	
	// Draw Editor
	if(App->editor != nullptr)
		App->editor->Draw();

	//Swap buffers
	SDL_GL_SwapWindow(App->window->GetWindow());

	return ret;
}

bool ModuleRenderer3D::CleanUp()
{
	bool ret = true;

	delete camera;

	//Delete context
	SDL_GL_DeleteContext(mainContext);

	return ret;
}

void ModuleRenderer3D::DrawEditor()
{
	if(ImGui::CollapsingHeader("Renderer 3D"))
	{
		if (ImGui::Checkbox((vsync) ? "Disable VSync" : "Enable VSync", &vsync))
			enableVSync(vsync);

		if (ImGui::Checkbox((cullface) ? "Disable Cull Face" : "Enable Cull Face", &cullface))
			enableFaceCulling(cullface);

		if (ImGui::Checkbox((depthtest) ? "Disable Depht Test" : "Enable Depht Test", &depthtest))
			enableDepthTest(depthtest);

		if (ImGui::Checkbox((lighting) ? "Disable Lighting" : "Enable Lighting", &lighting))
			enableLighting(lighting);

		ImGui::Checkbox((isLine) ? "Disable Wireframe" : "Enable Wireframe", &isLine);
	}
}

void ModuleRenderer3D::RecieveEvent(const Event * e)
{
	
}

void ModuleRenderer3D::enableVSync(const bool enable)
{
	SDL_GL_SetSwapInterval(enable ? 1 : 0);
}

void ModuleRenderer3D::enableDepthTest(const bool enable)
{
	enable ? glEnable(GL_DEPTH_TEST) : glDisable(GL_DEPTH_TEST);
}

void ModuleRenderer3D::enableFaceCulling(const bool enable)
{
	enable ? glEnable(GL_CULL_FACE) : glDisable(GL_CULL_FACE);
}

void ModuleRenderer3D::enableLighting(const bool enable)
{
	enable ? glEnable(GL_LIGHTING) : glDisable(GL_LIGHTING);
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
	glLoadMatrixf((camera->GetView() * model).ptr());

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

void ModuleRenderer3D::ResetCamera()
{
}

void ModuleRenderer3D::ResetAspectRatio()
{
	glViewport(0, 0, App->window->GetWidth(), App->window->GetHeight());
	//camera->ResetFov();
}
