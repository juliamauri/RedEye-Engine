#ifndef __PROPERTIES_WINDOW__
#define __PROPERTIES_WINDOW__

#include "EditorWindow.h"

class PropertiesWindow : public EditorWindow
{
public:
	PropertiesWindow(const char* name = "Properties", bool start_active = true) : EditorWindow(name, start_active) {}
	~PropertiesWindow() {}

private:

	void Draw(bool secondary = false) override;
};

#endif //!__PROPERTIES_WINDOW__