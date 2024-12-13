#ifndef JR_WINDOW_CLASS
#define JR_WINDOW_CLASS

#include <string>

#include <imgui.h>

class JR_Window
{
public:
	JR_Window(const char* name);
	~JR_Window();

	void Draw();

	void PushWindowFlag(ImGuiWindowFlags flag);
	void PopWindowFlag(ImGuiWindowFlags flag);

	void SetTitleAlign(ImVec2 align);
	void SetTitleBGColor(ImVec4 color);

private:
	virtual void DrawContent() = 0;

private:
	std::string name;
	ImGuiWindowFlags w_flags = ImGuiWindowFlags_None;

	bool apply_styles = false;
	ImVec2 title_align = { 0.0f, 0.5f };
	ImVec4 title_bg_color = { 0.09f, 0.09f, 0.09f, 1.00f };
};

#endif // !JR_WINDOW_CLASS