#pragma once

#include <SDL.h>

namespace pgp {

    class IEventListener {
    public:
        enum EventResponse {
            EVT_IGNORED,
            EVT_PROCESSED,
            EVT_DROPPED
        };
        virtual EventResponse onEvent(SDL_Event *evt) = 0;
        virtual ~IEventListener();
    private:

    };

}

