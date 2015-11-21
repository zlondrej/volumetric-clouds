#pragma once

#include <SDL.h>
#include <glm/glm.hpp>

#include "IEventListener.hpp"
#include "IProcessor.hpp"

namespace pgp {

    class Camera : public IEventListener, public IProcessor {
    protected:
        SDL_Window *window;
        glm::ivec2 windowSize;
        glm::vec3 position;
        glm::vec2 rotation;
        glm::ivec3 movement;

    public:
        Camera(SDL_Window *window);

        virtual IEventListener::EventResponse onEvent(SDL_Event* evt);

        virtual void step(float dt);

    };

}
