#ifndef RL_PROJECTS_CLASS
#define RL_PROJECTS_CLASS

#include <string>
#include <vector>

class RL_Projects
{
  private:
    enum class State : int
    {
        NONE = -1,
        NEW,
        LOAD
    } _state = State::NONE;

    struct Project
    {
        std::string name;
        std::string path;
    };

  public:
    bool Init();
    void CleanUp();

    void DrawGUI();

  private:
    void Load();
    void Save() const;

    bool GenerateProject(const char* name, const char* path);
    bool LoadProject(const char* path, std::string& outName);

  private:
    static constexpr const char* PROJECTS_FILE = "WriteDir/Projects.json";
    static constexpr const char* PROJECT_CONFIG_TEMPLATE = "template";
    static constexpr const char* IMGUI_CONFIG_TEMPLATE = "imgui.ini";
#ifdef _DEBUG
    static constexpr const char* ENGINE_EXECUTABLE = "Engine-d.exe";
#else
    static constexpr const char* ENGINE_EXECUTABLE = "Engine.exe";
#endif
    std::vector<Project> _projects;
};

#endif // RL_PROJECTS_CLASS