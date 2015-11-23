#pragma once

#include "Camera.hpp"
#include "IRenderer.hpp"
#include "IProcessor.hpp"
#include "RenderShaderProgram.hpp"
#include "RegistrablesContainer.hpp"

namespace pgp {

    using glm::vec3;

    class Landscape : public IRenderer, public IProcessor, public IEventListener, public RegistrablesContainer {
    private:
        Camera *camera;
        RenderShaderProgram renderProgram;
        GLuint vao, vbo, ebo;
        GLint uView, uProjection;
        GLint aPosition, aColor;
        GLenum polygonMode;
        vec3 center;
    public:
        Landscape(Camera *camera);

        void reloadTerrain();

        virtual void render();

        virtual void step(float dt);

        virtual IEventListener::EventResponse onEvent(SDL_Event* evt);

    };

}
