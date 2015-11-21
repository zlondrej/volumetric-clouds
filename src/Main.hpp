#pragma once

#define S_OK 0
#define S_SDL_ERROR 1

#include <vector>
#include <SDL.h>

#include "IRegisterable.hpp"
#include "IEventListener.hpp"
#include "IRenderer.hpp"
#include "IProcessor.hpp"

using std::vector;
namespace pgp {

    class Main : public IEventListener {
    protected:
        int argc;
        char **argv;
        SDL_Window *sdlWindow;
        bool quitFlag = false;
        vector<IEventListener*> eventListenerList;
        vector<IRenderer*> rendererList;
        vector<IProcessor*> processorList;

    public:
        Main();
        Main(int argc, char **argv);
        void run();
        int init();
        void onQuit();

        inline void quit() {
            quitFlag = true;
        };

        inline void registerEventListener(IEventListener *listener) {
            eventListenerList.push_back(listener);
        }

        inline void registerRenderer(IRenderer *renderer) {
            rendererList.push_back(renderer);
        }

        inline void registerProcessor(IProcessor *processor) {
            processorList.push_back(processor);
        }

        void autoregister(IRegisterable *registerable);

        // Event listener interface
        virtual IEventListener::EventResponse onEvent(SDL_Event* evt);
    };
}

int main(int argc, char **argv);
