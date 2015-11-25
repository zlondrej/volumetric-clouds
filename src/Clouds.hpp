#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>

#include "IRenderer.hpp"
#include "Camera.hpp"
#include "Landscape.hpp"

namespace pgp {

    class Clouds : public IRenderer {
        Camera *camera;
        Landscape *landscape;
    public:
        Clouds(Camera *camera, Landscape *landscape);
        ~Clouds();

        virtual void render();

    private:

    };

}