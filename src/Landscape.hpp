#pragma once

#include "Camera.hpp"
#include "IRenderer.hpp"
#include "IProcessor.hpp"
#include "RenderShaderProgram.hpp"
#include "RegistrablesContainer.hpp"

namespace pgp {

    using glm::vec3;
    using glm::mat4;

    /**
     * Procedural terrain rendered as a set of nested grids (a simple geometry
     * clipmap). Heights are computed on the GPU in the vertex shader.
     */
    class Landscape : public IRenderer, public IProcessor, public IEventListener, public RegistrablesContainer {
    private:
        Camera *camera;
        RenderShaderProgram renderProgram;
        GLuint vao, ebo;
        GLuint fbo, colTex, depTex;
        GLuint rbo;
        GLint uView, uProjection;
        GLint uEyePosition, uSunDirection;
        GLint uLevelOrigin, uSpacing, uGridSize, uDiscardRect;
        GLenum polygonMode;
        GLsizei indexCount;
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

        inline vec3 getSunDirection() {
            return glm::normalize(vec3(0.55, 0.38, 0.75));
        }

        mat4 getProjectionMatrix();
        mat4 getViewMatrix();

        virtual void render();

        virtual void step(float time, float delta);

        virtual IEventListener::EventResponse onEvent(SDL_Event* evt);
    };

}
