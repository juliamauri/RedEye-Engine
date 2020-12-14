#include "Application.h"

#include "RE_Time.h"
#include "RE_Math.h"
#include "RE_Hardware.h"
#include "RE_FileSystem.h"
#include "RE_ResourceManager.h"
#include "ModuleInput.h"
#include "ModuleWindow.h"
#include "ModuleScene.h"
#include "ModuleEditor.h"
#include "ModuleRenderer3D.h"
#include "ModuleAudio.h"

#include "SDL2\include\SDL.h"
#include "Optick\include\optick.h"
#include <EAAssert\version.h>
#include <EAStdC\internal\Config.h>
#include <EAStdC\EASprintf.h>
#include <eathread\internal\config.h>

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
	editor = new ModuleEditor();
	renderer = new ModuleRenderer3D();
	audio = new ModuleAudio();
}

Application::~Application()
{
	DEL(audio);
	DEL(renderer);
	DEL(editor);
	DEL(scene);
	DEL(window);
	DEL(input);

	DEL(res);
	DEL(fs);

	DEL(hardware);
	DEL(math);
	DEL(time);
}

bool Application::Init(int _argc, char* _argv[])
{
	bool ret = false;
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
			if (input->Init() && window->Init() && scene->Init() && editor->Init() && renderer->Init() && audio->Init())
			{
				hardware->Init();
				res->Init();

				if (scene->Start() && editor->Start() && renderer->Start() && audio->Start())
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
	OPTICK_FRAME("MainThread RedEye");
	RE_LOG_SEPARATOR("Entering Application's Main Loop - %.3f", time->FrameDeltaTime());

	do {
		OPTICK_CATEGORY("Application Update", Optick::Category::GameLogic);

		time->FrameDeltaTime();
		input->PreUpdate();
		editor->PreUpdate();

		scene->Update();
		editor->Update();

		scene->PostUpdate();
		renderer->PostUpdate();
		audio->PostUpdate();

		if (flags & LOAD_CONFIG) LoadConfig();
		if (flags & SAVE_CONFIG) SaveConfig();
		if (time->GetState() == GS_TICK)
		{
			time->PauseGameTimer();
			scene->OnPause();
		}

		unsigned int extra_ms = time->FrameExtraMS();
		if (extra_ms > 0) extra_ms = fs->ReadAssetChanges(extra_ms);
		if (extra_ms > 0) extra_ms = audio->ReadBanksChanges(extra_ms);
		if (extra_ms > 0)
		{
			OPTICK_CATEGORY("Application Delay extra ms", Optick::Category::WaitEmpty);
			time->Delay(extra_ms);
		}

	} while (!(flags & WANT_TO_QUIT));
}

void Application::CleanUp()
{
	if (flags & SAVE_ON_EXIT) fs->SaveConfig();

	audio->CleanUp();
	renderer->CleanUp();
	editor->CleanUp();
	scene->CleanUp();
	window->CleanUp();
	input->CleanUp();

	res->Clear();
	fs->Clear();

	if (SDL_WasInit(0) != 0)
		SDL_Quit();
}

void Application::RecieveEvent(const Event& e)
{
	switch (e.type)
	{
	case PLAY: 
	{
		time->StartGameTimer();
		scene->OnPlay();
		break;
	}
	case PAUSE:
	{
		time->PauseGameTimer();
		scene->OnPause();
		break;
	}
	case TICK:
	{
		time->TickGameTimer();
		scene->OnPlay();
		break;
	}
	case STOP:
	{
		time->StopGameTimer();
		scene->OnStop();
		break;
	}
	case REQUEST_LOAD: flags |= LOAD_CONFIG; break;
	case REQUEST_SAVE: flags |= SAVE_CONFIG; break;
	case REQUEST_QUIT: flags |= WANT_TO_QUIT; break; }
}

void Application::LoadConfig()
{
	if (flags & LOAD_CONFIG) flags -= LOAD_CONFIG;

	OPTICK_CATEGORY("Load module configuration - Application", Optick::Category::IO);
	window->Load();
	renderer->Load();
	audio->Load();
}

void Application::SaveConfig()
{
	if (flags & SAVE_CONFIG) flags -= SAVE_CONFIG;

	OPTICK_CATEGORY("Save module configuration - Application", Optick::Category::IO);
	window->Save();
	renderer->Save();
	audio->Save();

	fs->SaveConfig();
}