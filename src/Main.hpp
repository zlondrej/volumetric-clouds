#pragma once

#define S_OK 0
#define S_SDL_ERROR 1

#include <SDL.h>

#include "RegistrablesContainer.hpp"




namespace pgp {

    class Main : public RegistrablesContainer, public IEventListener {
    protected:
        SDL_Window *sdlWindow;
        bool quitFlag = false;

    public:
        Main();
        void run();
        int init();
        void onQuit();

        inline void quit() {
            quitFlag = true;
        };

        // Event listener interface
        virtual IEventListener::EventResponse onEvent(SDL_Event* evt);
    };
}

int main(int argc, char **argv);
