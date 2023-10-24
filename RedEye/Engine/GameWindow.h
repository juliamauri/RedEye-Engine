#ifndef __GAME_WINDOW__
#define __GAME_WINDOW__

#include "RenderedWindow.h"

class GameWindow :public RenderedWindow
{
public:

	GameWindow();
	~GameWindow() final = default;

	RE_Camera& GetCamera() final;
	const RE_Camera& GetCamera() const final;

private:

	void Draw(bool secondary = false) final;
};

#endif //!__GAME_WINDOW__