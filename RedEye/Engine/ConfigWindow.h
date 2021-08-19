#ifndef __CONFIG_WINDOW__
#define __CONFIG_WINDOW__

class ConfigWindow : public EditorWindow
{
public:
	ConfigWindow();
	~ConfigWindow() {}

private:

	void Draw(bool secondary = false) override;

public:

	bool changed_config = false;
};

#endif // !__CONFIG_WINDOW__