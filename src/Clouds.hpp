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
        RenderShaderProgram *blendProgram;

        GLint uDepth, uNoise, uWeather, uCloud, uCloudDepth;
        GLint uPosition, uTime, uSunDirection, uCoverage;
        GLint uInvVP;

        GLint uBlendSceneColor, uBlendSceneDepth;
        GLint uBlendCloudColor, uBlendCloudDepth;
        GLint uBlendNoise, uBlendWeather, uBlendPosition, uBlendInvVP;
        GLint uBlendTime, uBlendSunDirection, uBlendCoverage;

        GLuint cloudTexture, cloudDepthTexture, noiseTexture, weatherTexture;
        GLuint vao;

        float time;
        float coverage;

    public:
        Clouds(Camera *camera, Landscape *landscape);
        ~Clouds();

        virtual void render();
        virtual void step(float, float);
        virtual IEventListener::EventResponse onEvent(SDL_Event *evt);

    private:
        void loadShaders();
        void allocateTextures();
        void generateNoise();
        void generateWeather();
    };

}
