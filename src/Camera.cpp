#include "Camera.hpp"

#include <glm/ext.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <GL/glew.h>

#define PI_HALF_CLAMP (1.57079632679f - 0.0001f)

using namespace pgp;
using namespace glm;

Camera::Camera(SDL_Window *_window) : position(0.0, 70.0, 0.0), rotation(-0.25, 0), movement(0, 0, 0) {
    window = _window;
    updateWindowSize();
}

void Camera::updateWindowSize() {
    // Size in pixels, which differs from the window size on HiDPI displays.
    SDL_GL_GetDrawableSize(window, &(windowSize.x), &(windowSize.y));
}

vec3 Camera::getViewVector() {
    vec3 direction = vec3(0, 0, 1);

    direction = rotateX(direction, rotation.x);
    direction = rotateY(direction, rotation.y);

    return normalize(direction);
}

IEventListener::EventResponse Camera::onEvent(SDL_Event* evt) {
    if (evt->type == SDL_WINDOWEVENT) {
        SDL_WindowEvent *e = &evt->window;

        if (e->event == SDL_WINDOWEVENT_RESIZED
                || e->event == SDL_WINDOWEVENT_SIZE_CHANGED) {

            updateWindowSize();

            return EVT_PROCESSED;
        }
    } else if (evt->type == SDL_MOUSEMOTION) {
        SDL_MouseMotionEvent *e = &evt->motion;

        if ((SDL_BUTTON_LEFT & e->state) && hasFocus()) {
            // TODO: Adjust mouse sensitivity coeficient
            rotation.y -= e->xrel * 0.002;
            rotation.x += e->yrel * 0.002;

            // To prevent horizontal turn
            rotation.x = clamp(rotation.x, -PI_HALF_CLAMP, PI_HALF_CLAMP);
        }

        return EVT_PROCESSED;
    }
    return EVT_IGNORED;
}

bool Camera::hasFocus() {
    return (SDL_GetWindowFlags(window) & SDL_WINDOW_INPUT_FOCUS) != 0;
}

void Camera::step(float time, float delta) {
    // Ignore all input while the window isn't focused (the keyboard state
    // could still contain keys that were held when the focus was lost).
    if (!hasFocus()) {
        movement = ivec3(0);
        return;
    }

    // Poll the current keyboard state every frame instead of tracking
    // key events, so movement always reflects what is held right now.
    const Uint8 *keys = SDL_GetKeyboardState(NULL);

    movement = ivec3(
            keys[SDL_SCANCODE_A] - keys[SDL_SCANCODE_D],
            keys[SDL_SCANCODE_SPACE] - keys[SDL_SCANCODE_C],
            keys[SDL_SCANCODE_W] - keys[SDL_SCANCODE_S]);

    // Avoid huge jumps after a stall (e.g. window being dragged).
    delta = min(delta, 0.1f);

    vec3 direction;
    if (movement.x == 0 && movement.z == 0) {
        direction = movement;
    } else {
        direction = vec3(movement.x, 0, movement.z);
        direction = rotateX(direction, rotation.x);
        direction = rotateY(direction, rotation.y);

        direction.y += movement.y;

        direction = normalize(direction);
    }

    // Hold shift to move faster.
    bool boost = keys[SDL_SCANCODE_LSHIFT] || keys[SDL_SCANCODE_RSHIFT];
    float speed = boost ? 150.0f : 15.0f;
    position += direction * delta * speed;
}

void Camera::render() {
    glViewport(0, 0, windowSize.x, windowSize.y);
}
