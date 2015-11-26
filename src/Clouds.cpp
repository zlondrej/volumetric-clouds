#include "Clouds.hpp"

using namespace pgp;

#define DIV_ROUND_UP(x,d) ((x + d - 1)/d)

Clouds::Clouds(Camera *cam, Landscape *land) {
    string computeShaderFile("./shaders/clouds.comp");

    camera = cam;
    landscape = land;

    computeProgram.setComputeShaderFromFile(computeShaderFile);

    GLuint program = computeProgram.getProgram();

    uColor = glGetUniformLocation(program, "color");
    uDepth = glGetUniformLocation(program, "depth");

    uPosition = glGetUniformLocation(program, "position");
    uTime = glGetUniformLocation(program, "time");

    uScreenSize = glGetUniformLocation(program, "screenSize");

}

Clouds::~Clouds() {
}

void Clouds::render() {

    glm::ivec2 ws = camera->getWindowSize();
    glm::vec3 pos = camera->getPosition();

    glUseProgram(computeProgram.getProgram());

    glBindFramebuffer(GL_FRAMEBUFFER, landscape->getFramebuffer());
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, 0, 0);

    glBindImageTexture(0, landscape->getColorTexture(), 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
    glUniform1i(uColor, 0);

    glBindImageTexture(1, landscape->getDepthTexture(), 0, GL_FALSE, 0, GL_READ_ONLY, GL_R32F);
    glUniform1i(uDepth, 1);

    glUniform3fv(uPosition, 1, &pos[0]);
    glUniform1f(uTime, 0.0f);

    glUniform2iv(uScreenSize, 1, &ws[0]);

    glDispatchCompute(DIV_ROUND_UP(ws.x, 8), DIV_ROUND_UP(ws.y, 8), 1);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, landscape->getColorTexture(), 0);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, landscape->getFramebuffer());

    glBlitFramebuffer(0, 0, ws.x, ws.y, 0, 0, ws.x, ws.y, GL_COLOR_BUFFER_BIT, GL_LINEAR);
}
