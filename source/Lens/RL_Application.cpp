#include "RL_Application.h"

#include "RL_Input.h"
#include "RL_WindowAndRenderer.h"

#include "RL_Projects.h"

#include <SDL2/SDL.h>

import FileSystem;
import EventSystem;
import GUIManager;

JR_Application* JR_Application::App = nullptr;

bool JR_Application::Init(char* argv[])
{
	if (SDL_Init(0) == 0
		&& RE::FileSystem::Init(argv, "RedEye", "Lens")
		&& RE::Event::Init([this](SDL_Event* event) { this->EventListener(event); })
		&& (input = new JR_Input())->Init()
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
		switch (status)
		{
		case JR_Application::Status::RUNNING:
			RE::Event::PumpEvents();
			visual_magnament->PostUpdate();
			break;
		case JR_Application::Status::WANT_EXIT:
		default:
			return;
			break;
		}
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

	if (input) delete input;

	if (projects_manager)
	{
		projects_manager->CleanUp();
		delete projects_manager;
	}

	RE::FileSystem::CleanUp();

	SDL_Quit();
}

void JR_Application::EventListener(SDL_Event* event)
{
	if (event->type == SDL_QUIT) RequestExit();
}
