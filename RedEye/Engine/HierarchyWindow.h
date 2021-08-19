#ifndef __HIERARCHY_WINDOW__
#define __HIERARCHY_WINDOW__

class HierarchyWindow : public EditorWindow
{
public:
	HierarchyWindow() : EditorWindow("Heriarchy", true) {}
	~HierarchyWindow() {}

private:

	void Draw(bool secondary = false) override;
};

#endif // !__HIERARCHY_WINDOW__