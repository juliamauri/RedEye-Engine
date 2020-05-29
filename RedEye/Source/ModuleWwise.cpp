#include "ModuleWwise.h"

#include "Application.h"
#include "OutputLog.h"


#include <AK/SoundEngine/Common/AkTypes.h>

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
	return true;
}

update_status ModuleWwise::PreUpdate()
{
	return update_status::UPDATE_CONTINUE;
}

update_status ModuleWwise::PostUpdate()
{
	return update_status::UPDATE_CONTINUE;
}

bool ModuleWwise::CleanUp()
{
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
}

bool ModuleWwise::Load(JSONNode* node)
{
	return true;
}

bool ModuleWwise::Save(JSONNode* node) const
{
	return true;
}
