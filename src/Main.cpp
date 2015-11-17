#include "Main.hpp"

#include <iostream>
#include <csignal>

using namespace std;
using namespace pgp;

Main program;

void sigintHandler(int signal) {
    program.quit();
}

int main(int argc, char **argv) {
    signal(SIGINT, sigintHandler);

    program = Main(argc, argv);

    return program.run();
}

Main::Main() {
}

Main::Main(int c, char **v) : argc(c), argv(v) {
}

int Main::run() {
    int status = init();

    if (status != S_OK) {
        quit();
        return status;
    }

    while(!quitFlag) {
        SDL_Event evt;
        while(SDL_PollEvent(&evt)) {
            // TODO: Handle events
        }

        // TODO: Possibly do some processing stuff
        // TODO: Render stuff
    }

    onQuit();
    return S_OK;
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

    if(sdlWindow == NULL) {
        cout << "SDL_CreateWindow Error: " << SDL_GetError() << endl;
        return S_SDL_ERROR;
    }

    
    return S_OK;
}

void Main::onQuit() {
    SDL_Quit();
}