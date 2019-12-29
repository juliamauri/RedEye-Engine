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

#include "RE_Shader.h"
#include "RE_SkyBox.h"

#include "RE_CompCamera.h"
#include "RE_CompMesh.h"
#include "RE_CompPrimitive.h"
#include "RE_CameraManager.h"
#include "RE_CompTransform.h"

#include "OutputLog.h"
#include "TimeManager.h"
#include "RE_GLCache.h"
#include "RE_FBOManager.h"

#include <stack>

#include "SDL2/include/SDL.h"
#include "Glew/include/glew.h"
#include <gl/GL.h>

#include "MathGeoLib/include/Math/float3.h"
#include "MathGeoLib/include/Math/Quat.h"

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
	sceneEditorFBO = App->fbomanager->CreateFBO(1024, 768, 1, true, true);
	sceneGameFBO = App->fbomanager->CreateFBO(1024, 768);

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

	// Prepare if using wireframe
	if (wireframe)
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	float time = (App->GetState() == GameState::GS_STOP) ? App->time->GetEngineTimer() : App->time->GetGameTimer();
	float dt = App->time->GetDeltaTime();
	std::vector<const char*> activeShaders = App->resources->GetAllResourcesActiveByType(Resource_Type::R_SHADER);

	RE_CompCamera* mainSkybox = (RE_CameraManager::MainCamera()->isUsingSkybox()) ? RE_CameraManager::MainCamera() : nullptr;

	OPTICK_CATEGORY("Scene Editor Draw", Optick::Category::Rendering);
	OPTICK_CATEGORY("Culling Editor", Optick::Category::Rendering);
	RE_CompCamera* current_camera = RE_CameraManager::EditorCamera();
	current_camera->Update();
	for (auto sMD5 : activeShaders) ((RE_Shader*)App->resources->At(sMD5))->UploatMainUniforms(current_camera, dt, time);
	DrawScene(App->cams->GetCullingFrustum(), sceneEditorFBO, mainSkybox, true, true);

	OPTICK_CATEGORY("Scene Game Draw", Optick::Category::Rendering);
	OPTICK_CATEGORY("Culling Game", Optick::Category::Rendering);
	current_camera = RE_CameraManager::MainCamera();
	current_camera->Update();
	for (auto sMD5 : activeShaders) ((RE_Shader*)App->resources->At(sMD5))->UploatMainUniforms(current_camera, dt, time);
	DrawScene(current_camera->GetFrustum(), sceneGameFBO, mainSkybox);

	RE_FBOManager::ChangeFBOBind(0, App->window->GetWidth(), App->window->GetHeight());

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
		RE_CameraManager::EditorCamera()->SetBounds(e.data1.AsInt(), e.data2.AsInt());
		ChangeFBOSize(e.data1.AsInt(), e.data2.AsInt(), true);
		break;
	}
	case GAMEWINDOWCHANGED:
	{
		App->cams->OnWindowChangeSize(e.data1.AsInt(), e.data2.AsInt());
		ChangeFBOSize(e.data1.AsInt(), e.data2.AsInt());
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

void ModuleRenderer3D::DrawScene(const math::Frustum& frustum, unsigned int fbo, RE_CompCamera* mainSkybox, bool debugDraw, bool stencilToSelected)
{
	std::vector<const RE_GameObject*> objects;

	if (cull_scene)
		App->scene->FustrumCulling(objects, frustum);

	RE_FBOManager::ChangeFBOBind(fbo, App->fbomanager->GetWidth(fbo), App->fbomanager->GetHeight(fbo));

	// Reset background with a clear color
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	if (stencilToSelected)
	{
		glClearStencil(0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	}
	else
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	std::stack<RE_Component*> comptsToDraw;
	// Frustum Culling
	if (cull_scene) {
		for (auto object : objects) {
			std::stack<RE_Component*> fromO = object->GetDrawableComponentsItselfOnly();
			while (!fromO.empty())
			{
				comptsToDraw.push(fromO.top());
				fromO.pop();
			}
		}
	}
	else
		comptsToDraw = App->scene->GetRoot()->GetDrawableComponentsWithChilds();

	std::stack<RE_Component*> drawAsLast;
	RE_Component* drawing = nullptr;
	while (!comptsToDraw.empty())
	{
		drawing = comptsToDraw.top();
		bool blend = false;
		if (drawing->GetType() == C_MESH) {
			 blend = ((RE_CompMesh*)drawing)->isBlend();
		}
		if (!blend) drawing->Draw();
		else drawAsLast.push(drawing);
		comptsToDraw.pop();
	}


	// Draw Debug Geometry
	if (debugDraw) App->editor->DrawDebug(lighting);

	// Draw skybox as last
	if (mainSkybox) {
		OPTICK_CATEGORY("SkyBox Draw", Optick::Category::Rendering);
		RE_GLCache::ChangeTextureBind(0);
		RE_Shader* skyboxShader = (RE_Shader*)App->resources->At(App->internalResources->GetDefaultSkyBoxShader());
		uint skysphereshader = skyboxShader->GetID();
		RE_GLCache::ChangeShader(skysphereshader);
		RE_ShaderImporter::setInt(skysphereshader, "cubemap", 0);
		glDepthFunc(GL_LEQUAL);
		mainSkybox->DrawSkybox();
		glDepthFunc(GL_LESS); // set depth function back to default

	}

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	while (!drawAsLast.empty())
	{
		drawAsLast.top()->Draw();
		drawAsLast.pop();
	}

	glDisable(GL_BLEND);

	if (stencilToSelected) {
		RE_GameObject* stencilGO = App->editor->GetSelected();
		if (stencilGO == nullptr)
			return;

		std::stack<RE_Component*> stackComponents = stencilGO->GetDrawableComponentsItselfOnly();
		if (stackComponents.empty())
			return;

		OPTICK_CATEGORY("Stencil Draw", Optick::Category::Rendering);
		std::stack<unsigned int> vaoToStencil;
		std::stack<unsigned int> triangleToStencil;
		std::stack<RE_Component*> stackTemp;
		while (!stackComponents.empty())
		{
			RE_Component* dC = stackComponents.top();
			ComponentType cT = dC->GetType();
			if (cT == ComponentType::C_MESH) {
				vaoToStencil.push(((RE_CompMesh*)dC)->GetVAOMesh());
				triangleToStencil.push(((RE_CompMesh*)dC)->GetTriangleMesh());
				stackTemp.push(dC);
			}
			else if (cT > ComponentType::C_PRIMIVE_MIN&& cT < ComponentType::C_PRIMIVE_MAX) {
				vaoToStencil.push(((RE_CompPrimitive*)dC)->GetVAO());
				triangleToStencil.push(((RE_CompPrimitive*)dC)->GetTriangleCount());
				stackTemp.push(dC);
			}
			stackComponents.pop();
		}

		glEnable(GL_STENCIL_TEST);

		glDisable(GL_DEPTH_TEST);

		while (!vaoToStencil.empty())
		{
			//Getting the scale shader and setting some values
			const char* scaleShader = App->internalResources->GetDefaultScaleShader();
			RE_Shader* sShader = (RE_Shader*)App->resources->At(scaleShader);
			unsigned int shaderiD = sShader->GetID();
			RE_GLCache::ChangeShader(shaderiD);
			RE_GLCache::ChangeVAO(vaoToStencil.top());
			sShader->UploadModel(stencilGO->GetTransform()->GetShaderModel());
			RE_ShaderImporter::setFloat(shaderiD, "useColor", 1.0);
			RE_ShaderImporter::setFloat(shaderiD, "useTexture", 0.0);
			RE_ShaderImporter::setFloat(shaderiD, "cdiffuse", { 1.0, 0.5, 0.0 });
			RE_ShaderImporter::setFloat(shaderiD, "center", (stackTemp.top()->GetType() == ComponentType::C_MESH) ? ((RE_CompMesh*)stackTemp.top())->GetAABB().CenterPoint() : math::vec::zero);

			//Prepare stencil for detect
			glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE); //don't draw to color buffer
			glStencilFunc(GL_ALWAYS, 1, 0xFF); //mark to 1 where pass
			glStencilMask(0xFF);
			glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);

			//Draw scaled mesh 
			RE_ShaderImporter::setFloat(shaderiD, "scaleFactor", 0.5 / stencilGO->GetTransform()->GetLocalScale().Length());
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
			RE_ShaderImporter::setFloat(shaderiD, "scaleFactor", 0.5 / stencilGO->GetTransform()->GetLocalScale().Length());
			glDrawElements(GL_TRIANGLES, triangleToStencil.top() * 3, GL_UNSIGNED_INT, nullptr);

			vaoToStencil.pop();
			triangleToStencil.pop();
		}

		glEnable(GL_DEPTH_TEST);
		glDisable(GL_STENCIL_TEST);
	}
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

void ModuleRenderer3D::ChangeFBOSize(int width, int height, bool editor)
{
	static const int options[2] = { sceneGameFBO, sceneEditorFBO }; 
	App->fbomanager->ChangeFBOSize(options[editor], width, height);
}

unsigned int ModuleRenderer3D::GetRenderedEditorSceneTexture() const
{
	return App->fbomanager->GetTextureID(sceneEditorFBO, 0);
}

unsigned int ModuleRenderer3D::GetRenderedGameSceneTexture() const
{
	return App->fbomanager->GetTextureID(sceneGameFBO, 0);
}
