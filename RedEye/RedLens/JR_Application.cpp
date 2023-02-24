#include "JR_Application.h"

#include "JR_Input.h"
#include "JR_WindowAndRenderer.h"
#include "JR_WindowsManager.h"

#include <SDL2/SDL.h>

JR_Application* JR_Application::App = nullptr;

bool JR_Application::Init()
{
	if (SDL_Init(0) == 0
		&& (JR_Input::instance = input = new JR_Input())->Init()
		&& (visual_magnament = new JR_WindowAndRenderer())->Init()
		&& (windows_manager = new JR_WindowsManager())->Init())
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
	if (windows_manager)
	{
		windows_manager->CleanUp();
		delete windows_manager;
	}

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

	SDL_Quit();
}