#include "Clouds.hpp"
#include "Exceptions.hpp"

#include <glm/gtc/type_ptr.hpp>
#include <iostream>

using namespace pgp;
using namespace glm;

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

    initComputeUniforms(program);

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
  uColor = glGetUniformLocation(program, "color");
  uDepth = glGetUniformLocation(program, "depth");

  uPosition = glGetUniformLocation(program, "eyePosition");
  uSunPosition = glGetUniformLocation(program, "sunPosition");
  uSunColor = glGetUniformLocation(program, "sunColor");

  uTime = glGetUniformLocation(program, "time");
  uScreenSize = glGetUniformLocation(program, "screenSize");

  uInvVP = glGetUniformLocation(program, "invVP");


}

Clouds::~Clouds() {

    glDeleteBuffers(1, &vbo);
    glDeleteBuffers(1, &ebo);

    glDeleteVertexArrays(1, &vao);

    delete computeProgram;
}

void Clouds::render() {

    ivec2 ws = camera->getWindowSize();
    vec3 pos = camera->getPosition();

    mat4 viewMat = landscape->getViewMatrix();
    mat4 projMat = landscape->getProjectionMatrix();

    mat4 invVPMat = glm::inverse(projMat*viewMat);

    glUseProgram(computeProgram->getProgram());

    glBindImageTexture(0, landscape->getColorTexture(), 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA8);
    glUniform1i(uColor, 0);

    glBindImageTexture(1, landscape->getDepthTexture(), 0, GL_FALSE, 0, GL_READ_ONLY, GL_R32F);
    glUniform1i(uDepth, 1);

    glUniform3fv(uPosition, 1, &pos[0]);
    glUniform1f(uTime, time*10);

    glUniform2iv(uScreenSize, 1, &ws[0]);

    glUniformMatrix4fv(uInvVP, 1, GL_FALSE, glm::value_ptr(invVPMat));

    glDispatchCompute(DIV_ROUND_UP(ws.x, 8), DIV_ROUND_UP(ws.y, 8), 1);

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
