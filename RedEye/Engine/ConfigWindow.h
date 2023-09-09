#ifndef __CONFIG_WINDOW__
#define __CONFIG_WINDOW__

class ConfigWindow : public EditorWindow
{
public:
	ConfigWindow();
	~ConfigWindow() final = default;

private:

	void Draw(bool secondary = false) final;

	void DrawOptions() const;
	void DrawModules() const;

public:

	bool changed_config = false;
};

#endif // !__CONFIG_WINDOW__