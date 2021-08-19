#ifndef __CONSOLE_WINDOW__
#define __CONSOLE_WINDOW__

class ConsoleWindow : public EditorWindow
{
public:
	ConsoleWindow();
	~ConsoleWindow() {}

	void ChangeFilter(const int new_filter);
	void SwapCategory(const unsigned int category);

	void AppendLog(unsigned int category, const char* text, const char* file_name);

private:

	void Draw(bool secondary = false) override;
	void ResetBuffer();

private:

	ImGuiTextBuffer console_buffer;
	bool needs_rewriting = false;
	bool scroll_to_bot = true;
	int file_filter = -1;
	bool categories[8] = {};

	struct RE_Log
	{
		unsigned int caller_id, category, count;
		eastl::string data;
	};

	eastl::vector<RE_Log> logHistory;
	eastl::map<eastl::string, unsigned int> callers;
};

#endif // !__CONSOLE_WINDOW__