#include <climits>
#include <glm/glm.hpp>
#include <string>
#include <glm/gtc/matrix_transform.hpp>

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
    aNormal = glGetAttribLocation(program, "normal");
    aColor = glGetAttribLocation(program, "color");

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, (LANDSCAPE_SIZE + 1) * (LANDSCAPE_SIZE + 1) * sizeof (Vertex), NULL, GL_STREAM_DRAW);

    glEnableVertexAttribArray(aPosition);
    glVertexAttribPointer(aPosition, 3, GL_FLOAT, GL_FALSE, sizeof (Vertex), (GLvoid*) offsetof(Vertex, position));

    glEnableVertexAttribArray(aNormal);
    glVertexAttribPointer(aNormal, 3, GL_FLOAT, GL_FALSE, sizeof (Vertex), (GLvoid*) offsetof(Vertex, normal));

    glEnableVertexAttribArray(aColor);
    glVertexAttribPointer(aColor, 3, GL_UNSIGNED_BYTE, GL_TRUE, sizeof (Vertex), (GLvoid*) offsetof(Vertex, color));

    glGenBuffers(1, &ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, INDEX_COUNT * sizeof (GLuint), NULL, GL_STATIC_DRAW);

    GLuint *eboData = (GLuint*) glMapBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_WRITE_ONLY);

    GLuint *dataPtr = eboData;
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

        dataPtr[0] = UINT_MAX; // Reset index
        dataPtr++;
        //        ctr++;

        //        std::cout << "Element: " << USHRT_MAX << std::endl;

    }

    //    std::cout << "Size: " << ctr << std::endl;
    //    std::cout << "Estimated size: " << INDEX_COUNT << std::endl;

    glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);

    glBindVertexArray(0);

    center = camera->getPosition();

    heightmap = new float[(LANDSCAPE_SIZE + 2) * (LANDSCAPE_SIZE + 2)];

    reloadTerrain();
}

#ifdef WRITE_HEIGHTMAP

void writePGM(int width, int height, unsigned char *data, std::string filename) {

    std::ofstream file(filename.c_str());

    file << "P5" << std::endl;
    file << width << " " << height << std::endl;
    file << "255" << std::endl;

    file.write((char*) data, width * height * sizeof (unsigned char));

    file.close();
}
#endif

void Landscape::reloadTerrain() {

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    Vertex *vboData = (Vertex*) glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);

    vec3 pos = camera->getPosition();
    float *map = heightmap;

    float max = 0;
    for (int row = -1; row <= LANDSCAPE_SIZE; row++) {
        for (int col = -1; col <= LANDSCAPE_SIZE; col++) {
            float x, y, z;

            x = ((row - 1) - (LANDSCAPE_SIZE / 2)) / 2.0f + pos.x;
            z = ((col - 1) - (LANDSCAPE_SIZE / 2)) / 2.0f + pos.z;
            y = 0;

            y += parametrizedNoise(x, z, 4.0 / LANDSCAPE_SIZE, 2.0 / LANDSCAPE_SIZE, 50.0);
            y += parametrizedNoise(x + 7769.0, z + 1103.0, 16.0 / LANDSCAPE_SIZE, 18.0 / LANDSCAPE_SIZE, 5.0);
            y += parametrizedNoise(x - 356.0, z + 32776.0, 64.0 / LANDSCAPE_SIZE, 64.0 / LANDSCAPE_SIZE, 0.5);

            *map = y;
            map++;

            if (y > max) {
                max = y;
            }
        }
    }

#ifdef WRITE_HEIGHTMAP
    unsigned char *_out = new unsigned char[(LANDSCAPE_SIZE + 2) * (LANDSCAPE_SIZE + 2)];
    unsigned char *out = _out;

    map = heightmap;

    for (int row = -1; row <= LANDSCAPE_SIZE; row++) {
        for (int col = -1; col <= LANDSCAPE_SIZE; col++) {
            int i = (row + 1) * (LANDSCAPE_SIZE + 2) + col;
            *out = heightmap[i]*255 / max;
            out++;
        }
    }

    writePGM((LANDSCAPE_SIZE + 2), (LANDSCAPE_SIZE + 2), _out, "heightmap.pgm");

    delete _out;
#endif

    Vertex *dataPtr = vboData;
    for (int row = 0; row <= LANDSCAPE_SIZE; row++) {
        for (int col = 0; col <= LANDSCAPE_SIZE; col++) {
            Vertex *v = dataPtr++;
            vec3 a, b, c, d;

            // Calculate position
            v->position.x = ((row + 1) - (LANDSCAPE_SIZE / 2)) / 2.0f + pos.x;
            v->position.y = heightmap[(row + 1) * (LANDSCAPE_SIZE + 2) + (col + 1)];
            v->position.z = ((col + 1) - (LANDSCAPE_SIZE / 2)) / 2.0f + pos.z;

            a.x = ((row) - (LANDSCAPE_SIZE / 2)) / 2.0f + pos.x;
            a.y = heightmap[(row) * (LANDSCAPE_SIZE + 2) + (col)];
            a.z = ((col) - (LANDSCAPE_SIZE / 2)) / 2.0f + pos.z;

            b.x = ((row + 1) - (LANDSCAPE_SIZE / 2)) / 2.0f + pos.x;
            b.y = heightmap[(row + 1) * (LANDSCAPE_SIZE + 2) + (col)];
            b.z = ((col) - (LANDSCAPE_SIZE / 2)) / 2.0f + pos.z;

            c.x = ((row + 1) - (LANDSCAPE_SIZE / 2)) / 2.0f + pos.x;
            c.y = heightmap[(row + 1) * (LANDSCAPE_SIZE + 2) + (col + 1)];
            c.z = ((col + 1) - (LANDSCAPE_SIZE / 2)) / 2.0f + pos.z;

            d.x = ((row) - (LANDSCAPE_SIZE / 2)) / 2.0f + pos.x;
            d.y = heightmap[(row) * (LANDSCAPE_SIZE + 2) + (col + 1)];
            d.z = ((col + 1) - (LANDSCAPE_SIZE / 2)) / 2.0f + pos.z;

            a = normalize(a - c);
            b = normalize(b - d);

            v->normal = normalize(cross(b, a));

            //            std::cout << "Normal: (" << v->normal.x << ", " << v->normal.y << ", " << v->normal.z << ")" << std::endl;

            // Calculate color
            v->color = u8vec3(0, 80, 0); // Color is green, grass is green, what is nice color it is. (Haiku?)

        }
    }

    glUnmapBuffer(GL_ARRAY_BUFFER);
}

float Landscape::smoothNoise2D(float _x, float _y) {
    int x0 = _x;
    int y0 = _y;

    int x1, y1;

    if (_x >= 0) {
        x1 = x0 + 1;
    } else {
        x1 = x0 - 1;
    }

    if (_y >= 0) {
        y1 = y0 + 1;
    } else {
        y1 = y0 - 1;
    }

    float rx = _x - x0;
    float ry = _y - y0;

    float a0 = 0, a1 = 0, b0 = 0, b1 = 0;

    a0 = noise2D(x0, y0);
    a1 = noise2D(x1, y0);

    b0 = noise2D(x0, y1);
    b1 = noise2D(x1, y1);

    float a = interpolateCos(a0, a1, rx);
    float b = interpolateCos(b0, b1, rx);

    return interpolateCos(a, b, ry);

}

float Landscape::noise2D(int x, int y) {
    x += 338573;
    y += 77313501;
    unsigned int n = ((((x * x) << 3) * 23) + ((y * y) << 1) * 51);

    return (((((n * 3342687 + 1144763) & 0xf2fcf7dd) - 77663544) * -113) * n) / (float) UINT_MAX;
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
    glDrawElements(GL_TRIANGLE_STRIP, INDEX_COUNT, GL_UNSIGNED_INT, 0);
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