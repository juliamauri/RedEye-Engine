#ifndef JR_INPUT_CLASS
#define JR_INPUT_CLASS

class JR_Input
{
  public:
    bool Init();

    void EventListener(union SDL_Event* event);
};

#endif // !JR_INPUT_CLASS