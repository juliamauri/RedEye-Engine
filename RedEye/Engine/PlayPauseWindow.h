#ifndef __PLAY_PAUSE_WINDOW__
#define __PLAY_PAUSE_WINDOW__

class PlayPauseWindow : public EditorWindow
{
public:
	PlayPauseWindow(const char* name = "Play Controls", bool start_active = true) : EditorWindow(name, start_active) {}
	~PlayPauseWindow() {}

private:

	void Draw(bool secondary = false) override;
};

#endif //!__PLAY_PAUSE_WINDOW__