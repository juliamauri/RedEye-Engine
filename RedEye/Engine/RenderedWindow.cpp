#include "RenderedWindow.h"

#include "RE_Math.h"
#include "RE_Time.h"
#include "RE_Memory.h"
#include "RE_Json.h"

#include "Application.h"
#include "ModuleRenderer3D.h"
#include "ModuleWindow.h"
#include "ModuleScene.h"

#include "RE_FBOManager.h"
#include "RE_CameraManager.h"

#include "RE_CompPrimitive.h"
#include "RE_CompTransform.h"

#include <EASTL/string.h>
#include <EASTL/bit.h>
#include <ImGui/imgui_internal.h>

RenderedWindow::RenderedWindow(const char* name, bool start_active, uint default_fbo, uint deferred_fbo) :
	EditorWindow(name, start_active)
{
	render_view.fbos[0] = default_fbo > 0U ? default_fbo : RE_FBOManager::CreateFBO(1024, 768, 1, true, true);
	render_view.fbos[1] = deferred_fbo > 0U ? deferred_fbo : RE_FBOManager::CreateDeferredFBO(1024, 768);

	render_view.settings.flags =
		RenderSettings::Flag::FACE_CULLING |
		RenderSettings::Flag::TEXTURE_2D |
		RenderSettings::Flag::COLOR_MATERIAL |
		RenderSettings::Flag::DEPTH_TEST |
		RenderSettings::Flag::CLIP_DISTANCE |

		RenderSettings::Flag::FRUSTUM_CULLING |
		RenderSettings::Flag::SKYBOX |
		RenderSettings::Flag::BLENDED;

	render_view.settings.light = RenderSettings::LightMode::DEFERRED;
}

void RenderedWindow::RenderFBO() const
{
	if (!active || !need_render) return;

	RE_RENDER->DrawScene(
		render_view,
		GetCamera(),
		GatherDrawables(),
		GatherSceneLights(),
		GatherParticleLights(),
		this);

	DrawOther();
}

void RenderedWindow::DrawEditor()
{
	if (ImGui::TreeNodeEx("Render View", ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_OpenOnArrow))
	{
		render_view.DrawEditor(name);
		ImGui::TreePop();
	}
	DrawEditor2();
}

RE_Camera& RenderedWindow::GetCamera() { return RE_CameraManager::MainCamera()->Camera; }
const RE_Camera& RenderedWindow::GetCamera() const { return RE_CameraManager::MainCamera()->Camera; }
const math::Frustum* RenderedWindow::GetFrustum() const { return &GetCamera().GetFrustum(); }

void RenderedWindow::Load(RE_Json* node)
{
	render_view.Load(node->PullJObject("Render View"));
	Load2(node);
	DEL(node)
}

void RenderedWindow::Save(RE_Json* node) const
{
	render_view.Save(node->PushJObject("Render View"));
	Save2(node);
	DEL(node)
}

void RenderedWindow::UpdateWindow()
{
	// Update Size
	static int lastWidth = 0;
	static int lastHeight = 0;
	ImVec2 size = ImGui::GetWindowSize();
	static const float tabHeight = 28.f;
	width = static_cast<int>(size.x);
	height = static_cast<int>(size.y - tabHeight);
	if (lastWidth != width || lastHeight != height)
	{
		RE_FBOManager::ChangeFBOSize(render_view.GetFBO(), width, height);
		GetCamera().SetBounds({ size.x, size.y - tabHeight });

		viewport = GetCamera().GetTargetViewPort(RE_WINDOW->GetWidth(), RE_WINDOW->GetHeight());
		viewport.x = (width - viewport.z) * 0.5f;
		viewport.y = (height - viewport.w + 40.f) * 0.5f;
	}

	// Check if window is focused
	isWindowSelected = (ImGui::IsWindowHovered() && ImGui::IsWindowFocused(ImGuiHoveredFlags_AnyWindow));
	ImGui::SetCursorPos({ viewport.x, viewport.y });

	// Push rendered texture
	auto texture_id = RE_FBOManager::GetTextureID(
		render_view.GetFBO(),
		4 * (render_view.settings.light == RenderSettings::LightMode::DEFERRED));

	ImGui::Image(
		eastl::bit_cast<void*>(&texture_id), // texture
		{ viewport.z, viewport.w }, // size
		{ 0.0, 1.0 }, // uv0
		{ 1.0, 0.0 }); // uv1
}

eastl::stack<const RE_Component*> RenderedWindow::GatherDrawables() const
{
	auto culling_frustum = GetFrustum();
	if (culling_frustum == nullptr)
		return RE_SCENE->GetCScenePool()->GetRootCPtr()->GetAllChildsActiveRenderGeosC();

	// Cull Scene
	eastl::vector<const RE_GameObject*> objects;
	RE_SCENE->FustrumCulling(objects, *culling_frustum);

	eastl::stack<const RE_Component*> to_draw;
	for (auto& go : objects)
	{
		if (go->HasFlag(RE_GameObject::Flag::ACTIVE))
		{
			auto render_geo = go->GetRenderGeoC();
			if (render_geo) to_draw.push(render_geo);
		}
	}
	return to_draw;
}

eastl::vector<const RE_Component*> RenderedWindow::GatherSceneLights() const
{
	return RE_SCENE->GetScenePool()->GetAllCompCPtr(RE_Component::Type::LIGHT);
}

eastl::vector<const RE_CompParticleEmitter*> RenderedWindow::GatherParticleLights() const
{
	eastl::vector<const RE_CompParticleEmitter*> ret;
	for (auto comp : RE_SCENE->GetScenePool()->GetAllCompCPtr(RE_Component::Type::PARTICLEEMITER))
	{
		auto emitter = comp->As<const RE_CompParticleEmitter*>();
		if (emitter->HasLight()) ret.push_back(emitter);
	}
	return ret;
}

OwnCameraRenderedWindow::OwnCameraRenderedWindow(const char* name, bool start_active, uint default_fbo, uint deferred_fbo) :
	RenderedWindow(name, start_active, default_fbo, deferred_fbo)
{
	cam.SetupFrustum();

	grid_size[0] = grid_size[1] = 1.f;
	grid = new RE_CompGrid();
	grid->SetParent(0);
	grid->GridSetUp(50);
}

OwnCameraRenderedWindow::~OwnCameraRenderedWindow()
{
	DEL(grid)
}

void OwnCameraRenderedWindow::DrawEditor2()
{
	// Camera
	eastl::string id = name;
	ImGui::PushID((id + " Camera speed").c_str());
	ImGui::DragFloat("Camera speed", &cam_speed, 0.1f, 0.1f, 100.0f, "%.1f");
	ImGui::PopID();

	ImGui::PushID((id + " Camera sensitivity").c_str());
	ImGui::DragFloat("Camera sensitivity", &cam_sensitivity, 0.01f, 0.01f, 1.0f, "%.2f");
	ImGui::PopID();

	ImGui::PushID((id + " Camera").c_str());
	if (ImGui::TreeNodeEx("Camera", ImGuiTreeNodeFlags_(ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick)))
	{
		cam.DrawProperties();
		ImGui::TreePop();
	}
	ImGui::PopID();

	// Grid
	ImGui::PushID((id + " Draw Grid").c_str());
	bool active_grid = grid->IsActive();
	if (ImGui::Checkbox("Draw Grid", &active_grid)) grid->SetActive(active_grid);
	ImGui::PopID();

	if (active_grid)
	{
		ImGui::PushID((id + " Grid Size").c_str());
		if (ImGui::DragFloat2("Grid Size", grid_size, 0.2f, 0.01f, 100.0f, "%.1f"))
		{
			grid->GetTransformPtr()->SetScale(math::vec(grid_size[0], 0.f, grid_size[1]));
			grid->GetTransformPtr()->Update();
		}
		ImGui::PopID();
	}

	// Other values
	DrawEditor3();
}

void OwnCameraRenderedWindow::DrawDebug() const
{
	if (grid->IsActive()) grid->Draw();
}

void OwnCameraRenderedWindow::UpdateCamera()
{
	if (!active || !isWindowSelected) return;

	if (ImGui::GetKeyData(ImGuiKey::ImGuiKey_LeftAlt)->Down &&
		ImGui::IsMouseDown(ImGuiMouseButton_::ImGuiMouseButton_Left))
	{
		auto& mouse_delta = ImGui::GetIO().MouseDelta;
		if (mouse_delta.x != 0 || mouse_delta.y != 0)
			Orbit(mouse_delta.x, mouse_delta.y);
		return;
	}

	if (ImGui::GetKeyData(ImGuiKey::ImGuiKey_F)->Down)
	{
		Focus();
		return;
	}

	if (ImGui::IsMouseDown(ImGuiMouseButton_::ImGuiMouseButton_Right))
	{
		// Camera Speed
		float speed = cam_speed * RE_Time::DeltaTime();
		if (ImGui::GetKeyData(ImGuiKey::ImGuiKey_LeftShift)->Down) speed *= 2.0f;

		// Move
		if (ImGui::GetKeyData(ImGuiKey::ImGuiKey_W)->Down)	   cam.Move(Direction::FORWARD, speed);
		if (ImGui::GetKeyData(ImGuiKey::ImGuiKey_S)->Down)	   cam.Move(Direction::BACKWARD, speed);
		if (ImGui::GetKeyData(ImGuiKey::ImGuiKey_A)->Down)	   cam.Move(Direction::LEFT, speed);
		if (ImGui::GetKeyData(ImGuiKey::ImGuiKey_D)->Down)	   cam.Move(Direction::RIGHT, speed);
		if (ImGui::GetKeyData(ImGuiKey::ImGuiKey_Space)->Down) cam.Move(Direction::UP, speed);
		if (ImGui::GetKeyData(ImGuiKey::ImGuiKey_C)->Down)	   cam.Move(Direction::DOWN, speed);

		// Rotate
		auto& mouse_delta = ImGui::GetIO().MouseDelta;
		if (mouse_delta.x != 0 || mouse_delta.y != 0)
			cam.Pan(cam_sensitivity * -mouse_delta.x, cam_sensitivity * mouse_delta.y);
	}

	// Zoom
	float mouse_wheel = ImGui::GetIO().MouseWheel;
	if (mouse_wheel != 0) cam.SetFOVRads(cam.GetVFOVRads() - mouse_wheel * RE_Math::deg_to_rad);
}

void OwnCameraRenderedWindow::Load2(RE_Json* node)
{
	// Camera
	cam_speed = node->PullFloat("C_Speed", 25.0f);
	cam_sensitivity = node->PullFloat("C_Sensitivity", 0.01f);
	cam.JsonDeserialize(node->PullJObject("Camera"));

	// Grid
	grid->SetActive(node->PullBool("Grid_Draw", true));
	math::float2 grid_size_vec = node->PullFloat2("Grid_Size", { 1.0f, 1.0f });
	memcpy_s(grid_size, sizeof(float) * 2, grid_size_vec.ptr(), sizeof(float) * 2);
	grid->GetTransformPtr()->SetScale(math::vec(grid_size[0], 0.f, grid_size[1]));
	grid->GetTransformPtr()->Update();

	// Other values
	Load3(node);
}

void OwnCameraRenderedWindow::Save2(RE_Json* node) const
{
	// Camera
	node->Push("C_Speed", cam_speed);
	node->Push("C_Sensitivity", cam_sensitivity);
	cam.JsonSerialize(node->PushJObject("Camera"));

	// Grid
	node->Push("Grid_Draw", grid->IsActive());
	node->PushFloat2("Grid_Size", { grid_size[0], grid_size[0] });

	// Other values
	Save3(node);
}

void RenderedWindow::Draw(bool secondary)
{
	need_render = ImGui::Begin(name, nullptr, ImGuiWindowFlags_::ImGuiWindowFlags_NoCollapse);

	if (!need_render)
	{
		ImGui::End();
		return;
	}

	if (secondary)
	{
		ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
		ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
	}

	UpdateWindow();

	if (secondary)
	{
		ImGui::PopItemFlag();
		ImGui::PopStyleVar();
	}

	ImGui::End();
}