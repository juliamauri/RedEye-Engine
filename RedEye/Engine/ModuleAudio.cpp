#include <EASTL/string.h>

#include "ModuleAudio.h"

#include "RE_Memory.h"
#include "RE_Profiler.h"
#include "RE_Time.h"
#include "Application.h"
#include "RE_FileSystem.h"
#include "RE_FileBuffer.h"
#include "RE_Config.h"
#include "RE_Json.h"

#include <ImGui/imgui.h>
#include <ImGuiImpl/imgui_stdlib.h>
#include <EAStdC/EAString.h>

#include <AK/SoundEngine/Platforms/Windows/AkTypes.h>
#include <AK/SoundEngine/Common/AkMemoryMgr.h>                  // Memory Manager interface
#include <AK/SoundEngine/Common/AkModule.h>                     // Default memory manager
#include <AK/SoundEngine/Common/IAkStreamMgr.h>                 // Streaming Manager
#include <AK/SoundEngine/Common/AkStreamMgrModule.h>            // Streaming Manager
#include <AK/Tools/Common/AkPlatformFuncs.h>                    // Thread defines
#include <AK/SoundEngine/Common/AkSoundEngine.h>                // Sound engine
#include <AK/MusicEngine/Common/AkMusicEngine.h>                // Music Engine
#include <AK/SpatialAudio/Common/AkSpatialAudio.h>              // Spatial Audio

#ifndef _DEBUG
#define AK_OPTIMIZED
#endif // DEBUG

#ifndef AK_OPTIMIZED
#include <AK/Comm/AkCommunication.h>
#endif // AK_OPTIMIZED

#define INITBNKSTR "Init.bnk"

bool ModuleAudio::Init()
{
	bool ret = true;

	RE_PROFILE(RE_ProfiledFunc::Init, RE_ProfiledClass::ModuleAudio)
	RE_LOG("Initializing Module Audio");

	RE_SOFT_NVS("Wwise SDK", eastl::string(
				eastl::to_string(AK_WWISESDK_VERSION_MAJOR) + "." + 
				eastl::to_string(AK_WWISESDK_VERSION_MINOR) + "." + 
				eastl::to_string(AK_WWISESDK_VERSION_SUBMINOR)).c_str(),
				"https://www.audiokinetic.com/products/wwise/");

	AkMemSettings memSettings;
	AK::MemoryMgr::GetDefaultSettings(memSettings);
	if (AK::MemoryMgr::Init(&memSettings) != AK_Success)
	{
		RE_LOG_ERROR("Could not create the audio memory manager.");
		ret = false;
	}

	AkStreamMgrSettings stmSettings;
	AK::StreamMgr::GetDefaultSettings(stmSettings);
	if (!AK::StreamMgr::Create(stmSettings))
	{
		RE_LOG_ERROR("Could not create the audio Streaming Manager");
		ret = false;
	}

	AkInitSettings initSettings;
	AkPlatformInitSettings platformInitSettings;
	AK::SoundEngine::GetDefaultInitSettings(initSettings);
	AK::SoundEngine::GetDefaultPlatformInitSettings(platformInitSettings);
	if (AK::SoundEngine::Init(&initSettings, &platformInitSettings) != AK_Success)
	{
		RE_LOG_ERROR("Could not initialize the audio Sound Engine.");
		ret = false;
	}

	AkMusicSettings musicInit;
	AK::MusicEngine::GetDefaultInitSettings(musicInit);
	if (AK::MusicEngine::Init(&musicInit) != AK_Success)
	{
		RE_LOG_ERROR("Could not initialize the audio Music Engine.");
		ret = false;
	}

	const AkSpatialAudioInitSettings settings; // The constructor fills AkSpatialAudioInitSettings with the recommended default settings. 
	if (AK::SpatialAudio::Init(settings) != AK_Success)

	{
		RE_LOG_ERROR("Could not initialize the Spatial Audio.");
		ret = false;
	}

#ifndef AK_OPTIMIZED

	AkCommSettings commSettings;
	AK::Comm::GetDefaultInitSettings(commSettings);
	if (AK::Comm::Init(commSettings) != AK_Success)
	{
		RE_LOG_ERROR("Could not initialize audio communication.");
		ret = false;
	}

#endif // AK_OPTIMIZED

	Load();

	return ret;
}

static AkGameObjectID MY_DEFAULT_LISTENER = 0;

bool ModuleAudio::Start()
{
	RE_PROFILE(RE_ProfiledFunc::Start, RE_ProfiledClass::ModuleAudio)
	RE_LOG("Starting Module Audio");
	AK::StreamMgr::SetCurrentLanguage(AKTEXT("English(US)"));
	
	MY_DEFAULT_LISTENER = 0;
	// Register the main listener.
	AK::SoundEngine::RegisterGameObj(MY_DEFAULT_LISTENER, "My Default Listener");
	// Set one listener as the default.
	AK::SoundEngine::SetDefaultListeners(&MY_DEFAULT_LISTENER, 1);
	AkSoundPosition position;
	position.Set(0, 0, 0,1,0,0,0,1,0);
	AK::SoundEngine::SetPosition(MY_DEFAULT_LISTENER, position);
	return true;
}

void ModuleAudio::PostUpdate()
{
	RE_PROFILE(RE_ProfiledFunc::PostUpdate, RE_ProfiledClass::ModuleAudio)
	AK::SoundEngine::RenderAudio();
}

void ModuleAudio::CleanUp()
{
	RE_PROFILE(RE_ProfiledFunc::CleanUp, RE_ProfiledClass::ModuleAudio)
	soundbanks.clear();
	AK::SoundEngine::ClearBanks();
#ifndef AK_OPTIMIZED
	AK::Comm::Term();
#endif // AK_OPTIMIZED
	AK::MusicEngine::Term();
	AK::SoundEngine::Term();
	if (AK::IAkStreamMgr::Get())
		AK::IAkStreamMgr::Get()->Destroy();
	AK::MemoryMgr::Term();
}

void ModuleAudio::DrawEditor()
{
	static eastl::string tempPath = audioBanksFolderPath;
	static bool pathChanged = false;

	if (audioBanksFolderPath == "NONE SELECTED") {
		ImGui::Text("PATH NONE SELECTED");
		located_banksFolder = false;
	}
	else {
		ImGui::Text("The actual audio bank Path is: \n%s \n", audioBanksFolderPath.c_str());
		ImGui::Text((located_SoundBanksInfo) ? "SoundBanksInfo.json located at:\n%s\\Windows\\SoundBanksInfo.json" : "Don't located SoundBanksInfo.json.", audioBanksFolderPath.c_str());
		ImGui::Text((initBnkLoaded) ? "Init.bnk is loaded." : "Init.bnk culdn't be loaded.");
		located_banksFolder = true;
	}

	ImGui::Separator();

	if (ImGui::InputText("Audio Bank Folder Name (path root is project folder)", &tempPath))
	{
		if (tempPath != audioBanksFolderPath)
			pathChanged = true;
		else if (tempPath == audioBanksFolderPath)
			pathChanged = false;
	}

	if (pathChanged) {
		ImGui::Text("The path will changed to:\n%s\\ \n", tempPath.c_str());
		if (ImGui::Button("Check and Apply Path")) {

			RE_LOGGER::ScopeProcedureLogging();

			if (tempPath[tempPath.size()] != '\\') tempPath += "\\";

			if (RE_FS->ExistsOnOSFileSystem(tempPath.c_str()))
			{
				audioBanksFolderPath = tempPath;
			}
			else
			{
				RE_LOG_ERROR("This Folder don't exist: \n%s", tempPath.c_str());
				tempPath = audioBanksFolderPath;
			}

			RE_LOGGER::EndScope();
			pathChanged = false;
		}
	}
}

void ModuleAudio::DrawWwiseElementsDetected()
{
	if (!located_banksFolder || !located_SoundBanksInfo)
	{
		if (!located_banksFolder) ImGui::Text("Needed to set the sound banks folder on engine folder.");
		if (!located_SoundBanksInfo) ImGui::Text("Can not locate SoundBanksInfo.json");
		return;
	}

	eastl::string id;
	unsigned int count = 0;
	for (auto& sB : soundbanks)
	{
		ImGui::Text("SoundBank: %s", sB.name.c_str());
		ImGui::SameLine();
		id = eastl::to_string(count);
		id += ". ";
		if (sB.name == INITBNKSTR)
		{
			ImGui::Text("Could not be unloaded.");
		}
		else if(sB.loaded)
		{
			id += "Unload";
			if (ImGui::Button(id.c_str())) sB.Unload();
		}
		else if (!sB.loaded)
		{
			id += "Load";
			if (ImGui::Button(id.c_str())) sB.LoadBank();
		}

		if (!sB.events.empty())
		{
			id = eastl::to_string(count++);
			id += ". Events";

			unsigned int eventCnt = 0;
			if (ImGui::TreeNode(id.c_str()))
			{
				for (auto& e : sB.events)
				{
					ImGui::Text("Event: %s", e.name.c_str());
					id = eastl::to_string(eventCnt++);
					id += ". ";
					id += "Send Event";
					if (sB.loaded)
					{
						ImGui::SameLine();
						if (ImGui::Button(id.c_str())) AK::SoundEngine::PostEvent(e.ID, NULL);
					}
				}
				ImGui::TreePop();
			}
		}
		else
			ImGui::Text("This sound bank don't contain events.");

		ImGui::Separator();
	}
}

void ModuleAudio::Load()
{
	RE_PROFILE(RE_ProfiledFunc::Load, RE_ProfiledClass::ModuleAudio)
	RE_Json* node = RE_FS->ConfigNode("Audio");
	audioBanksFolderPath = node->PullString("FolderBanks", "NONE SELECTED");
	located_banksFolder = (audioBanksFolderPath != "NONE SELECTED");
	DEL(node)
}

void ModuleAudio::Save() const
{
	RE_PROFILE(RE_ProfiledFunc::Save, RE_ProfiledClass::ModuleAudio)
	RE_Json* node = RE_FS->ConfigNode("Audio");
	node->Push("FolderBanks", audioBanksFolderPath.c_str());
	DEL(node)
}

unsigned int ModuleAudio::ReadBanksChanges(unsigned int extra_ms)
{
	RE_PROFILE(RE_ProfiledFunc::ReadAssetChanges, RE_ProfiledClass::ModuleAudio)
	RE_Timer time;
	if (located_banksFolder)
	{
		static const char* platformPath = "Windows\\";

		eastl::string soundbankInfoPath(audioBanksFolderPath);
		soundbankInfoPath += platformPath;
		soundbankInfoPath += "SoundbanksInfo.json";

		if (RE_FS->ExistsOnOSFileSystem(soundbankInfoPath.c_str(), false))
		{
			signed long long lastMod = RE_FS->GetLastTimeModified(soundbankInfoPath.c_str());
			if (lastMod != 0 && lastSoundBanksInfoModified != lastMod)
			{
				soundbanks.clear();
				initBnkLoaded = false;
				Config soundbanksInfo(soundbankInfoPath.c_str());
				if (soundbanksInfo.LoadFromWindowsPath())
				{
					RE_Json* rootNode = soundbanksInfo.GetRootNode("SoundBanksInfo");
					rapidjson::Value* soundBanks = &rootNode->GetDocument()->FindMember("SoundBanksInfo")->value.FindMember("SoundBanks")->value;
					for (auto& v : soundBanks->GetArray())
					{
						SoundBank newSB(v.FindMember("Path")->value.GetString(), static_cast<unsigned long>(EA::StdC::AtoU64(v.FindMember("Id")->value.GetString())));
						newSB.path = audioBanksFolderPath;
						newSB.path += platformPath;
						newSB.path += newSB.name;

						if (v.FindMember("IncludedEvents") != v.MemberEnd())
						{
							rapidjson::Value* events = &v.FindMember("IncludedEvents")->value;
							for (auto& e : events->GetArray())
								newSB.AddEvent(e.FindMember("Name")->value.GetString(), static_cast<unsigned long>(EA::StdC::AtoU64(e.FindMember("Id")->value.GetString())));
						}

						soundbanks.push_back(newSB);
					}

					DEL(rootNode)
					lastSoundBanksInfoModified = lastMod;

					for (auto& sB : soundbanks)
					{
						if (sB.name == INITBNKSTR)
						{
							sB.LoadBank();
							initBnkLoaded = true;
						}
					}
				}
			}

			located_SoundBanksInfo = true;
		}
		else
			located_SoundBanksInfo = false;
	}

	unsigned int realTime = time.Read();
	return (extra_ms < realTime) ? 0 : extra_ms - realTime;
}

void ModuleAudio::SendRTPC(const char* name, float value)
{	
	AK::SoundEngine::SetRTPCValue(name, value);
}

void ModuleAudio::SendState(const char* stateGroupName, const char* stateName)
{
	AK::SoundEngine::SetState(stateGroupName, stateName);
}

void ModuleAudio::SendSwitch(const char* switchName, const char* switchStateName)
{
	AK::SoundEngine::SetSwitch(switchName, switchStateName, MY_DEFAULT_LISTENER);
}

SoundBank::~SoundBank()
{
	if (loaded) Unload();
}

void SoundBank::LoadBank()
{
	RE_FileBuffer* bnkLoaded = RE_FS->QuickBufferFromPDPath(path.c_str());
	if (bnkLoaded != nullptr)
	{
		AKRESULT result = AK::SoundEngine::LoadBankMemoryCopy(bnkLoaded->GetBuffer(), static_cast<AkUInt32>(bnkLoaded->GetSize()), ID);

		if (result == AK_Success) loaded = true;
		else RE_LOG_ERROR("Error while loading bank sound: %s", name.c_str());

		DEL(bnkLoaded)
	}
}

void SoundBank::Unload()
{
	AK::SoundEngine::UnloadBank(ID, nullptr);
	loaded = false;
}
