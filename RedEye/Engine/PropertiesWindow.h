#ifndef __PROPERTIES_WINDOW__
#define __PROPERTIES_WINDOW__

class PropertiesWindow : public EditorWindow
{
public:
	PropertiesWindow() : EditorWindow("Properties", true) {}
	~PropertiesWindow() final = default;

private:

	void Draw(bool secondary = false) final;
};

#endif //!__PROPERTIES_WINDOW__