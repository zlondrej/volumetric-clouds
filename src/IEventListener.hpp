#pragma once

#include <SDL.h>
#include "IRegisterable.hpp"

namespace pgp {

    class IEventListener : virtual public IRegisterable {
    public:

        enum EventResponse {
            EVT_IGNORED,
            EVT_PROCESSED,
            EVT_DROPPED
        };

        virtual EventResponse onEvent(SDL_Event *evt) = 0;

    };

}
