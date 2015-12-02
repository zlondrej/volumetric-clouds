#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>

#include "IRenderer.hpp"
#include "IProcessor.hpp"
#include "IEventListener.hpp"
#include "Camera.hpp"
#include "Landscape.hpp"
#include "ComputeShaderProgram.hpp"

namespace pgp {

    class Clouds : public IRenderer, public IProcessor, public IEventListener {
        Camera *camera;
        Landscape *landscape;
        ComputeShaderProgram *computeProgram;
        RenderShaderProgram blitProgram;
        GLuint uColor, uDepth, uCloud, uCloudDepth;
        GLuint uPosition, uTime;
        GLuint uSunPosition, uSunColor;
        GLuint uInvVP;

        GLuint cloudTexture, cloudDepthTexture;

        GLuint aBlitPosition;
        GLuint uBlitTexture;
        GLuint vao, vbo, ebo;

        float time;
    public:
        Clouds(Camera *camera, Landscape *landscape);
        ~Clouds();

        virtual void render();
        virtual void step(float, float);
        virtual IEventListener::EventResponse onEvent(SDL_Event *evt);

    private:
        void initComputeUniforms(GLuint program);
    };

}
