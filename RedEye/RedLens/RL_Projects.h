#ifndef RL_PROJECTS_CLASS
#define RL_PROJECTS_CLASS

#include <EASTL/vector.h>
#include <EASTL/string.h>

class RL_Projects
{
private:

	enum class State : int
	{
		NONE = -1,
		NEW,
		LOAD
	} _state = State::NONE;

	struct Project {
		eastl::string name;
		eastl::string path;
	};

public:
	bool Init();
	void CleanUp();

	void DrawGUI();

private:
	void Load();
	void Save() const;

private:
	static constexpr const char* PROJECTS_FILE = "WriteDir/Projects.json";
	static constexpr const char* PROJECT_CONFIG_TEMPLATE = "template";
#ifdef _DEBUG
	static constexpr const char* ENGINE_EXECUTABLE = "Engine-d.exe";
#else
	static constexpr const char* ENGINE_EXECUTABLE = "Engine.exe";
#endif
	eastl::vector<Project> _projects;
};

#endif //RL_PROJECTS_CLASS