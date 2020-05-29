#pragma once
#include "Module.h"

class ModuleWwise :
	public Module
{
public:
	ModuleWwise(const char* name, bool start_enabled = true);
	~ModuleWwise();

	bool Init(JSONNode* node) override;
	bool Start() override;
	update_status PreUpdate() override;
	update_status PostUpdate() override;
	bool CleanUp() override;

	void RecieveEvent(const Event& e) override;
	void DrawEditor() override;

	bool Load(JSONNode* node) override;
	bool Save(JSONNode* node) const override;

	static unsigned long LoadBank(const char* buffer, unsigned int size);
};

