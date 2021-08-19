#ifndef __PLAY_PAUSE_WINDOW__
#define __PLAY_PAUSE_WINDOW__

class PlayPauseWindow : public EditorWindow
{
public:
	PlayPauseWindow() : EditorWindow("Play Controls", true) {}
	~PlayPauseWindow() {}

private:

	void Draw(bool secondary = false) override;
};

#endif //!__PLAY_PAUSE_WINDOW__