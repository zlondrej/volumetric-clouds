#pragma once

#include <SDL.h>
#include <glm/glm.hpp>

#include "IEventListener.hpp"
#include "IProcessor.hpp"
#include "IRenderer.hpp"

namespace pgp {

    using namespace glm;

    class Camera : public IEventListener, public IProcessor, public IRenderer {
    protected:
        SDL_Window *window;
        ivec2 windowSize;
        vec3 position;
        vec2 rotation;
        ivec3 movement;

    public:
        Camera(SDL_Window *window);

        vec3 getPosition();

        virtual IEventListener::EventResponse onEvent(SDL_Event* evt);

        virtual void step(float dt);

        virtual void render();

    };

}
