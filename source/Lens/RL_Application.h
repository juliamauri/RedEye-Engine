#ifndef JR_APPLICATION_CLASS
#define JR_APPLICATION_CLASS

class JR_Application
{
  public:
    bool Init(char* argv[]);

    void Update();

    void CleanUp();

    void RequestExit()
    {
        status = Status::WANT_EXIT;
    }

    void EventListener(union SDL_Event* event);

  public:
    static JR_Application* App;
    class JR_Input* input = nullptr;
    class JR_WindowAndRenderer* visual_magnament = nullptr;
    class RL_FileSystem* file_system = nullptr;
    class RL_Projects* projects_manager = nullptr;

  private:
    enum class Status
    {
        RUNNING,
        WANT_EXIT
    } status = Status::RUNNING;
};

#define APP JR_Application::App
#define JR_INPUT JR_Application::App->input
#define JR_VISUAL JR_Application::App->visual_magnament
#define RL_FS JR_Application::App->file_system
#define RL_PROJECTS JR_Application::App->projects_manager

#endif // !JR_APPLICATION_CLASS