#pragma once

#include "Camera.hpp"
#include "IRenderer.hpp"
#include "IProcessor.hpp"
#include "RenderShaderProgram.hpp"
#include "RegistrablesContainer.hpp"

namespace pgp {

    class Landscape : public IRenderer, public IProcessor, public RegistrablesContainer {
    private:
        Camera *camera;
        RenderShaderProgram renderProgram;
        GLuint vao, vbo, ebo;
        GLint uView, uProjection;
        GLint aPosition, aColor;
    public:
        Landscape(Camera *camera);

        virtual void render();

        virtual void step(float dt);


    };

}
