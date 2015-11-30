#pragma once

#include "Camera.hpp"
#include "IRenderer.hpp"
#include "IProcessor.hpp"
#include "RenderShaderProgram.hpp"
#include "RegistrablesContainer.hpp"

namespace pgp {

    using glm::vec3;
    using glm::mat4;

    class Landscape : public IRenderer, public IProcessor, public IEventListener, public RegistrablesContainer {
    private:
        Camera *camera;
        RenderShaderProgram renderProgram;
        GLuint vao, vbo, ebo;
        GLuint fbo, colTex, depTex;
        GLuint rbo;
        GLint uView, uProjection;
        GLint uEyePosition;
        GLint aPosition, aNormal, aColor;
        GLenum polygonMode;
        vec3 center;
        float *heightmap;
    public:
        Landscape(Camera *camera);

        ~Landscape();

        inline GLuint getColorTexture() {
            return colTex;
        }

        inline GLuint getDepthTexture() {
            return depTex;
        }

        inline GLuint getFramebuffer() {
            return fbo;
        }

        mat4 getProjectionMatrix();
        mat4 getViewMatrix();

        virtual void render();

        virtual void step(float time, float delta);

        virtual IEventListener::EventResponse onEvent(SDL_Event* evt);

    private:

        static inline float parametrizedNoise(float x, float y, float xPeriod, float yPeriod, float amplitude) {
            return smoothNoise2D(x * xPeriod, y * yPeriod) * amplitude;
        }

        static float smoothNoise2D(float x, float y);

        static float noise2D(int x, int y);

        static float interpolateCos(float a, float b, float factor);

        void reloadTerrain();

    };

}
