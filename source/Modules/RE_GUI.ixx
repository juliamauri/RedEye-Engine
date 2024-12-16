module;

#include <functional>
#include <imgui.h>
#include <string>
#include <vector>

export module GUI;

namespace
{
    struct Window
    {
        std::string name;
        ImGuiWindowFlags w_flags = ImGuiWindowFlags_None;
        bool apply_styles = false;
        ImVec2 title_align = {0.0f, 0.5f};
        ImVec4 title_bg_color = {0.09f, 0.09f, 0.09f, 1.00f};
        std::function<void()> DrawContent;
    };
    std::vector<Window> _windows;
} // namespace

export namespace RE
{
    namespace GUI
    {
        /**
         * @brief Adds a new window to the GUI system.
         * @param name The name of the window.
         * @param DrawContent The function to draw the content of the window.
         * @return The index of the newly added window.
         */
        unsigned int AddWindow(const char* name, std::function<void()> DrawContent)
        {
            _windows.push_back({name, ImGuiWindowFlags_None, false, ImVec2(0.0f, 0.5f),
                                ImVec4(0.09f, 0.09f, 0.09f, 1.00f), DrawContent});
            return _windows.size() - 1;
        }

        /**
         * @brief Pushes a window flag to the specified window.
         * @param index The index of the window.
         * @param flag The window flag to push.
         */
        void PushWindowFlag(unsigned int index, ImGuiWindowFlags flag)
        {
            _windows[index].w_flags |= flag;
        }

        /**
         * @brief Pops a window flag from the specified window.
         * @param index The index of the window.
         * @param flag The window flag to pop.
         */
        void PopWindowFlag(unsigned int index, ImGuiWindowFlags flag)
        {
            _windows[index].w_flags &= ~flag;
        }

        /**
         * @brief Sets the title alignment of the specified window.
         * @param index The index of the window.
         * @param align The alignment of the title.
         */
        void SetTitleAlign(unsigned int index, ImVec2 align)
        {
            _windows[index].apply_styles = true;
            _windows[index].title_align = align;
        }

        /**
         * @brief Sets the title background color of the specified window.
         * @param index The index of the window.
         * @param color The background color of the title.
         */
        void SetTitleBGColor(unsigned int index, ImVec4 color)
        {
            _windows[index].apply_styles = true;
            _windows[index].title_bg_color = color;
        }

        /**
         * @brief Draws all the windows in the GUI system.
         */
        void DrawWindows()
        {
            for (auto& window : _windows)
            {
                static bool pop;
                if (pop = window.apply_styles)
                {
                    ImGui::PushStyleVar(ImGuiStyleVar_WindowTitleAlign, window.title_align);
                    ImGui::PushStyleColor(ImGuiCol_TitleBgActive, window.title_bg_color);
                }

                if (ImGui::Begin(window.name.c_str(), NULL, window.w_flags))
                {
                    window.DrawContent();
                }

                ImGui::End();

                if (pop)
                {
                    ImGui::PopStyleColor();
                    ImGui::PopStyleVar();
                }
            }
        }
    } // namespace GUI
} // namespace RE