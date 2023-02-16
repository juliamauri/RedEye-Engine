#include "JR_Input.h"

#include "JR_Application.h"
#include "JR_WindowAndRenderer.h"

#include <SDL2/SDL.h>
#include <ImGui/backends/imgui_impl_sdl.h>

#define EVENT_NULL -1

JR_Input* JR_Input::instance = nullptr;

bool JR_Input::Init()
{
	if (SDL_InitSubSystem(SDL_INIT_EVENTS) == 0) {

		// SDL Custom events
		//Uint32 custom_event = SDL_RegisterEvents(1);

		//if (custom_event != static_cast<Uint32>(EVENT_NULL))

		return true;
	}

	return false;
}

bool JR_Input::PreUpdate()
{
	SDL_PumpEvents();

	while (SDL_PollEvent(&event))
	{
		ImGui_ImplSDL2_ProcessEvent(&event);
		if (event.type == SDL_QUIT ||
			event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE && event.window.windowID == SDL_GetWindowID(JR_VISUAL->GetWindow()))
			return false;

		//if(event.type == custom_event)
	}

	return true;
}

void JR_Input::CleanUp() {	}