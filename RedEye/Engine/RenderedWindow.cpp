#include "RenderedWindow.h"

#include "RE_Math.h"
#include "RE_Time.h"
#include "RE_Memory.h"
#include "Application.h"
#include "ModuleRenderer3D.h"
#include "RE_FBOManager.h"
#include "ModuleScene.h"
#include "RE_Json.h"
#include "RE_CompPrimitive.h"
#include "RE_CompTransform.h"

#include <EASTL/string.h>
#include <EASTL/bit.h>
#include <ImGui/imgui_internal.h>

void RenderedWindow::RenderFBO() const
{
	if (!active || !need_render) return;

	RE_RENDER->DrawScene(
		render_view,
		GetCamera(),
		GatherDrawables(),
		GatherSceneLights(),
		GatherParticleLights());
}

void RenderedWindow::DrawEditor()
{
	ImGui::PushID((eastl::string(name) + " Render View").c_str());
	if (ImGui::TreeNodeEx("Render View"), ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_OpenOnArrow)
	{
		render_view.DrawEditor(name);
		ImGui::TreePop();
	}
	ImGui::PopID();

	// Other values
	DrawEditor2();
}

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
	int lastWidht = width;
	int lastHeight = height;
	ImVec2 size = ImGui::GetWindowSize();
	width = static_cast<int>(size.x);
	height = static_cast<int>(size.y) - 28;
	if (recalc || lastWidht != width || lastHeight != height)
	{
		GetCamera().SetBounds(size.x, size.y - 28.f);
		RE_FBOManager::ChangeFBOSize(render_view.GetFBO(), width, height);
		UpdateViewPort();
		recalc = false;
	}

	// Check if window is focused
	isWindowSelected = (ImGui::IsWindowHovered() && ImGui::IsWindowFocused(ImGuiHoveredFlags_AnyWindow));
	ImGui::SetCursorPos({ viewport.x, viewport.y });

	// Push rendered texture
	auto texture_id = RE_FBOManager::GetTextureID(render_view.GetFBO(), 4 * (render_view.light_mode == RenderView::LightMode::DEFERRED));
	ImGui::Image(
		eastl::bit_cast<void*>(&texture_id), // texture
		{ viewport.z, viewport.w }, // size
		{ 0.0, 1.0 }, // uv0
		{ 1.0, 0.0 }); // uv1
}

eastl::stack<const RE_Component*> RenderedWindow::GatherDrawables() const
{
	auto culling_frustum = render_view.GetFrustum();
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

OwnCameraRenderedWindow::OwnCameraRenderedWindow(const char* name, bool start_active) :
	RenderedWindow(name, start_active)
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