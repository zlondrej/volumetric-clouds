#include "Clouds.hpp"
#include "Exceptions.hpp"

#include <glm/gtc/type_ptr.hpp>
#include <iostream>

using namespace pgp;
using namespace glm;

#define DOWNSCALE 4

//#define USE_QUAD_BLIT

#define DIV_ROUND_UP(x,d) ((x + d - 1)/d)

static float verticies[] = {
    -1.0, -1.0,
    1.0, -1.0,
    1.0, 1.0,
    -1.0, 1.0,
};

static GLuint indicies[] = {
    0, 1, 2, 3
};

static string computeShaderFile("./shaders/clouds.comp");
static string blitVertexShaderFile("./shaders/blit.vert");
static string blitFragmentShaderFile("./shaders/blit.frag");

Clouds::Clouds(Camera *cam, Landscape *land) {

    camera = cam;
    landscape = land;

    computeProgram = new ComputeShaderProgram();

    computeProgram->setComputeShaderFromFile(computeShaderFile);

    GLuint program = computeProgram->getProgram();

    ivec2 windowSize = camera->getWindowSize()/DOWNSCALE;

    initComputeUniforms(program);

    glGenTextures(1, &cloudTexture);
    glBindTexture(GL_TEXTURE_2D, cloudTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, windowSize.x, windowSize.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glGenTextures(1, &cloudDepthTexture);
    glBindTexture(GL_TEXTURE_2D, cloudDepthTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, windowSize.x, windowSize.y, 0, GL_RED, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    blitProgram.setVertexShaderFromFile(blitVertexShaderFile);
    blitProgram.setFragmenShaderFromFile(blitFragmentShaderFile);

    program = blitProgram.getProgram();

    aBlitPosition = glGetAttribLocation(program, "position");
    uBlitTexture = glGetUniformLocation(program, "texture");

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof (verticies), verticies, GL_STATIC_DRAW);

    glEnableVertexAttribArray(aBlitPosition);
    glVertexAttribPointer(aBlitPosition, 2, GL_FLOAT, GL_FALSE, 0, NULL);

    glGenBuffers(1, &ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof (indicies), indicies, GL_STATIC_DRAW);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

}

void Clouds::initComputeUniforms(GLuint program) {
  uColor = glGetUniformLocation(program, "colorIm");
  uDepth = glGetUniformLocation(program, "depthIm");
  uCloud = glGetUniformLocation(program, "cloudIm");
  uCloudDepth = glGetUniformLocation(program, "cloudDepthIm");

  uPosition = glGetUniformLocation(program, "eyePosition");
  uSunPosition = glGetUniformLocation(program, "sunPosition");
  uSunColor = glGetUniformLocation(program, "sunColor");

  uTime = glGetUniformLocation(program, "time");

  uInvVP = glGetUniformLocation(program, "invVP");
}

Clouds::~Clouds() {

    glDeleteBuffers(1, &vbo);
    glDeleteBuffers(1, &ebo);

    glDeleteVertexArrays(1, &vao);

    glDeleteTextures(1, &cloudTexture);
    glDeleteTextures(1, &cloudDepthTexture);

    delete computeProgram;
}

void Clouds::render() {

    ivec2 ws = camera->getWindowSize();
    ivec2 dws = ws/DOWNSCALE;
    vec3 pos = camera->getPosition();

    mat4 viewMat = landscape->getViewMatrix();
    mat4 projMat = landscape->getProjectionMatrix();

    mat4 invVPMat = glm::inverse(projMat*viewMat);

    glUseProgram(computeProgram->getProgram());

    glBindImageTexture(0, landscape->getColorTexture(), 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA8);
    glUniform1i(uColor, 0);

    glBindImageTexture(1, landscape->getDepthTexture(), 0, GL_FALSE, 0, GL_READ_ONLY, GL_R32F);
    glUniform1i(uDepth, 1);

    glBindImageTexture(2, cloudTexture, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA8);
    glUniform1i(uCloud, 2);

    glBindImageTexture(3, cloudDepthTexture, 0, GL_FALSE, 0, GL_READ_WRITE, GL_R32F);
    glUniform1i(uCloudDepth, 3);

    glUniform3fv(uPosition, 1, &pos[0]);
    glUniform1f(uTime, time*10);

    glUniformMatrix4fv(uInvVP, 1, GL_FALSE, glm::value_ptr(invVPMat));

    glDispatchCompute(DIV_ROUND_UP(dws.x, 16), DIV_ROUND_UP(dws.y, 4), 1);

#ifdef USE_QUAD_BLIT

    glUseProgram(blitProgram.getProgram());

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, landscape->getColorTexture());

    glUniform1i(uBlitTexture, 0);

    glBindVertexArray(vao);

    glDisable(GL_DEPTH_TEST);

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glDrawElements(GL_TRIANGLE_STRIP, 6, GL_UNSIGNED_INT, NULL);

#else

    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, landscape->getFramebuffer());

    glBlitFramebuffer(0, 0, ws.x, ws.y, 0, 0, ws.x, ws.y, GL_COLOR_BUFFER_BIT, GL_LINEAR);

#endif
}

void Clouds::step(float _time, float) {
    time = _time;
}

IEventListener::EventResponse Clouds::onEvent(SDL_Event *evt) {
    if (evt->type == SDL_KEYDOWN) {
        SDL_KeyboardEvent *e = &evt->key;

        if (e->keysym.sym == SDLK_r) {
            ComputeShaderProgram *p = new ComputeShaderProgram;

            try {
                p->setComputeShaderFromFile(computeShaderFile);

                GLuint program = computeProgram->getProgram();
                delete computeProgram;
                computeProgram = p;
                initComputeUniforms(program);

                std::cerr << "Cloud shader reloaded" << std::endl;
            }
            catch (Exception &e) {
                std::cerr << e.getMessage() << std::endl;
                delete p;
            }

            return EVT_PROCESSED;
        }
    }

    return EVT_IGNORED;
}
