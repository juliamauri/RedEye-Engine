#ifndef __GAME_WINDOW__
#define __GAME_WINDOW__

class GameWindow :public EditorWindow
{
public:

	GameWindow(const char* name = "Game Scene", bool start_active = true) : EditorWindow(name, start_active) {}
	~GameWindow() {}

	unsigned int GetSceneWidht()const { return (width == 0) ? 500 : width; }
	unsigned int GetSceneHeight()const { return (heigth == 0) ? 500 : heigth; }

	void Recalc() { recalc = true; }

	bool isSelected() const { return isWindowSelected; }
	bool NeedRender() const { return need_render; };

	void UpdateViewPort();

private:

	void Draw(bool secondary = false) override;

	math::float4 viewport = math::float4::zero;
	int width = 0;
	int heigth = 0;

	bool isWindowSelected = false;
	bool recalc = false;
	bool need_render = true;
};

#endif //!__GAME_WINDOW__