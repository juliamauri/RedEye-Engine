#include "EditorWindow.h"
#include <MGL/Math/float4.h>

#include "GameWindow.h"

#include "Application.h"
#include "ModuleInput.h"
#include "ModuleEditor.h"
#include "ModuleRenderer3D.h"
#include "RE_FBOManager.h"
#include "RE_CameraManager.h"

#include <ImGui/imgui_internal.h>
#include <EASTL/bit.h>

GameWindow::GameWindow() : RenderedWindow("Game Scene", true)
{
	render_view.flags =
		RenderView::Flag::FRUSTUM_CULLING |
		RenderView::Flag::SKYBOX |
		RenderView::Flag::BLENDED |
		RenderView::Flag::FACE_CULLING |
		RenderView::Flag::TEXTURE_2D |
		RenderView::Flag::COLOR_MATERIAL |
		RenderView::Flag::DEPTH_TEST;
	render_view.light_mode = RenderView::LightMode::DEFERRED;
	render_view.fbos = {
		RE_FBOManager::CreateFBO(1024, 768, 1, true, true),
		RE_FBOManager::CreateDeferredFBO(1024, 768) };
}

RE_Camera& GameWindow::GetCamera() { return RE_CameraManager::MainCamera()->Camera; }
const RE_Camera& GameWindow::GetCamera() const { return RE_CameraManager::MainCamera()->Camera; }

void GameWindow::Draw(bool secondary)
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