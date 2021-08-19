#ifndef __ABOUT_WINDOW__
#define __ABOUT_WINDOW__

#include "EditorWindow.h"
#include <EASTL/vector.h>

class AboutWindow : public EditorWindow
{
public:
	AboutWindow() : EditorWindow("About", false) {}
	~AboutWindow() {}

private:

	void Draw(bool secondary = false) override;

public:

	struct SoftwareInfo { const char* name, * version, * website; };
	eastl::vector<SoftwareInfo> sw_info;
};

#endif //!__ABOUT_WINDOW__