#include "Application.h"

#include "RE_Assert.h"
#include "RE_Memory.h"
#include "RE_Profiler.h"
#include "RE_Time.h"
#include "RE_Random.h"
#include "RE_Hardware.h"
#include "RE_FileSystem.h"
#include "RE_ResourceManager.h"
#include "ModuleInput.h"
#include "ModuleWindow.h"
#include "ModuleScene.h"
#include "ModulePhysics.h"
#include "ModuleEditor.h"
#include "ModuleRenderer3D.h"
#include "ModuleAudio.h"

#include <SDL2/SDL.h>
#include <EAAssert/version.h>
#include <EAStdC/internal/Config.h>
#include <EAStdC/EASprintf.h>
#include <eathread/internal/config.h>

Application::Application()
{
	fs = new RE_FileSystem();
	res = new RE_ResourceManager();

	input = new ModuleInput();
	window = new ModuleWindow();
	scene = new ModuleScene();
	physics = new ModulePhysics();
	editor = new ModuleEditor();
	renderer = new ModuleRenderer3D();
	audio = new ModuleAudio();

#ifdef INTERNAL_PROFILING
	ProfilingTimer::operations.reserve(20000u);
#endif // INTERNAL_PROFILING
}

Application::~Application()
{
	DEL(audio)
	DEL(renderer)
	DEL(editor)
	DEL(scene)
	DEL(physics)
	DEL(window)
	DEL(input)

	DEL(res)
	DEL(fs)

#ifdef INTERNAL_PROFILING
	if (ProfilingTimer::recording) RE_Profiler::Deploy();
#endif // INTERNAL_PROFILING
}

bool Application::Init(int _argc, char* _argv[])
{
	RE_PROFILE(RE_ProfiledFunc::Init, RE_ProfiledClass::Application);
	RE_LOG_SEPARATOR("Initializing Application");

	const size_t capacity = 8;
	eastl::string tmp;
	tmp.set_capacity(capacity);

	EA::StdC::Snprintf(&tmp[0], capacity, "%i.%i.%i", EAASSERT_VERSION_MAJOR, EAASSERT_VERSION_MINOR, EAASSERT_VERSION_PATCH);
	RE_SOFT_NVS("EABase", EABASE_VERSION, "https://github.com/electronicarts/EABase");
	RE_SOFT_NVS("EASTL", EASTL_VERSION, "https://github.com/electronicarts/EASTL");
	RE_SOFT_NVS("EAStdC", EASTDC_VERSION, "https://github.com/electronicarts/EAStdC");
	RE_SOFT_NVS("EAAssert", tmp.c_str(), "https://github.com/electronicarts/EAAssert");
	RE_SOFT_NVS("EAThread", EATHREAD_VERSION, "https://github.com/electronicarts/EAThread");
	RE_SOFT_NVS("Optick", "1.2.9", "https://optick.dev/");
	RE_SOFT_NS("MathGeoLib", "https://github.com/juj/MathGeoLib");
	RE_SOFT_NVS("Assimp", "5.0.1", "http://www.assimp.org/");
	RE_SOFT_N("gpudetect");

	RE_LOG_SECONDARY("Initializing SDL file I/O and threading subsystems");
	if (SDL_Init(0) != 0)
	{
		RE_LOG_ERROR("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
		return false;
	}

	SDL_version sdl_version;
	SDL_GetVersion(&sdl_version);
	EA::StdC::Snprintf(&tmp[0], capacity, "%u.%u.%u", sdl_version.major, sdl_version.minor, sdl_version.patch);
	RE_SOFT_NVS("SDL", tmp.c_str(), "https://www.libsdl.org/");

	argc = _argc;
	argv = _argv;
	if (!fs->Init(argc, argv))
	{
		RE_LOG_ERROR("Application Init failed to initialize File System");
		return false;
	}

	if (!InitModules())
	{
		RE_LOG_ERROR("Application Init failed to Initialize Modules");
		return false;
	}

	// Initialize Intermediate Systems
	RE_Random::SetRandomRNGSeed();
	RE_Hardware::CheckHardware();
	res->Init();

	if (!StartModules())
	{
		RE_LOG_ERROR("Application Init failed to Start Modules");
		return false;
	}

	res->ThumbnailResources();

	return true;
}

void Application::MainLoop()
{
	RE_LOG_SEPARATOR("Entering Application's Main Loop - %.3f", RE_Time::FrameDeltaTime());
	do {
		RE_PROFILE_FRAME();
		RE_PROFILE(RE_ProfiledFunc::Update, RE_ProfiledClass::Application);

		RE_Time::FrameDeltaTime();

		input->PreUpdate();
		editor->PreUpdate();

		scene->Update();
		physics->Update();
		editor->Update();

		scene->PostUpdate();
		renderer->PostUpdate();
		audio->PostUpdate();

		if (HasFlag(Flag::LOAD_CONFIG)) LoadConfig();
		if (HasFlag(Flag::SAVE_CONFIG)) SaveConfig();
		if (RE_Time::StateIs(RE_Time::GameState::TICK))
		{
			RE_Time::PauseGameTimer();
			scene->OnPause();
		}

		unsigned int extra_ms = RE_Time::FrameExtraMS();
		if (extra_ms > 0) extra_ms = fs->ReadAssetChanges(extra_ms);
		if (extra_ms > 0) extra_ms = audio->ReadBanksChanges(extra_ms);
		if (extra_ms > 0) RE_Time::Delay(extra_ms);

	} while (!HasFlag(Flag::WANT_TO_QUIT));
}

void Application::CleanUp()
{
	if (HasFlag(Flag::SAVE_ON_EXIT)) fs->SaveConfig();

	audio->CleanUp();
	renderer->CleanUp();
	editor->CleanUp();
	physics->CleanUp();
	scene->CleanUp();
	window->CleanUp();
	input->CleanUp();

	res->Clear();
	fs->Clear();

#ifdef INTERNAL_PROFILING
	RE_Profiler::Exit();
#endif // INTERNAL_PROFILING

	if (SDL_WasInit(0) != 0) SDL_Quit();
}

void Application::Quit()
{
	AddFlag(Flag::WANT_TO_QUIT);
}

void Application::RecieveEvent(const Event& e)
{
	switch (e.type)
	{
	case RE_EventType::PLAY:
	{
		scene->OnPlay();
		physics->OnPlay(RE_Time::StateIs(RE_Time::GameState::PAUSE));

		RE_Time::StartGameTimer();
		break;
	}
	case RE_EventType::PAUSE:
	{
		scene->OnPause();
		physics->OnPause();

		RE_Time::PauseGameTimer();
		break;
	}
	case RE_EventType::TICK:
	{
		scene->OnPlay();
		physics->OnPlay(RE_Time::StateIs(RE_Time::GameState::PAUSE));

		RE_Time::TickGameTimer();
		break;
	}
	case RE_EventType::STOP:
	{
		scene->OnStop();
		physics->OnStop();

		RE_Time::StopGameTimer();
		break;
	}
	case RE_EventType::REQUEST_LOAD: AddFlag(Flag::LOAD_CONFIG); break;
	case RE_EventType::REQUEST_SAVE: AddFlag(Flag::SAVE_CONFIG); break;
	case RE_EventType::REQUEST_QUIT: AddFlag(Flag::WANT_TO_QUIT); break;
	default: RE_ASSERT(false); break;
	}
}

bool Application::InitModules()
{
	if (!input->Init())
	{
		RE_LOG_ERROR("Application Init failed to initialize Input Module");
		return false;
	}

	if (!window->Init())
	{
		RE_LOG_ERROR("Application Init failed to initialize Window Module");
		return false;
	}

	if (!scene->Init())
	{
		RE_LOG_ERROR("Application Init failed to initialize Scene Module");
		return false;
	}

	if (!renderer->Init())
	{
		RE_LOG_ERROR("Application Init failed to initialize Renderer Module");
		return false;
	}

	if (!editor->Init())
	{
		RE_LOG_ERROR("Application Init failed to initialize Editor Module");
		return false;
	}

	if (!audio->Init())
	{
		RE_LOG_ERROR("Application Init failed to initialize Audio Module");
		return false;
	}

	return true;
}

bool Application::StartModules()
{
	if (!scene->Start())
	{
		RE_LOG_ERROR("Application Init failed to Start Scene Module");
		return false;
	}

	if (!editor->Start())
	{
		RE_LOG_ERROR("Application Init failed to Start Editor Module");
		return false;
	}

	if (!renderer->Start())
	{
		RE_LOG_ERROR("Application Init failed to Start Render Module");
		return false;
	}

	if (!audio->Start())
	{
		RE_LOG_ERROR("Application Init failed to Start Audio Module");
		return false;
	}

	return true;
}

void Application::LoadConfig()
{
	RE_PROFILE(RE_ProfiledFunc::Load, RE_ProfiledClass::Application);

	if (HasFlag(Flag::LOAD_CONFIG))
		RemoveFlag(Flag::LOAD_CONFIG);

	window->Load();
	editor->Load();
	renderer->Load();
	physics->Load();
	audio->Load();
}

void Application::SaveConfig()
{
	RE_PROFILE(RE_ProfiledFunc::Save, RE_ProfiledClass::Application);

	if (HasFlag(Flag::SAVE_CONFIG))
		RemoveFlag(Flag::SAVE_CONFIG);

	window->Save();
	editor->Save();
	renderer->Save();
	physics->Save();
	audio->Save();

	fs->SaveConfig();
}

inline void Application::AddFlag(Flag flag)
{
	flags |= static_cast<uchar>(flag);
}

inline void Application::RemoveFlag(Flag flag)
{
	flags -= static_cast<uchar>(flag);
}

inline const bool Application::HasFlag(Flag flag) const
{
	return flags & static_cast<const uchar>(flag);
}