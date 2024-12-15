module;

#include <SDL2/SDL.h>

#include <unordered_map>

export module WindowManager;

namespace {
	std::unordered_map<uint32_t, SDL_Window*> _windows;
}

export namespace RE {
	namespace Window {

		uint32_t NewWindow(const char* title, int x = SDL_WINDOWPOS_CENTERED, int y = SDL_WINDOWPOS_CENTERED, int w = 500, int h = 500, uint32_t flags = (SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_SHOWN))
		{
			SDL_Window* window = SDL_CreateWindow(title, x, y, w, h, flags);
			uint32_t id = SDL_GetWindowID(window);
			_windows[id] = window;
			return id;
		}

		SDL_Window* GetWindow(uint32_t id)
		{
			return _windows[id];
		}

		void RemoveWindow(uint32_t id)
		{
			SDL_DestroyWindow(_windows[id]);
			_windows.erase(id);
		}

		void CleanUp()
		{
			for (auto& window : _windows)
			{
				SDL_DestroyWindow(window.second);
			}
			_windows.clear();
		}
	}
}