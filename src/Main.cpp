#include <iostream>
#include <csignal>

#include "Main.hpp"
#include "Exceptions.hpp"
#include "Camera.hpp"

using namespace std;
using namespace pgp;

Main program;

void sigintHandler(int signal) {
    program.quit();
}

int main(int argc, char **argv) {
    signal(SIGINT, sigintHandler);

    try {
        program.init();

        program.run();
    } catch (string &str) {
        cerr << str << endl;
        return 2;
    } catch (Exception &e) {
        cerr << "Exception: " << e.getMessage() << endl;
        return 1;
    }

    return 0;
}


}

Main::Main() {
    Camera camera(sdlWindow);

    registerEventListener(&camera);
    registerProcessor(&camera);

}

void Main::run() {

    while (!quitFlag) {
        SDL_Event evt;
        while (SDL_PollEvent(&evt)) {
            int processed = 0;
            IEventListener::EventResponse r;
            for (IEventListener *listener : eventListenerList) {
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
            if(quitFlag) {
                goto quit;
            }
        }

        runProcessors(0.02);

        runRenderers();
    }
quit:
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
        throw string(SDL_GetError());
    }

    eventListenerList.push_back(this);

    return S_OK;
}

void Main::onQuit() {
    SDL_Quit();
}

// Event listener interface implementation

IEventListener::EventResponse Main::onEvent(SDL_Event* evt) {
    if(evt->type == SDL_WINDOWEVENT) {
        SDL_WindowEvent *e = &evt->window;

        if (e->event == SDL_WINDOWEVENT_CLOSE) {
            quit();
            return EVT_DROPPED;
        }
    }
    else if(evt->type == SDL_KEYDOWN) {
        SDL_KeyboardEvent *e = &evt->key;

        if(e->keysym.sym == SDLK_ESCAPE) {
            quit();
            return EVT_DROPPED;
        }
    }

    return EVT_IGNORED;
}
