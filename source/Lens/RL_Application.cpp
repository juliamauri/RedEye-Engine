#include "RL_Application.h"

#include "RL_Input.h"
#include "RL_WindowAndRenderer.h"

#include "RL_Projects.h"

#include <SDL2/SDL.h>

import FileSystem;
import GUIManager;

JR_Application* JR_Application::App = nullptr;

bool JR_Application::Init(char* argv[])
{
	if (SDL_Init(0) == 0
		&& RE::FileSystem::Init(argv, "RedEye", "Lens")
		&& (JR_Input::instance = input = new JR_Input())->Init()
		&& (visual_magnament = new JR_WindowAndRenderer())->Init()
		&& RE::GUI::Init(visual_magnament->GetWindow(), visual_magnament->GetContext())
		&& (projects_manager = new RL_Projects())->Init())
			return true;

	CleanUp();
	return false;
}

void JR_Application::Update()
{
	while (true)
	{
		if (!input->PreUpdate()) return;
		visual_magnament->PostUpdate();
	}
}

void JR_Application::CleanUp()
{
	RE::GUI::CleanUp();

	if (visual_magnament)
	{
		visual_magnament->CleanUp();
		delete visual_magnament;
	}

	if (input)
	{
		input->CleanUp();
		delete input;
	}

	if (projects_manager)
	{
		projects_manager->CleanUp();
		delete projects_manager;
	}

	RE::FileSystem::CleanUp();

	SDL_Quit();
}