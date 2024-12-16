#include "RL_Application.h"
#include <SDL2/SDL.h>

int main(int argc, char* argv[])
{
    APP = new JR_Application();
    if (APP->Init(argv))
    {
        APP->Update();
        APP->CleanUp();
    }
    delete APP;
    return 0;
}