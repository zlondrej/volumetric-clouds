#include <GL/glew.h>
#include <fstream>
#include <iterator>

#include "BaseShaderProgram.hpp"

using namespace pgp;
using namespace std;

BaseShaderProgram::BaseShaderProgram() : program(0) {

}

GLuint BaseShaderProgram::getProgram() {
    if (program == 0) {
        program = compile();
    }

    return program;
}

string BaseShaderProgram::getFileContents(string filename) {
    ifstream fileStream(filename.c_str());

    if (!fileStream) {
        throw string("Could not load file '" + filename + "'.");
    }

    return string(istream_iterator<char>(fileStream), istream_iterator<char>());
}

GLuint BaseShaderProgram::createShaderFromSource(string source, GLenum type) {
    GLint compileStatus;
    GLuint shader = glCreateShader(type);
    const char *cSource = source.c_str();

    glShaderSource(shader, 1, &cSource, NULL);

    glGetShaderiv(shader, GL_COMPILE_STATUS, &compileStatus);

    if (compileStatus != GL_TRUE) {
        throw string("Could not compile shader from given source file.");
    }

    return shader;
}

GLuint BaseShaderProgram::createShaderFromFile(string filename, GLenum type) {
    string source = getFileContents(filename);

    return createShaderFromSource(source, type);
}