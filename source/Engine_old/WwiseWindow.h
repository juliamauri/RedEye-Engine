#ifndef __WWISE_WINDOW__
#define __WWISE_WINDOW__

class WwiseWindow : public EditorWindow
{
public:
	WwiseWindow() : EditorWindow("Wwise", true) {}
	~WwiseWindow() {}

private:

	void Draw(bool secondary = false) override;
};

#endif // !__WWISE_WINDOW__