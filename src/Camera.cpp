#include "Camera.hpp"

#include <glm/ext.hpp>
#include <GL/glew.h>

using namespace pgp;
using namespace glm;

Camera::Camera(SDL_Window *_window) : position(0., 0, 0), rotation(0, 0) {
    window = _window;
    SDL_GetWindowSize(window, &(windowSize.x), &(windowSize.y));
}

vec3 Camera::getViewVector() {
    vec3 direction = vec3(1, 1, 1);

    direction = rotateX(direction, rotation.x);
    direction = rotateY(direction, rotation.y);

    return normalize(direction);
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
        ivec3 positive, negative;
        SDL_KeyboardEvent *e = &evt->key;

        positive = max(ivec3(0,0,0), movement);
        negative = min(ivec3(0,0,0), movement);

        int active = (evt->type == SDL_KEYDOWN) ? 1 : 0;

        switch (e->keysym.sym) {
            case SDLK_w:
                positive.z = active;
                break;
            case SDLK_s:
                negative.z = active;
                break;
            case SDLK_a:
                negative.x = active;
                break;
            case SDLK_d:
                positive.x = active;
                break;
            case SDLK_LSHIFT:
                positive.y = active;
                break;
            case SDLK_LCTRL:
                negative.y = active;
                break;
            default:
                return EVT_IGNORED;
        }

        movement = positive - negative;

        return EVT_PROCESSED;

    } else if (evt->type == SDL_MOUSEMOTION) {
        SDL_MouseMotionEvent *e = &evt->motion;

        if (SDL_BUTTON_LEFT & e->state) {
            // TODO: Adjust mouse sensitivity coeficient
            rotation.y += e->xrel * 0.005;
            rotation.x += e->yrel * 0.005;
        }

        return EVT_PROCESSED;
    }
    return EVT_IGNORED;
}

void Camera::step(float dt) {

    if (movement.x == 0 && movement.y == 0 && movement.z == 0) {
        return;
    }

    vec3 direction = vec3(movement.x, 0, movement.z);

    direction = rotateX(direction, rotation.x);
    direction = rotateY(direction, rotation.y);
    direction.y += movement.y;


    // TODO: Add speed coeficient
    position += normalize(direction) * dt;
}

void Camera::render() {
    glViewport(0, 0, windowSize.x, windowSize.y);
}
