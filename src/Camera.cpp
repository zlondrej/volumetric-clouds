#include "Camera.hpp"

#define GLM_FORCE_RADIANS
#include <glm/ext.hpp>

using namespace pgp;
using namespace glm;

Camera::Camera(SDL_Window *_window) : position(0, 0, 0), rotation(0, 0) {
    window = _window;
    SDL_GetWindowSize(window, &(windowSize.x), &(windowSize.y));
}

IEventListener::EventResponse Camera::onEvent(SDL_Event* evt) {
    if (evt->type == SDL_WINDOWEVENT) {
        SDL_WindowEvent *e = &evt->window;

        if (e->event == SDL_WINDOWEVENT_RESIZED
                || e->event == SDL_WINDOWEVENT_SIZE_CHANGED) {

            windowSize.x = e->data1;
            windowSize.y = e->data2;

            return EVT_PROCESSED;
        }
    } else if (evt->type == SDL_KEYDOWN || evt->type == SDL_KEYUP) {
        SDL_KeyboardEvent *e = &evt->key;

        int sign = (evt->type == SDL_KEYDOWN) ? 1 : -1;

        switch (e->keysym.sym) {
            case SDLK_w:
                movement.z += sign;
                break;
            case SDLK_s:
                movement.z -= sign;
                break;
            case SDLK_a:
                movement.x -= sign;
                break;
            case SDLK_d:
                movement.x += sign;
                break;
            case SDLK_LSHIFT:
                movement.y += sign;
                break;
            case SDLK_LCTRL:
                movement.y -= sign;
                break;
            default:
                return EVT_IGNORED;
        }

    } else if (evt->type == SDL_MOUSEMOTION) {
        SDL_MouseMotionEvent *e = &evt->motion;

        if (SDL_BUTTON_LEFT & e->state) {
            // TODO: Adjust mouse sensitivity coeficient
            rotation.y += e->xrel * 0.2;
            rotation.x += e->yrel * 0.2;
        }
    }
    return EVT_IGNORED;
}

void Camera::step(float dt) {
    vec3 direction = vec3(movement.x, 0, movement.y);

    direction = rotateX(direction, rotation.x);
    direction = rotateY(direction, rotation.y);
    direction.z = movement.z;


    // TODO: Add speed coeficient
    position += normalize(direction) * dt;
}
