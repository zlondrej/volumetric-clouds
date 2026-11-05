#include "Clouds.hpp"
#include "Exceptions.hpp"

#include <glm/gtc/type_ptr.hpp>
#include <iostream>

using namespace pgp;
using namespace glm;

// Clouds are ray-marched at 1/DOWNSCALE of the window resolution.
#define DOWNSCALE 2
#define NOISE_SIZE 128
#define WEATHER_SIZE 512

#define DIV_ROUND_UP(x,d) ((x + d - 1)/d)

static string computeShaderFile("./shaders/clouds.comp");
static string noiseShaderFile("./shaders/noise.comp");
static string weatherShaderFile("./shaders/weather.comp");
static string blendVertexShaderFile("./shaders/blend.vert");
static string blendFragmentShaderFile("./shaders/blend.frag");

static void setupTexture2D(GLuint texture) {
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}

Clouds::Clouds(Camera *cam, Landscape *land) :
    camera(cam), landscape(land), computeProgram(NULL), blendProgram(NULL),
    time(0), coverage(0.55) {

    loadShaders();

    if (computeProgram == NULL || blendProgram == NULL) {
        throw Exception("Could not compile cloud shaders.");
    }

    glGenTextures(1, &cloudTexture);
    setupTexture2D(cloudTexture);

    glGenTextures(1, &cloudDepthTexture);
    setupTexture2D(cloudDepthTexture);

    allocateTextures();

    generateNoise();
    generateWeather();

    // Empty VAO for the attribute-less full screen triangle.
    glGenVertexArrays(1, &vao);
}

void Clouds::loadShaders() {
    ComputeShaderProgram *compute = new ComputeShaderProgram();
    RenderShaderProgram *blend = new RenderShaderProgram();

    try {
        compute->setComputeShaderFromFile(computeShaderFile);
        compute->getProgram();

        blend->setVertexShaderFromFile(blendVertexShaderFile);
        blend->setFragmenShaderFromFile(blendFragmentShaderFile);
        blend->getProgram();
    } catch (Exception &e) {
        std::cerr << e.getMessage() << std::endl;
        delete compute;
        delete blend;
        return;
    }

    delete computeProgram;
    delete blendProgram;
    computeProgram = compute;
    blendProgram = blend;

    GLuint program = computeProgram->getProgram();

    uDepth = glGetUniformLocation(program, "depthTex");
    uNoise = glGetUniformLocation(program, "noiseTex");
    uWeather = glGetUniformLocation(program, "weatherTex");
    uCloud = glGetUniformLocation(program, "cloudIm");
    uCloudDepth = glGetUniformLocation(program, "cloudDepthIm");
    uPosition = glGetUniformLocation(program, "eyePosition");
    uSunDirection = glGetUniformLocation(program, "sunDirection");
    uTime = glGetUniformLocation(program, "time");
    uCoverage = glGetUniformLocation(program, "coverage");
    uInvVP = glGetUniformLocation(program, "invVP");

    program = blendProgram->getProgram();

    uBlendSceneColor = glGetUniformLocation(program, "sceneColor");
    uBlendSceneDepth = glGetUniformLocation(program, "sceneDepth");
    uBlendCloudColor = glGetUniformLocation(program, "cloudColor");
    uBlendCloudDepth = glGetUniformLocation(program, "cloudDepth");
    uBlendNoise = glGetUniformLocation(program, "noiseTex");
    uBlendWeather = glGetUniformLocation(program, "weatherTex");
    uBlendPosition = glGetUniformLocation(program, "eyePosition");
    uBlendInvVP = glGetUniformLocation(program, "invVP");
    uBlendTime = glGetUniformLocation(program, "time");
    uBlendSunDirection = glGetUniformLocation(program, "sunDirection");
    uBlendCoverage = glGetUniformLocation(program, "coverage");
}

void Clouds::allocateTextures() {
    ivec2 size = DIV_ROUND_UP(camera->getWindowSize(), DOWNSCALE);

    glBindTexture(GL_TEXTURE_2D, cloudTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, size.x, size.y, 0, GL_RGBA, GL_HALF_FLOAT, NULL);

    glBindTexture(GL_TEXTURE_2D, cloudDepthTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, size.x, size.y, 0, GL_RED, GL_FLOAT, NULL);

    glBindTexture(GL_TEXTURE_2D, 0);
}

void Clouds::generateNoise() {
    ComputeShaderProgram noiseProgram;
    noiseProgram.setComputeShaderFromFile(noiseShaderFile);
    GLuint program = noiseProgram.getProgram();

    glGenTextures(1, &noiseTexture);
    glBindTexture(GL_TEXTURE_3D, noiseTexture);
    glTexStorage3D(GL_TEXTURE_3D, 1 + 7 /* log2(128) */, GL_RGBA8, NOISE_SIZE, NOISE_SIZE, NOISE_SIZE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_REPEAT);

    glUseProgram(program);
    glBindImageTexture(0, noiseTexture, 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA8);
    glUniform1i(glGetUniformLocation(program, "noiseIm"), 0);

    // Generate in slabs to avoid a single very long dispatch.
    GLint uZOffset = glGetUniformLocation(program, "zOffset");
    const int slab = 16;
    for (int z = 0; z < NOISE_SIZE; z += slab) {
        glUniform1i(uZOffset, z);
        glDispatchCompute(NOISE_SIZE / 4, NOISE_SIZE / 4, slab / 4);
    }

    glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT | GL_TEXTURE_UPDATE_BARRIER_BIT);
    glGenerateMipmap(GL_TEXTURE_3D);

    glBindTexture(GL_TEXTURE_3D, 0);
    glUseProgram(0);
}

void Clouds::generateWeather() {
    ComputeShaderProgram weatherProgram;
    weatherProgram.setComputeShaderFromFile(weatherShaderFile);
    GLuint program = weatherProgram.getProgram();

    const int levels = 1 + 9; // log2(512)

    glGenTextures(1, &weatherTexture);
    glBindTexture(GL_TEXTURE_2D, weatherTexture);
    glTexStorage2D(GL_TEXTURE_2D, levels, GL_R8, WEATHER_SIZE, WEATHER_SIZE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glUseProgram(program);
    glBindImageTexture(0, weatherTexture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R8);
    glUniform1i(glGetUniformLocation(program, "weatherIm"), 0);

    glDispatchCompute(WEATHER_SIZE / 8, WEATHER_SIZE / 8, 1);

    glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT | GL_TEXTURE_UPDATE_BARRIER_BIT);
    glGenerateMipmap(GL_TEXTURE_2D);

    glBindTexture(GL_TEXTURE_2D, 0);
    glUseProgram(0);
}

Clouds::~Clouds() {
    glDeleteVertexArrays(1, &vao);

    glDeleteTextures(1, &cloudTexture);
    glDeleteTextures(1, &cloudDepthTexture);
    glDeleteTextures(1, &noiseTexture);
    glDeleteTextures(1, &weatherTexture);

    delete computeProgram;
    delete blendProgram;
}

void Clouds::render() {
    ivec2 dws = DIV_ROUND_UP(camera->getWindowSize(), DOWNSCALE);
    vec3 pos = camera->getPosition();
    vec3 sunDir = landscape->getSunDirection();

    mat4 viewMat = landscape->getViewMatrix();
    mat4 projMat = landscape->getProjectionMatrix();
    // Inverse of projection * (rotation part of view). Unprojecting a point
    // with it gives the ray direction directly, without precision problems
    // caused by the camera translation and the extreme near/far ratio.
    mat4 invVPMat = glm::inverse(projMat * mat4(mat3(viewMat)));

    // Bindings shared by both passes:
    // unit 0 = scene color, 1 = scene depth, 2 = cloud color, 3 = cloud depth,
    // 4 = noise, 5 = weather
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, landscape->getDepthTexture());
    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_3D, noiseTexture);
    glActiveTexture(GL_TEXTURE5);
    glBindTexture(GL_TEXTURE_2D, weatherTexture);

    // Ray-march clouds.
    glUseProgram(computeProgram->getProgram());

    glUniform1i(uDepth, 1);
    glUniform1i(uNoise, 4);
    glUniform1i(uWeather, 5);

    glBindImageTexture(2, cloudTexture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA16F);
    glUniform1i(uCloud, 2);

    glBindImageTexture(3, cloudDepthTexture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R32F);
    glUniform1i(uCloudDepth, 3);

    glUniform3fv(uPosition, 1, &pos[0]);
    glUniform3fv(uSunDirection, 1, &sunDir[0]);
    glUniform1f(uTime, time);
    glUniform1f(uCoverage, coverage);
    glUniformMatrix4fv(uInvVP, 1, GL_FALSE, glm::value_ptr(invVPMat));

    glDispatchCompute(DIV_ROUND_UP(dws.x, 8), DIV_ROUND_UP(dws.y, 8), 1);

    // Make image stores visible to the texture fetches in the blend pass.
    glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT);

    // Composite.
    glUseProgram(blendProgram->getProgram());

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, landscape->getColorTexture());
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, cloudTexture);
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, cloudDepthTexture);

    glUniform1i(uBlendSceneColor, 0);
    glUniform1i(uBlendSceneDepth, 1);
    glUniform1i(uBlendCloudColor, 2);
    glUniform1i(uBlendCloudDepth, 3);
    glUniform1i(uBlendNoise, 4);
    glUniform1i(uBlendWeather, 5);

    glUniform3fv(uBlendPosition, 1, &pos[0]);
    glUniform3fv(uBlendSunDirection, 1, &sunDir[0]);
    glUniform1f(uBlendTime, time);
    glUniform1f(uBlendCoverage, coverage);
    glUniformMatrix4fv(uBlendInvVP, 1, GL_FALSE, glm::value_ptr(invVPMat));

    glBindVertexArray(vao);

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    glDrawArrays(GL_TRIANGLES, 0, 3);

    glBindVertexArray(0);
    glActiveTexture(GL_TEXTURE0);
}

void Clouds::step(float _time, float) {
    time = _time;
}

IEventListener::EventResponse Clouds::onEvent(SDL_Event *evt) {
    if (evt->type == SDL_WINDOWEVENT) {
        SDL_WindowEvent *e = &evt->window;

        if (e->event == SDL_WINDOWEVENT_RESIZED
                || e->event == SDL_WINDOWEVENT_SIZE_CHANGED) {
            allocateTextures();
            return EVT_PROCESSED;
        }
    } else if (evt->type == SDL_KEYDOWN) {
        SDL_KeyboardEvent *e = &evt->key;

        switch (e->keysym.sym) {
            case SDLK_r:
                loadShaders();
                std::cerr << "Cloud shaders reloaded" << std::endl;
                return EVT_PROCESSED;
            case SDLK_LEFTBRACKET:
            case SDLK_RIGHTBRACKET:
                coverage += (e->keysym.sym == SDLK_RIGHTBRACKET) ? 0.05f : -0.05f;
                coverage = clamp(coverage, 0.0f, 1.0f);
                std::cerr << "Cloud coverage: " << coverage << std::endl;
                return EVT_PROCESSED;
        }
    }

    return EVT_IGNORED;
}
