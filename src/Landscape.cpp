#include <climits>
#include <glm/glm.hpp>
#include <string>
#include <glm/gtc/matrix_transform.hpp>

//#include <iostream>

#include "Landscape.hpp"

#define LANDSCAPE_SIZE 250
#define INDEX_COUNT (2 * (LANDSCAPE_SIZE + 1) * LANDSCAPE_SIZE + LANDSCAPE_SIZE)

using namespace pgp;

using glm::u8vec3;
using glm::vec3;

typedef struct {
    vec3 position;
    vec3 normal;
    u8vec3 color;
} Vertex;

Landscape::Landscape(Camera *_camera) : camera(_camera), vao(0), vbo(0), ebo(0), polygonMode(GL_FILL) {
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

    aPosition = glGetAttribLocation(program, "position");
    aColor = glGetAttribLocation(program, "color");

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, (LANDSCAPE_SIZE + 1) * (LANDSCAPE_SIZE + 1) * sizeof (Vertex), NULL, GL_STREAM_DRAW);

    glEnableVertexAttribArray(aPosition);
    glVertexAttribPointer(aPosition, 3, GL_FLOAT, GL_FALSE, sizeof (Vertex), offsetof(Vertex, position));

    // Normal attribute

    glEnableVertexAttribArray(aColor);
    glVertexAttribPointer(aColor, 3, GL_UNSIGNED_BYTE, GL_TRUE, sizeof (Vertex), (GLvoid*) offsetof(Vertex, color));

    glGenBuffers(1, &ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, INDEX_COUNT * sizeof (GLushort), NULL, GL_STATIC_DRAW);

    GLushort *eboData = (GLushort*) glMapBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_WRITE_ONLY);

    GLushort *dataPtr = eboData;
    //    int ctr = 0;
    for (int row = 0; row < LANDSCAPE_SIZE; row++) {
        for (int col = 0; col <= LANDSCAPE_SIZE; col++) {
            int i = (row + 1) * (LANDSCAPE_SIZE + 1) + col;
            int j = row * (LANDSCAPE_SIZE + 1) + col;

            dataPtr[0] = i;
            dataPtr[1] = j;
            dataPtr += 2;
            //            ctr += 2;

            //            std::cout << "Element: " << i << std::endl;
            //            std::cout << "Element: " << j << std::endl;
        }

        dataPtr[0] = USHRT_MAX; // Reset index
        dataPtr++;
        //        ctr++;

        //        std::cout << "Element: " << USHRT_MAX << std::endl;

    }

    //    std::cout << "Size: " << ctr << std::endl;
    //    std::cout << "Estimated size: " << INDEX_COUNT << std::endl;

    glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);

    glBindVertexArray(0);

    center = camera->getPosition();
    reloadTerrain();
}

void Landscape::reloadTerrain() {

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    Vertex *vboData = (Vertex*) glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);

    vec3 pos = camera->getPosition();

    Vertex *dataPtr = vboData;
    for (int row = 0; row <= LANDSCAPE_SIZE; row++) {
        for (int col = 0; col <= LANDSCAPE_SIZE; col++) {
            Vertex *v = dataPtr++;


            v->position.x = (row - (LANDSCAPE_SIZE / 2)) / 2.0f + pos.x;
            v->position.z = (col - (LANDSCAPE_SIZE / 2)) / 2.0f + pos.z;

            v->position.y = smoothNoise2D(v->position.x / 47.4, v->position.z / -57.3) * 15.0;

            v->color.x = (sin(v->position.x / 1.3f) / 2.0f + 0.5f) * 255;
            v->color.z = (sin(v->position.z / 0.7f + 12.35f) / 2.0f + 0.5f) * 255;
            v->color.y = (v->color.x + (int) v->color.z) / 2;

        }
    }

    glUnmapBuffer(GL_ARRAY_BUFFER);
}

float Landscape::smoothNoise2D(float _x, float _y) {
    int x0 = _x;
    int y0 = _y;
    int x1 = x0 + 1;
    int y1 = y0 + 1;

    float rx = _x - x0;
    float ry = _y - y0;


    float a0 = 0, a1 = 0, b0 = 0, b1 = 0;

    for (int i = 1; i <= 4; i++) {
        a0 += noise2D(x0 * i, y0 * i);
        a1 += noise2D(x1 * i, y0 * i);

        b0 += noise2D(x0 * i, y1 * i);
        b1 += noise2D(x1 * i, y1 * i);
    }

    a0 /= 4.0;
    a1 /= 4.0;
    b0 /= 4.0;
    b1 /= 4.0;

    float a = interpolateCos(a0, a1, rx);
    float b = interpolateCos(b0, b1, rx);

    return interpolateCos(a, b, ry);

}

float Landscape::noise2D(int x, int y) {
    x += 338573;
    y += 77313501;
    int n = (((x * x << 3) * 23) + (y * y << 1) * 51) * x * y;

    return (((((n * 3342687 + 1144763) & 0xf2fcf7dd) - 77663544) * -113) * n & 0x7fffffff) / (float) INT_MAX;
}

float Landscape::interpolateCos(float a, float b, float factor) {
    factor = (1 - cos(factor * glm::pi<float>())) * 0.5;

    return a * (1 - factor) + b * factor;
}

void Landscape::render() {

    runRenderers();

    glUseProgram(renderProgram.getProgram());

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    glClearColor(0.0, 0.7, 1.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    vec3 cameraPosition = camera->getPosition();
    vec3 atPosition = cameraPosition + camera->getViewVector();

    //    std::cout << "Eye: (" << cameraPosition.x << ", " << cameraPosition.y << ", " << cameraPosition.z << ")" << std::endl;
    //    std::cout << "At: (" << atPosition.x << ", " << atPosition.y << ", " << atPosition.z << ")" << std::endl;

    vec2 windowSize = camera->getWindowSize();
    float fov = radians(75.0);
    float aspect = windowSize.x / windowSize.y;
    float near = 0.001;
    float far = 1e5;

    mat4 viewMat = glm::lookAt(cameraPosition, atPosition, vec3(0, 1, 0));
    mat4 projMat = glm::perspective(fov, aspect, near, far);

    glUniformMatrix4fv(uView, 1, GL_FALSE, (GLfloat*) & viewMat);
    glUniformMatrix4fv(uProjection, 1, GL_FALSE, (GLfloat*) & projMat);

    glBindVertexArray(vao);

    glPolygonMode(GL_FRONT_AND_BACK, polygonMode);

    glEnable(GL_PRIMITIVE_RESTART_FIXED_INDEX);
    glDrawElements(GL_TRIANGLE_STRIP, INDEX_COUNT, GL_UNSIGNED_SHORT, 0);
    glDisable(GL_PRIMITIVE_RESTART_FIXED_INDEX);

    glBindVertexArray(0);
}

void Landscape::step(float dt) {
    if (distance(center, camera->getPosition()) > 15.0) {
        reloadTerrain();
        center = camera->getPosition();
    }
}

IEventListener::EventResponse Landscape::onEvent(SDL_Event* evt) {
    if (evt->type == SDL_KEYDOWN) {
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