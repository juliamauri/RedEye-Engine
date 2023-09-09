#ifndef __PLAY_PAUSE_WINDOW__
#define __PLAY_PAUSE_WINDOW__

class PlayPauseWindow : public EditorWindow
{
public:
	PlayPauseWindow() : EditorWindow("Play Controls", true) {}
	~PlayPauseWindow() final = default;

private:

	void Draw(bool secondary = false) final;
};

#endif //!__PLAY_PAUSE_WINDOW__