#ifndef __WWISE_WINDOW__
#define __WWISE_WINDOW__

class WwiseWindow : public EditorWindow
{
public:
	WwiseWindow(const char* name = "Wwise", bool start_active = true) : EditorWindow(name, start_active) {}
	~WwiseWindow() {}

private:

	void Draw(bool secondary = false) override;
};

#endif // !__WWISE_WINDOW__