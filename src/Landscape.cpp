#include <cmath>
#include <string>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Landscape.hpp"

// Each level is a GRID_SIZE x GRID_SIZE quad grid. Level l has vertex spacing
// BASE_SPACING * 2^l, so it covers twice the area of the previous level.
#define GRID_SIZE 128
#define LEVELS 8
#define BASE_SPACING 1.0f

using namespace pgp;

using glm::vec2;
using glm::vec3;
using glm::vec4;

Landscape::Landscape(Camera *_camera) : camera(_camera), vao(0), ebo(0), polygonMode(GL_FILL) {
    string vertexShaderFile("./shaders/landscape.vert");
    string fragmentShaderFile("./shaders/landscape.frag");

    registerRenderer(camera);

    renderProgram.setVertexShaderFromFile(vertexShaderFile);
    renderProgram.setFragmenShaderFromFile(fragmentShaderFile);

    GLuint program = renderProgram.getProgram();
    if (program == 0) {
        throw string("Could not compile render program.");
    }

    uProjection = glGetUniformLocation(program, "projection");
    uView = glGetUniformLocation(program, "view");
    uEyePosition = glGetUniformLocation(program, "eyePosition");
    uSunDirection = glGetUniformLocation(program, "sunDirection");
    uLevelOrigin = glGetUniformLocation(program, "levelOrigin");
    uSpacing = glGetUniformLocation(program, "spacing");
    uGridSize = glGetUniformLocation(program, "gridSize");
    uDiscardRect = glGetUniformLocation(program, "discardRect");

    ivec2 windowSize = camera->getWindowSize();

    glGenTextures(1, &colTex);
    glGenTextures(1, &depTex);

    glBindTexture(GL_TEXTURE_2D, colTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, windowSize.x, windowSize.y, 0, GL_RGBA, GL_HALF_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glBindTexture(GL_TEXTURE_2D, depTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, windowSize.x, windowSize.y, 0, GL_RED, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glBindTexture(GL_TEXTURE_2D, 0);

    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    glGenRenderbuffers(1, &rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT32F, windowSize.x, windowSize.y);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colTex, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, depTex, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rbo);

    GLenum drawBuffers[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
    glDrawBuffers(2, drawBuffers);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Grid indices. Vertex positions are derived from gl_VertexID in the
    // vertex shader, so no vertex buffer is needed.
    std::vector<GLuint> indices;
    indices.reserve(GRID_SIZE * GRID_SIZE * 6);
    const GLuint rowLength = GRID_SIZE + 1;
    for (GLuint row = 0; row < GRID_SIZE; row++) {
        for (GLuint col = 0; col < GRID_SIZE; col++) {
            GLuint i = row * rowLength + col;
            indices.push_back(i);
            indices.push_back(i + rowLength);
            indices.push_back(i + 1);

            indices.push_back(i + 1);
            indices.push_back(i + rowLength);
            indices.push_back(i + rowLength + 1);
        }
    }
    indexCount = indices.size();

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glGenBuffers(1, &ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof (GLuint), indices.data(), GL_STATIC_DRAW);

    glBindVertexArray(0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

Landscape::~Landscape() {
    glDeleteFramebuffers(1, &fbo);

    glDeleteRenderbuffers(1, &rbo);

    glDeleteTextures(1, &colTex);
    glDeleteTextures(1, &depTex);

    glDeleteBuffers(1, &ebo);

    glDeleteVertexArrays(1, &vao);
}

mat4 Landscape::getProjectionMatrix() {

      vec2 windowSize = camera->getWindowSize();
      float fov = radians(75.0);
      float aspect = windowSize.x / windowSize.y;
      float near = 0.5;
      float far = 3e4;

      return glm::perspective(fov, aspect, near, far);
}

mat4 Landscape::getViewMatrix() {

      vec3 cameraPosition = camera->getPosition();
      vec3 atPosition = cameraPosition + camera->getViewVector();

      return glm::lookAt(cameraPosition, atPosition, vec3(0, 1, 0));
}

void Landscape::render() {

    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    runRenderers();

    glUseProgram(renderProgram.getProgram());

    glEnable(GL_DEPTH_TEST);
    // Not worth getting the winding right for a height field seen from above.
    glDisable(GL_CULL_FACE);

    glClearColor(0.0, 0.0, 0.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    float depClear = 1e15;
    glClearTexImage(depTex, 0, GL_RED, GL_FLOAT, &depClear);

    mat4 viewMat = getViewMatrix();
    mat4 projMat = getProjectionMatrix();

    vec3 camPos = camera->getPosition();
    vec3 sunDir = getSunDirection();

    glUniformMatrix4fv(uView, 1, GL_FALSE, (GLfloat*) & viewMat);
    glUniformMatrix4fv(uProjection, 1, GL_FALSE, (GLfloat*) & projMat);
    glUniform3fv(uEyePosition, 1, &camPos[0]);
    glUniform3fv(uSunDirection, 1, &sunDir[0]);
    glUniform1i(uGridSize, GRID_SIZE);

    glBindVertexArray(vao);

    glPolygonMode(GL_FRONT_AND_BACK, polygonMode);

    // Nothing to discard on the finest level.
    vec4 discardRect(1, 1, -1, -1);
    float spacing = BASE_SPACING;

    for (int level = 0; level < LEVELS; level++) {
        // Snap each level to twice its spacing, so its vertices always lie on
        // the same world positions (no "swimming") and its edges align with
        // the vertices of the next coarser level.
        float snap = 2.0f * spacing;
        vec2 origin = glm::floor(vec2(camPos.x, camPos.z) / snap) * snap;

        glUniform2fv(uLevelOrigin, 1, &origin[0]);
        glUniform1f(uSpacing, spacing);
        glUniform4fv(uDiscardRect, 1, &discardRect[0]);

        glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);

        // The next level skips the area covered by this one, except for a
        // one-cell overlap to avoid cracks between the levels.
        float half = GRID_SIZE / 2 * spacing;
        float margin = 2.0f * spacing;
        discardRect = vec4(origin - half + margin, origin + half - margin);

        spacing *= 2.0f;
    }

    glBindVertexArray(0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Landscape::step(float time, float delta) {
}

IEventListener::EventResponse Landscape::onEvent(SDL_Event* evt) {

    if (evt->type == SDL_WINDOWEVENT) {
        SDL_WindowEvent *e = &evt->window;

        if (e->event == SDL_WINDOWEVENT_RESIZED
                || e->event == SDL_WINDOWEVENT_SIZE_CHANGED) {

            ivec2 windowSize = camera->getWindowSize();

            glBindTexture(GL_TEXTURE_2D, colTex);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, windowSize.x, windowSize.y, 0, GL_RGBA, GL_HALF_FLOAT, NULL);

            glBindTexture(GL_TEXTURE_2D, depTex);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, windowSize.x, windowSize.y, 0, GL_RED, GL_FLOAT, NULL);

            glBindRenderbuffer(GL_RENDERBUFFER, rbo);
            glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT32F, windowSize.x, windowSize.y);

            glBindTexture(GL_TEXTURE_2D, 0);
            glBindFramebuffer(GL_FRAMEBUFFER, 0);

            return EVT_PROCESSED;
        }
    } else if (evt->type == SDL_KEYDOWN) {
        SDL_KeyboardEvent *e = &evt->key;

        if (e->keysym.sym == SDLK_p) {
            switch (polygonMode) {
                case GL_FILL:
                    polygonMode = GL_LINE;
                    break;
                case GL_LINE:
                    polygonMode = GL_POINT;
                    break;
                case GL_POINT:
                    polygonMode = GL_FILL;
                    break;
            }
            return EVT_PROCESSED;
        }
    }

    return EVT_IGNORED;
}
