#ifndef __ABOUT_WINDOW__
#define __ABOUT_WINDOW__

#include "EditorWindow.h"
#include <EASTL/string.h>
#include <EASTL/vector.h>

class AboutWindow : public EditorWindow
{
public:

	struct SoftwareInfo
	{
		eastl::string name;
		eastl::string version;
		eastl::string website;
	};

public:

	eastl::vector<SoftwareInfo> sw_info;

public:

	AboutWindow() : EditorWindow("About", false) {}
	~AboutWindow() final = default;

private:

	void Draw(bool secondary = false) final;
	void DrawThirdParties() const;
	void DisplayThirdParty(const SoftwareInfo& software) const;
};

#endif //!__ABOUT_WINDOW__