#ifndef __HIERARCHY_WINDOW__
#define __HIERARCHY_WINDOW__

#include "EditorWindow.h"

class HierarchyWindow : public EditorWindow
{
public:
	HierarchyWindow(const char* name = "Heriarchy", bool start_active = true) : EditorWindow(name, start_active) {}
	~HierarchyWindow() {}

private:

	void Draw(bool secondary = false) override;
};

#endif // !__HIERARCHY_WINDOW__