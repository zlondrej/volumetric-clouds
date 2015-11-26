#include "Clouds.hpp"

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

Clouds::Clouds(Camera *cam, Landscape *land) {
    string computeShaderFile("./shaders/clouds.comp");
    string blitVertexShaderFile("./shaders/blit.vert");
    string blitFragmentShaderFile("./shaders/blit.frag");

    camera = cam;
    landscape = land;

    computeProgram.setComputeShaderFromFile(computeShaderFile);

    GLuint program = computeProgram.getProgram();

    uColor = glGetUniformLocation(program, "color");
    uDepth = glGetUniformLocation(program, "depth");

    uPosition = glGetUniformLocation(program, "position");
    uTime = glGetUniformLocation(program, "time");

    uScreenSize = glGetUniformLocation(program, "screenSize");

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

Clouds::~Clouds() {


    glDeleteBuffers(1, &vbo);
    glDeleteBuffers(1, &ebo);

    glDeleteVertexArrays(1, &vao);
}

void Clouds::render() {

    ivec2 ws = camera->getWindowSize();
    vec3 pos = camera->getPosition();

    glUseProgram(computeProgram.getProgram());

    glBindImageTexture(0, landscape->getColorTexture(), 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
    glUniform1i(uColor, 0);

    glBindImageTexture(1, landscape->getDepthTexture(), 0, GL_FALSE, 0, GL_READ_ONLY, GL_R32F);
    glUniform1i(uDepth, 1);

    glUniform3fv(uPosition, 1, &pos[0]);
    glUniform1f(uTime, 0.0f);

    glUniform2iv(uScreenSize, 1, &ws[0]);

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
