module;

#include <SDL2/SDL.h>
#include <vector>
#include <functional>

export module EventSystem;

namespace {
	std::function<void(SDL_Event*)> _systemListener;
	std::function<void(SDL_Event*)> _inputListener;
	std::function<void(SDL_Event*)> _windowListener;

	std::vector<unsigned int> _customEvents;
	std::vector<std::function<void(SDL_Event*)>> _customlisteners;

	constexpr const uint32_t EVENT_NULL = -1;
}

export namespace RE {
	namespace Event {
		bool Init(std::function<void(SDL_Event*)> systemlistener)
		{
			_systemListener = systemlistener;
			if (SDL_InitSubSystem(SDL_INIT_EVENTS))
				return false;
			return true;
		}

		void SetInputListener(std::function<void(SDL_Event*)> listener)
		{
			_inputListener = listener;
		}

		void SetWindowListener(std::function<void(SDL_Event*)> listener)
		{
			_windowListener = listener;
		}

		void AddCustomEvent(uint32_t& event)
		{
			event = SDL_RegisterEvents(1);
			if (event != EVENT_NULL)
				_customEvents.push_back(event);
		}

		void AddCustomListener(std::function<void(SDL_Event*)> listener)
		{
			_customlisteners.push_back(listener);
		}

		void PumpEvents()
		{
			SDL_PumpEvents();

			SDL_Event event;
			while (SDL_PollEvent(&event))
			{
				_systemListener(&event);
				_inputListener(&event);
				if (event.type == SDL_WINDOWEVENT)
					_windowListener(&event);

				if (_customEvents.empty() || _customlisteners.empty()) continue;
				auto isCustom = std::find(_customEvents.begin(), _customEvents.end(), event.type);
				if (isCustom != _customEvents.end()) {
					for (auto& listener : _customlisteners)
					{
						listener(&event);
					}
				}
			}
		}
	}
}