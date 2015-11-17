#pragma once

#define S_OK 0
#define S_SDL_ERROR 1

#include <vector>
#include <SDL.h>

#include "IEventListener.hpp"
#include "IRenderer.hpp"

using std::vector;
namespace pgp {

    class Main {
    private:
        int argc;
        char **argv;
        SDL_Window *sdlWindow;
        bool quitFlag = false;
        vector<IEventListener> eventListenerList;
        vector<IRenderer> rendererList;

    public:
        Main();
        Main(int argc, char **argv);
        void run();
        int init();
        void onQuit();

        inline void quit() {
            quitFlag = true;
        };

        inline void registerEventListener(IEventListener &listener) {
            eventListenerList.push_back(listener);
        }

        inline void registerRenderer(IRenderer &renderer) {
            rendererList.push_back(renderer);
        }

    };
}

int main(int argc, char **argv);
