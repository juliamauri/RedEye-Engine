#ifndef JR_INPUT_CLASS
#define JR_INPUT_CLASS

class JR_Input
{
  public:
    bool Init();
    static void StaticEventListener(union SDL_Event* event);

  private:
    void EventListener(union SDL_Event* event);
    static JR_Input* instance;
};

#endif // !JR_INPUT_CLASS