#ifndef __HIERARCHY_WINDOW__
#define __HIERARCHY_WINDOW__

class HierarchyWindow : public EditorWindow
{
public:
	HierarchyWindow() : EditorWindow("Heriarchy", true) {}
	~HierarchyWindow() final = default;

private:

	void Draw(bool secondary = false) override;
};

#endif // !__HIERARCHY_WINDOW__