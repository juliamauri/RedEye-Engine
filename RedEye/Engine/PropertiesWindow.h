#ifndef __PROPERTIES_WINDOW__
#define __PROPERTIES_WINDOW__

class PropertiesWindow : public EditorWindow
{
public:
	PropertiesWindow() : EditorWindow("Properties", true) {}
	~PropertiesWindow() {}

private:

	void Draw(bool secondary = false) override;
};

#endif //!__PROPERTIES_WINDOW__