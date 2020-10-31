#pragma once
#include "Module.h"

#include <EASTL/string.h>
#include <EASTL/vector.h>

struct WwiseEvent {
	eastl::string name;
	unsigned long ID;
	WwiseEvent(const char* _name, unsigned long _ID) : name(_name), ID(_ID) {}
};

struct SoundBank {
	eastl::string name;
	eastl::string path;
	unsigned long ID;
	bool loaded = false;
	eastl::vector<WwiseEvent> events;

	SoundBank(const char* _name, unsigned long _ID) : name(_name), ID(_ID) {}
	~SoundBank();
	void AddEvent(const char* _name, unsigned long _ID) { events.push_back(WwiseEvent(_name, _ID)); }

	void LoadBank();
	void Unload();
};

class ModuleAudio : public Module
{
public:
	ModuleAudio(const char* name = "Audio", bool start_enabled = true);
	~ModuleAudio();

	bool Init(JSONNode* node) override;
	bool Start() override;
	update_status PreUpdate() override;
	update_status PostUpdate() override;
	bool CleanUp() override;

	void RecieveEvent(const Event& e) override;
	void DrawEditor() override;

	void DrawWwiseElementsDetected();

	bool Load(JSONNode* node) override;
	bool Save(JSONNode* node) const override;

	void ReadBanksChanges();

	static void SendRTPC(const char* name, float value);
	static void SendState(const char* stateGroupName, const char* stateName);
	static void SendSwitch(const char* switchName, const char* switchStateName);

private:
	eastl::string audioBanksFolderPath;
	bool located_banksFolder = false;

	signed long long lastSoundBanksInfoModified = 0;
	bool  located_SoundBanksInfo = false;
	
	eastl::vector<SoundBank> soundbanks;
	bool initBnkLoaded = false;
};
