#pragma once

#define S_OK 0
#define S_SDL_ERROR 1

#include <SDL.h>

namespace pgp {

    class Main {
    private:
        int argc;
        char **argv;
        SDL_Window *sdlWindow;
        bool quitFlag = false;

    public:
        Main();
        Main(int argc, char **argv);
        int run();
        int init();
        inline void quit() { quitFlag = true; };
        void onQuit();
    };
}

int main(int argc, char **argv);
