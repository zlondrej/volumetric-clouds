#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>

#include "IRenderer.hpp"
#include "Camera.hpp"
#include "Landscape.hpp"
#include "ComputeShaderProgram.hpp"

namespace pgp {

    class Clouds : public IRenderer {
        Camera *camera;
        Landscape *landscape;
        ComputeShaderProgram computeProgram;
        RenderShaderProgram blitProgram;
        GLuint uColor, uDepth;
        GLuint uPosition, uTime;
        GLuint uScreenSize;
        GLuint uInvVP;

        GLuint aBlitPosition;
        GLuint uBlitTexture;
        GLuint vao, vbo, ebo;
    public:
        Clouds(Camera *camera, Landscape *landscape);
        ~Clouds();

        virtual void render();

    private:

    };

}
