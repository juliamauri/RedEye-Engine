#include "Application.h"

#include "RE_Memory.h"
#include "RE_Profiler.h"
#include "RE_Time.h"
#include "RE_Math.h"
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

#include "SDL2\include\SDL.h"
#include <EAAssert\version.h>
#include <EAStdC\internal\Config.h>
#include <EAStdC\EASprintf.h>
#include <eathread\internal\config.h>

namespace AppFlags {
	constexpr unsigned char
		EMPLY_FLAGS = 0,
		LOAD_CONFIG = 1 << 0,
		SAVE_CONFIG = 1 << 1,
		WANT_TO_QUIT = 1 << 2,
		SAVE_ON_EXIT = 1 << 3;
}

Application::Application()
{
	time = new RE_Time();
	math = new RE_Math();
	hardware = new RE_Hardware();

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
	DEL(audio);
	DEL(renderer);
	DEL(editor);
	DEL(scene);
	DEL(physics);
	DEL(window);
	DEL(input);

	DEL(res);
	DEL(fs);

	DEL(hardware);
	DEL(math);
	DEL(time);

#ifdef INTERNAL_PROFILING
	if (ProfilingTimer::recording) RE_Profiler::Deploy();
#endif // INTERNAL_PROFILING
}

bool Application::Init(int _argc, char* _argv[])
{
	bool ret = false;
	RE_PROFILE(PROF_Init, PROF_Application);
	RE_LOG_SEPARATOR("Initializing Application");

	char tmp[8];
	EA::StdC::Snprintf(tmp, 8, "%i.%i.%i", EAASSERT_VERSION_MAJOR, EAASSERT_VERSION_MINOR, EAASSERT_VERSION_PATCH);
	RE_SOFT_NVS("EABase", EABASE_VERSION, "https://github.com/electronicarts/EABase");
	RE_SOFT_NVS("EASTL", EASTL_VERSION, "https://github.com/electronicarts/EASTL");
	RE_SOFT_NVS("EAStdC", EASTDC_VERSION, "https://github.com/electronicarts/EAStdC");
	RE_SOFT_NVS("EAAssert", tmp, "https://github.com/electronicarts/EAAssert");
	RE_SOFT_NVS("EAThread", EATHREAD_VERSION, "https://github.com/electronicarts/EAThread");
	RE_SOFT_NVS("Optick", "1.2.9", "https://optick.dev/");
	RE_SOFT_NS("MathGeoLib", "https://github.com/juj/MathGeoLib");
	RE_SOFT_NVS("Assimp", "5.0.1", "http://www.assimp.org/");
	RE_SOFT_N("gpudetect");

	RE_LOG_SECONDARY("Initializing SDL file I/O and threading subsystems");
	if (SDL_Init(0) == 0)
	{
		SDL_version sdl_version;
		SDL_GetVersion(&sdl_version);
		EA::StdC::Snprintf(tmp, 8, "%u.%u.%u", sdl_version.major, sdl_version.minor, sdl_version.patch);
		RE_SOFT_NVS("SDL", tmp, "https://www.libsdl.org/");

		if (fs->Init(argc = _argc, argv = _argv))
		{
			if (input->Init() &&
				window->Init() &&
				scene->Init() &&
				editor->Init() &&
				renderer->Init() &&
				audio->Init())
			{
				hardware->Init();
				res->Init();

				if (scene->Start() &&
					editor->Start() &&
					renderer->Start() &&
					audio->Start())
				{
					res->ThumbnailResources();
					ret = true;
				} else RE_LOG_ERROR("Application Init failed to start modules");
			} else RE_LOG_ERROR("Application Init failed to initialize");
		} else RE_LOG_ERROR("Application Init failed to initialize file system");
	} else RE_LOG_ERROR("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());

	return ret;
}

void Application::MainLoop()
{
	RE_LOG_SEPARATOR("Entering Application's Main Loop - %.3f", time->FrameDeltaTime());
	do {
		RE_PROFILE_FRAME();
		RE_PROFILE(PROF_Update, PROF_Application);

		time->FrameDeltaTime();

		input->PreUpdate();
		editor->PreUpdate();

		scene->Update();
		physics->Update();
		editor->Update();

		scene->PostUpdate();
		renderer->PostUpdate();
		audio->PostUpdate();

		if (flags & AppFlags::LOAD_CONFIG) LoadConfig();
		if (flags & AppFlags::SAVE_CONFIG) SaveConfig();
		if (time->GetState() == GS_TICK)
		{
			time->PauseGameTimer();
			scene->OnPause();
		}

		unsigned int extra_ms = time->FrameExtraMS();
		if (extra_ms > 0) extra_ms = fs->ReadAssetChanges(extra_ms);
		if (extra_ms > 0) extra_ms = audio->ReadBanksChanges(extra_ms);
		if (extra_ms > 0) time->Delay(extra_ms);

	} while (!(flags & AppFlags::WANT_TO_QUIT));
}

void Application::CleanUp()
{
	if (flags & AppFlags::SAVE_ON_EXIT) fs->SaveConfig();

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

	if (SDL_WasInit(0) != 0)
		SDL_Quit();
}

void Application::Quit()
{
	flags |= AppFlags::WANT_TO_QUIT;
}

void Application::RecieveEvent(const Event& e)
{
	switch (e.type)
	{
	case RE_EventType::PLAY:
	{
		scene->OnPlay();
		physics->OnPlay(time->GetState() == GS_PAUSE);

		time->StartGameTimer();
		break;
	}
	case RE_EventType::PAUSE:
	{
		scene->OnPause();
		physics->OnPause();

		time->PauseGameTimer();
		break;
	}
	case RE_EventType::TICK:
	{
		scene->OnPlay();
		physics->OnPlay(time->GetState() == GS_PAUSE);

		time->TickGameTimer();
		break;
	}
	case RE_EventType::STOP:
	{
		scene->OnStop();
		physics->OnStop();

		time->StopGameTimer();
		break;
	}
	case RE_EventType::REQUEST_LOAD: flags |= AppFlags::LOAD_CONFIG; break;
	case RE_EventType::REQUEST_SAVE: flags |= AppFlags::SAVE_CONFIG; break;
	case RE_EventType::REQUEST_QUIT: flags |= AppFlags::WANT_TO_QUIT; break;
	default: break;	}
}

void Application::LoadConfig()
{
	if (flags & AppFlags::LOAD_CONFIG) flags -= AppFlags::LOAD_CONFIG;

	RE_PROFILE(PROF_Load, PROF_Application);
	window->Load();
	renderer->Load();
	audio->Load();
}

void Application::SaveConfig()
{
	if (flags & AppFlags::SAVE_CONFIG) flags -= AppFlags::SAVE_CONFIG;

	RE_PROFILE(PROF_Save, PROF_Application);
	window->Save();
	renderer->Save();
	audio->Save();

	fs->SaveConfig();
}