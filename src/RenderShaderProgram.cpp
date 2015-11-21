#include "RenderShaderProgram.hpp"

using namespace pgp;

GLuint RenderShaderProgram::compile() {

    GLint linkStatus;
    GLuint renderProgram;

    if (vertexShader == 0) {
        throw string("Could not compile render program: Vertex shader missing.");
    } else if (fragmentShader == 0) {
        throw string("Could not compile render program: Fragment shader missing.");
    }


    renderProgram = glCreateProgram();

    glAttachShader(renderProgram, vertexShader);
    glAttachShader(renderProgram, fragmentShader);

    glLinkProgram(renderProgram);
  
	glGetProgramiv(renderProgram, GL_LINK_STATUS, &linkStatus);

	if (!linkStatus) {
        glDeleteProgram(renderProgram);
        throw string("Could not link render program.");
	}

    return renderProgram;

}

