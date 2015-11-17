#include <iostream>
#include <csignal>

#include "Main.hpp"
#include "IEventListener.hpp"
#include "IRenderer.hpp"

using namespace std;
using namespace pgp;

Main program;

void sigintHandler(int signal) {
    program.quit();
}

int main(int argc, char **argv) {
    signal(SIGINT, sigintHandler);

    program = Main(argc, argv);

    int status = program.init();

    if (status != S_OK) {
        program.quit();
        return status;
    }

    // TODO: Register event listeners and renderers

    return program.run();
}

Main::Main() {
}

Main::Main(int c, char **v) : argc(c), argv(v) {
}

void Main::run() {

    while (!quitFlag) {
        SDL_Event evt;
        while (SDL_PollEvent(&evt)) {
            int processed = 0;
            IEventListener::EventResponse r;
            for (const IEventListener &listener : eventListenerList) {
                r = listener->onEvent(&evt);

                switch (r) {
                    case IEventListener::EVT_IGNORED:
                        break;
                    case IEventListener::EVT_PROCESSED:
                        processed++;
                        break;
                    case IEventListener::EVT_DROPPED:
                        goto drop;
                }
            }
drop:
            continue;
        }

        for (const IRenderer &renderer : rendererList) {
            renderer->render();
        }
    }

    onQuit();
}

int Main::init() {
    if (SDL_Init(SDL_INIT_EVENTS | SDL_INIT_VIDEO) != 0) {
        return S_SDL_ERROR;
    }

    sdlWindow = SDL_CreateWindow(
            "Volumetric clouds in movement (hopefuly :P) PRE-ALPHA",
            SDL_WINDOWPOS_UNDEFINED,
            SDL_WINDOWPOS_UNDEFINED,
            1200,
            800,
            SDL_WINDOW_OPENGL
            );

    if (sdlWindow == NULL) {
        cout << "SDL_CreateWindow Error: " << SDL_GetError() << endl;
        return S_SDL_ERROR;
    }


    return S_OK;
}

void Main::onQuit() {
    SDL_Quit();
}