#include "EditorWindow.h"
#include <MGL/Math/float4.h>

#include "GameWindow.h"

#include "Application.h"
#include "ModuleInput.h"
#include "ModuleEditor.h"
#include "ModuleRenderer3D.h"
#include "RE_CameraManager.h"

#include <ImGui/imgui_internal.h>
#include <EASTL/bit.h>

void GameWindow::UpdateViewPort()
{
	RE_CameraManager::MainCamera()->Camera.GetTargetViewPort(viewport);
	viewport.x = (static_cast<float>(width) - viewport.z) * 0.5f;
	viewport.y = (static_cast<float>(heigth) - viewport.w) * 0.5f + 20;
}

void GameWindow::Draw(bool secondary)
{
	if ((need_render = ImGui::Begin(name, nullptr, ImGuiWindowFlags_::ImGuiWindowFlags_NoCollapse)))
	{
		if (secondary)
		{
			ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
			ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
		}

		static int lastWidht = 0;
		static int lastHeight = 0;
		ImVec2 size = ImGui::GetWindowSize();
		width = static_cast<int>(size.x);
		heigth = static_cast<int>(size.y) - 28;

		if (recalc || lastWidht != width || lastHeight != heigth)
		{
			RE_INPUT->Push(RE_EventType::GAMEWINDOWCHANGED, RE_RENDER, RE_Cvar(lastWidht = width), RE_Cvar(lastHeight = heigth));
			RE_INPUT->Push(RE_EventType::GAMEWINDOWCHANGED, RE_EDITOR);
			recalc = false;
		}

		isWindowSelected = (ImGui::IsWindowHovered() && ImGui::IsWindowFocused(ImGuiHoveredFlags_AnyWindow));
		ImGui::SetCursorPos({ viewport.x, viewport.y });
		ImGui::Image(eastl::bit_cast<void*>(RE_RENDER->GetRenderViewTexture(RenderView::Type::GAME)), { viewport.z, viewport.w }, { 0.0, 1.0 }, { 1.0, 0.0 });

		if (secondary)
		{
			ImGui::PopItemFlag();
			ImGui::PopStyleVar();
		}
	}

	ImGui::End();
}