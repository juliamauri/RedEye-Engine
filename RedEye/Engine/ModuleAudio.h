#ifndef __MODULEAUDIO__
#define __MODULEAUDIO__

#include <EASTL/vector.h>

struct WwiseEvent
{
	WwiseEvent(const char* _name, unsigned long _ID) : name(_name), ID(_ID) {}

	eastl::string name;
	unsigned long ID;
};

struct SoundBank
{
	SoundBank(const char* _name, unsigned long _ID) : name(_name), ID(_ID) {}
	~SoundBank();

	void AddEvent(const char* _name, unsigned long _ID) { events.push_back(WwiseEvent(_name, _ID)); }

	void LoadBank();
	void Unload();

	eastl::string name;
	eastl::string path;
	unsigned long ID;
	bool loaded = false;
	eastl::vector<WwiseEvent> events;
};

class ModuleAudio
{
public:
	ModuleAudio() {}
	~ModuleAudio() {}

	bool Init();
	bool Start();
	void PostUpdate();
	void CleanUp();

	void DrawEditor();

	void Load();
	void Save() const;

	void DrawWwiseElementsDetected();
	unsigned int ReadBanksChanges(unsigned int extra_ms = 0u);

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

#endif // !__MODULEAUDIO__