#ifndef __ABOUT_WINDOW__
#define __ABOUT_WINDOW__

#include "EditorWindow.h"
#include <EASTL/vector.h>

class AboutWindow : public EditorWindow
{
public:
	AboutWindow() : EditorWindow("About", false) {}
	~AboutWindow() final {}

private:

	void Draw(bool secondary = false) override;

	void DrawThirdParties() const;

public:

	struct SoftwareInfo
	{
		const char* name = nullptr;
		const char* version = nullptr;
		const char* website = nullptr;
	};
	eastl::vector<SoftwareInfo> sw_info;
};

#endif //!__ABOUT_WINDOW__