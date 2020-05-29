#include "ModuleWwise.h"

#include "Application.h"
#include "RE_FileSystem.h"
#include "OutputLog.h"

#include "ImGui/imgui.h"

#include <AK/SoundEngine/Platforms/Windows/AkTypes.h>
#include <AK/SoundEngine/Common/AkMemoryMgr.h>                  // Memory Manager interface
#include <AK/SoundEngine/Common/AkModule.h>                     // Default memory manager
#include <AK/SoundEngine/Common/IAkStreamMgr.h>                 // Streaming Manager
#include <AK/SoundEngine/Common/AkStreamMgrModule.h>                 // Streaming Manager
#include <AK/Tools/Common/AkPlatformFuncs.h>                    // Thread defines
#include <AK/SoundEngine/Common/AkSoundEngine.h>                // Sound engine
#include <AK/MusicEngine/Common/AkMusicEngine.h>                // Music Engine
#include <AK/SpatialAudio/Common/AkSpatialAudio.h>              // Spatial Audio

#ifdef _DEBUG
#pragma comment( lib, "WWISESDK/binaries/libdebug/AkMemoryMgr.lib" )
#pragma comment( lib, "WWISESDK/binaries/libdebug/AkStreamMgr.lib" )
#pragma comment( lib, "WWISESDK/binaries/libdebug/AkSoundEngine.lib" )
#pragma comment( lib, "WWISESDK/binaries/libdebug/AkMusicEngine.lib" )
#pragma comment( lib, "WWISESDK/binaries/libdebug/AkSpatialAudio.lib" )
#pragma comment( lib, "WWISESDK/binaries/libdebug/CommunicationCentral.lib" )

#elif GAMEMODE //retail releaase

#define AK_OPTIMIZED
#pragma comment( lib, "WWISESDK/binaries/librelase/AkMemoryMgr.lib" )
#pragma comment( lib, "WWISESDK/binaries/librelase/AkStreamMgr.lib" )
#pragma comment( lib, "WWISESDK/binaries/librelase/AkSoundEngine.lib" )
#pragma comment( lib, "WWISESDK/binaries/librelase/AkMusicEngine.lib" )
#pragma comment( lib, "WWISESDK/binaries/librelase/AkSpatialAudio.lib" )
#pragma comment( lib, "WWISESDK/binaries/librelase/CommunicationCentral.lib" )

#else //profiling

#pragma comment( lib, "WWISESDK/binaries/libprofiler/AkMemoryMgr.lib" )
#pragma comment( lib, "WWISESDK/binaries/libprofiler/AkStreamMgr.lib" )
#pragma comment( lib, "WWISESDK/binaries/libprofiler/AkSoundEngine.lib" )
#pragma comment( lib, "WWISESDK/binaries/libprofiler/AkMusicEngine.lib" )
#pragma comment( lib, "WWISESDK/binaries/libprofiler/AkSpatialAudio.lib" )
#pragma comment( lib, "WWISESDK/binaries/libprofiler/CommunicationCentral.lib" )


#endif // DEBUG

#ifndef AK_OPTIMIZED
#include <AK/Comm/AkCommunication.h>
#endif // AK_OPTIMIZED

ModuleWwise::ModuleWwise(const char* name, bool start_enabled) : Module(name, start_enabled)
{}

ModuleWwise::~ModuleWwise()
{}

bool ModuleWwise::Init(JSONNode * node)
{
	bool ret = true;

	App->ReportSoftware("Wwise SDK", AK_WWISESDK_VERSIONNAME, "https://www.audiokinetic.com/products/wwise/");

	AkMemSettings memSettings;
	AK::MemoryMgr::GetDefaultSettings(memSettings);
	if (AK::MemoryMgr::Init(&memSettings) != AK_Success)
	{
		LOG_ERROR("Could not create the audio memory manager.");
		ret = false;
	}

	AkStreamMgrSettings stmSettings;
	AK::StreamMgr::GetDefaultSettings(stmSettings);
	if (!AK::StreamMgr::Create(stmSettings))
	{
		LOG_ERROR("Could not create the audio Streaming Manager");
		ret = false;
	}

	AkInitSettings initSettings;
	AkPlatformInitSettings platformInitSettings;
	AK::SoundEngine::GetDefaultInitSettings(initSettings);
	AK::SoundEngine::GetDefaultPlatformInitSettings(platformInitSettings);
	if (AK::SoundEngine::Init(&initSettings, &platformInitSettings) != AK_Success)
	{
		LOG_ERROR("Could not initialize the audio Sound Engine.");
		ret = false;
	}

	AkMusicSettings musicInit;
	AK::MusicEngine::GetDefaultInitSettings(musicInit);
	if (AK::MusicEngine::Init(&musicInit) != AK_Success)

	{
		LOG_ERROR("Could not initialize the audio Music Engine.");
		ret = false;
	}

	const AkSpatialAudioInitSettings settings; // The constructor fills AkSpatialAudioInitSettings with the recommended default settings. 
	if (AK::SpatialAudio::Init(settings) != AK_Success)

	{
		LOG_ERROR("Could not initialize the Spatial Audio.");
		ret = false;
	}

#ifndef AK_OPTIMIZED

	AkCommSettings commSettings;
	AK::Comm::GetDefaultInitSettings(commSettings);
	if (AK::Comm::Init(commSettings) != AK_Success)
	{
		LOG_ERROR("Could not initialize audio communication.");
		ret = false;
	}

#endif // AK_OPTIMIZED

	return ret;
}

bool ModuleWwise::Start()
{
	AK::StreamMgr::SetCurrentLanguage(AKTEXT("English(US)"));

	AkGameObjectID MY_DEFAULT_LISTENER = 0;
	// Register the main listener.
	AK::SoundEngine::RegisterGameObj(MY_DEFAULT_LISTENER, "My Default Listener");
	// Set one listener as the default.
	AK::SoundEngine::SetDefaultListeners(&MY_DEFAULT_LISTENER, 1);
	AkSoundPosition position;
	position.Set(0, 0, 0,1,0,0,0,1,0);
	AK::SoundEngine::SetPosition(MY_DEFAULT_LISTENER, position);
	RE_FileIO initbank("Settings/DefaultAssets/Init.bnk", App->fs->GetZipPath());
	if (initbank.Load()) {
		LoadBank(initbank.GetBuffer(), initbank.GetSize());
	}
	RE_FileIO bankTest("Settings/DefaultAssets/Music.bnk", App->fs->GetZipPath());
	if (bankTest.Load()) {
		LoadBank(bankTest.GetBuffer(), bankTest.GetSize());
		AK::SoundEngine::PostEvent("PlayVentoAureo", NULL);
	}
	return true;
}

update_status ModuleWwise::PreUpdate()
{
	return update_status::UPDATE_CONTINUE;
}

update_status ModuleWwise::PostUpdate()
{
	OPTICK_CATEGORY("Render Audio", Optick::Category::Audio);
	AK::SoundEngine::RenderAudio();
	return update_status::UPDATE_CONTINUE;
}

bool ModuleWwise::CleanUp()
{
	AK::SoundEngine::ClearBanks();
#ifndef AK_OPTIMIZED
	AK::Comm::Term();
#endif // AK_OPTIMIZED
	AK::MusicEngine::Term();
	AK::SoundEngine::Term();
	if (AK::IAkStreamMgr::Get())
		AK::IAkStreamMgr::Get()->Destroy();
	AK::MemoryMgr::Term();

	return true;
}

void ModuleWwise::RecieveEvent(const Event& e)
{
}

void ModuleWwise::DrawEditor()
{
	if (ImGui::CollapsingHeader("Wwise Audio Engine"))
	{
	}
}

bool ModuleWwise::Load(JSONNode* node)
{
	return true;
}

bool ModuleWwise::Save(JSONNode* node) const
{
	return true;
}

unsigned long ModuleWwise::LoadBank(const char* buffer, unsigned int size)
{
	unsigned long ID = 0;
	//Using LoadBankMemoryView, for keeping the buffer on RE_FileIO sometimes throws AK_DataAlignmentError.
	AKRESULT result = AK::SoundEngine::LoadBankMemoryCopy(buffer, (unsigned long)size, ID);
	if (result != AK_Success) {
		LOG_ERROR("Error while loading bank sound.");
	}
	return ID;
}
