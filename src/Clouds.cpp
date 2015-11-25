#include "Clouds.hpp"

using namespace pgp;

Clouds::Clouds(Camera *cam, Landscape *land) {
    camera = cam;
    landscape = land;
}


Clouds::~Clouds() {
}

void Clouds::render() {

    glm::ivec2 ws = camera->getWindowSize();

    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, landscape->getFramebuffer());

    glBlitFramebuffer(0,0,ws.x, ws.y, 0,0,ws.x, ws.y, GL_COLOR_BUFFER_BIT, GL_LINEAR);
}
