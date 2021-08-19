#ifndef __GAME_WINDOW__
#define __GAME_WINDOW__

class GameWindow :public EditorWindow
{
public:

	GameWindow() : EditorWindow("Game Scene", true) {}
	~GameWindow() {}

	unsigned int GetSceneWidth() const
	{
		int no_branch[2] = { 500, width };
		return static_cast<unsigned int>(no_branch[width > 0]);
	}

	unsigned int GetSceneHeight() const
	{
		int no_branch[2] = { 500, heigth };
		return static_cast<unsigned int>(no_branch[heigth > 0]);
	}

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