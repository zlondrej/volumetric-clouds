#include <iostream>
#include <csignal>

#include "Main.hpp"
#include "Exceptions.hpp"

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

void glDebugCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar * message, void * userParam) {
    bool throwE;
    cerr << "Debug: " << message << endl;

    throwE = !((type == GL_DEBUG_TYPE_PERFORMANCE) || (severity == GL_DEBUG_SEVERITY_NOTIFICATION));

    if (throwE) {
        throw GLException(string(message));
    }
}

Main::Main() : sdlWindow(NULL), context(NULL), camera(NULL), landscape(NULL) {
}

Main::~Main() {
    delete landscape;
    delete camera;
    delete clouds;
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
            if (quitFlag) {
                goto quit;
            }
        }

        runProcessors(0.0, 0.02);

        runRenderers();

        SDL_GL_SwapWindow(sdlWindow);
    }
quit:
    onQuit();
}

void Main::init() {
    if (SDL_Init(SDL_INIT_EVENTS | SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0) {
        throw string("SDL_Init failed.");
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

    context = SDL_GL_CreateContext(sdlWindow);

    glewInit();

    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);

    glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_NOTIFICATION, 0, NULL, GL_FALSE);
    glDebugMessageControl(GL_DONT_CARE, GL_DEBUG_TYPE_PERFORMANCE, GL_DONT_CARE, 0, NULL, GL_FALSE);

    glDebugMessageCallback((GLDEBUGPROC) glDebugCallback, NULL);

    camera = new Camera(sdlWindow);
    landscape = new Landscape(camera);
    clouds = new Clouds(camera, landscape);

    registerEventListener(this);

    registerEventListener(camera);
    registerProcessor(camera);

    autoregister(landscape);
    autoregister(clouds);
}

void Main::onQuit() {

    delete landscape;
    delete camera;
    delete clouds;

    landscape = NULL;
    camera = NULL;
    clouds = NULL;

    SDL_DestroyWindow(sdlWindow);
    SDL_GL_DeleteContext(context);
    SDL_Quit();
}

// Event listener interface implementation

IEventListener::EventResponse Main::onEvent(SDL_Event* evt) {
    if (evt->type == SDL_WINDOWEVENT) {
        SDL_WindowEvent *e = &evt->window;

        if (e->event == SDL_WINDOWEVENT_CLOSE) {
            quit();
            return EVT_DROPPED;
        }
    } else if (evt->type == SDL_KEYDOWN) {
        SDL_KeyboardEvent *e = &evt->key;

        if (e->keysym.sym == SDLK_ESCAPE) {
            quit();
            return EVT_DROPPED;
        }
    }

    return EVT_IGNORED;
}
