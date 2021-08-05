#ifndef __CONFIG_WINDOW__
#define __CONFIG_WINDOW__

#include "EditorWindow.h"

class ConfigWindow : public EditorWindow
{
public:
	ConfigWindow(const char* name = "Configuration", bool start_active = true);
	~ConfigWindow();

private:

	void Draw(bool secondary = false) override;

public:

	bool changed_config;
};

#endif // !__CONFIG_WINDOW__