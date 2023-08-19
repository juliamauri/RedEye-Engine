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

	AboutWindow() : EditorWindow("About", false) {}
	~AboutWindow() final = default;

private:
	void Draw(bool secondary = false) override;

	void DrawThirdParties() const;
	void DisplayThirdParty(const SoftwareInfo& software) const;

public:

	eastl::vector<SoftwareInfo> sw_info;
};

#endif //!__ABOUT_WINDOW__