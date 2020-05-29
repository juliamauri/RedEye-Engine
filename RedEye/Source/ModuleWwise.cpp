#include "ModuleWwise.h"

#include "Application.h"
#include "RE_FileSystem.h"
#include "OutputLog.h"
#include "RE_HandleErrors.h"

#include "ImGui/imgui.h"
#include "ImGui/misc/cpp/imgui_stdlib.h"

#include <EAStdC/EAString.h>

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

#define INITBNKSTR "Init.bnk"

ModuleWwise::ModuleWwise(const char* name, bool start_enabled) : Module(name, start_enabled)
{
}

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

	audioBanksFolderPath = node->PullString("FolderBanks", "NONE SELECTED");
	located_banksFolder = (audioBanksFolderPath != "NONE SELECTED");

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

	return true;
}

void ModuleWwise::RecieveEvent(const Event& e)
{
}

void ModuleWwise::DrawEditor()
{
	if (ImGui::CollapsingHeader("Wwise Audio Engine"))
	{
		static eastl::string rootPath = App->fs->GetExecutableDirectory();
		static eastl::string tempPath = audioBanksFolderPath;
		static bool pathChanged = false;

		if (audioBanksFolderPath == "NONE SELECTED") {
			ImGui::Text("PATH NONE SELECTED");
			located_banksFolder = false;
		}
		else {
			ImGui::Text("The actual audio bank Path is: \n%s%s \n", rootPath.c_str(), audioBanksFolderPath.c_str());
			ImGui::Text((located_SoundBanksInfo) ? "SoundBanksInfo.json located at:\n%s\\Windows\\SoundBanksInfo.json" : "Don't located SoundBanksInfo.json.", audioBanksFolderPath.c_str());
			ImGui::Text((initBnkLoaded) ? "Init.bnk is loaded." : "Init.bnk culdn't be loaded.");
			located_banksFolder = true;
		}

		ImGui::Separator();

		if (ImGui::InputText("Audio Bank Folder Name (path root is project folder)", &tempPath))
		{
			if (tempPath != audioBanksFolderPath)
				pathChanged = true;
			else if(tempPath == audioBanksFolderPath)
				pathChanged = false;
		}

		if (pathChanged) {
			ImGui::Text("The path will changed to:\n%s%s\\ \n", rootPath.c_str(), tempPath.c_str());
			if (ImGui::Button("Check and Apply Path")) {

				App->handlerrors->StartHandling();

				if (tempPath[tempPath.size()] != '\\')
					tempPath += "\\";

				eastl::string tmp = rootPath;
				tmp += tempPath;
				if (App->fs->ExistsOnOSFileSystem(tmp.c_str()))
					audioBanksFolderPath = tempPath;
				else {
					LOG_ERROR("This Folder don't exist: \n%s%s", rootPath.c_str(), tempPath.c_str());
					tempPath = audioBanksFolderPath;
				}

				App->handlerrors->StopHandling();
				if (App->handlerrors->AnyErrorHandled()) {
					App->handlerrors->ActivatePopUp();
				}

				pathChanged = false;
			}
		}
	}
}

void ModuleWwise::DrawWwiseElementsDetected()
{
	if (!located_banksFolder || !located_SoundBanksInfo) {

		if (!located_banksFolder) ImGui::Text("Needed to set the sound banks folder on engine folder.");
		if (!located_SoundBanksInfo) ImGui::Text("Can not locate SoundBanksInfo.json");
		return;
	}

	eastl::string id;
	unsigned int count = 0;
	for (auto& sB : soundbanks) {
		ImGui::Text("SoundBank: %s", sB.name.c_str());
		ImGui::SameLine();
		id = eastl::to_string(count);
		id += ". ";
		if (sB.name == INITBNKSTR)
			ImGui::Text("Could not be unloaded.");
		else if(sB.loaded)
		{
			id += "Unload";
			if (ImGui::Button(id.c_str())) {
				sB.Unload();
			}
		}
		else if (!sB.loaded) {
			id += "Load";
			if (ImGui::Button(id.c_str())) {
				sB.LoadBank();
			}
		}

		if (!sB.events.empty()) {
			id = eastl::to_string(count++);
			id += ". Events";

			unsigned int eventCnt = 0;
			if (ImGui::TreeNode(id.c_str())) {

				for (auto& e : sB.events) {
	
					ImGui::Text("Event: %s", e.name.c_str());
					id = eastl::to_string(eventCnt++);
					id += ". ";
					id += "Send Event";
					if (sB.loaded) {
						ImGui::SameLine();
						if (ImGui::Button(id.c_str())) {
							AK::SoundEngine::PostEvent(e.ID, NULL);
						}
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

bool ModuleWwise::Load(JSONNode* node)
{
	audioBanksFolderPath = node->PullString("FolderBanks", "NONE SELECTED");
	located_banksFolder = (audioBanksFolderPath != "NONE SELECTED");
	
	return true;
}

bool ModuleWwise::Save(JSONNode* node) const
{
	node->PushString("FolderBanks", audioBanksFolderPath.c_str());
	return true;
}

void ModuleWwise::ReadBanksChanges()
{
	if (located_banksFolder) {

		static const char* platformPath = "Windows\\";
		eastl::string soundbankInfoPath(App->fs->GetExecutableDirectory());

		eastl_size_t posOfBack = soundbankInfoPath.find_last_of('\\..');
		if (posOfBack != eastl::string::npos) {
			eastl_size_t toBack = soundbankInfoPath.find_last_of('\\', posOfBack - 3);
			soundbankInfoPath.erase(toBack + 1, posOfBack - toBack + 1);	
		}

		soundbankInfoPath += audioBanksFolderPath;
		soundbankInfoPath += platformPath;
		soundbankInfoPath += "SoundbanksInfo.json";

		if (App->fs->ExistsOnOSFileSystem(soundbankInfoPath.c_str(), false)) {
			unsigned long lastMod = App->fs->GetLastTimeModified(soundbankInfoPath.c_str());
			if (lastMod != 0 && lastSoundBanksInfoModified != lastMod) {
				soundbanks.clear();
				initBnkLoaded = false;

				Config soundbanksInfo(soundbankInfoPath.c_str(), "None");
				if (soundbanksInfo.LoadFromWindowsPath()) {
					JSONNode* rootNode = soundbanksInfo.GetRootNode("SoundBanksInfo");

					rapidjson::Value* soundBanks = &rootNode->GetDocument()->FindMember("SoundBanksInfo")->value.FindMember("SoundBanks")->value;
					for (auto& v : soundBanks->GetArray()) {
						SoundBank newSB(v.FindMember("Path")->value.GetString(), EA::StdC::Atof(v.FindMember("Id")->value.GetString()));
						newSB.path = App->fs->GetExecutableDirectory();
						newSB.path += audioBanksFolderPath;
						newSB.path += platformPath;
						newSB.path += newSB.name;

						if (v.FindMember("IncludedEvents") != v.MemberEnd()) {
							rapidjson::Value* events = &v.FindMember("IncludedEvents")->value;
							for (auto& e : events->GetArray()) {
								newSB.AddEvent(e.FindMember("Name")->value.GetString(), EA::StdC::Atof(e.FindMember("Id")->value.GetString()));
							}
						}

						soundbanks.push_back(newSB);
					}

					DEL(rootNode);
					lastSoundBanksInfoModified = lastMod;

					for (auto& sB : soundbanks) {
						if (sB.name == INITBNKSTR) {
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

}

SoundBank::~SoundBank()
{
	if (loaded) Unload();
}

void SoundBank::LoadBank()
{
	RE_FileIO* bnkLoaded = App->fs->QuickBufferFromPDPath(path.c_str());
	if (bnkLoaded != nullptr) {
		AKRESULT result = AK::SoundEngine::LoadBankMemoryCopy(bnkLoaded->GetBuffer(), bnkLoaded->GetSize(), ID);
		if (result != AK_Success) {
			LOG_ERROR("Error while loading bank sound: %s", name.c_str());
		}
		else
			loaded = true;
		DEL(bnkLoaded);
	}
}

void SoundBank::Unload()
{
	AK::SoundEngine::UnloadBank(ID, nullptr);
	loaded = false;
}
