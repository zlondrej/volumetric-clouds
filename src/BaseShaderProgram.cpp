#include <GL/glew.h>
#include <fstream>
#include <iterator>

#include "BaseShaderProgram.hpp"
#include "Exceptions.hpp"

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

string BaseShaderProgram::getFileContents(string &filename) {
    ifstream fileStream(filename.c_str());

    if (!fileStream) {
        throw string("Could not load file '" + filename + "'.");
    }

    // Minimal support for `#include "file"` (relative to the including file).
    string dir;
    size_t slash = filename.find_last_of('/');
    if (slash != string::npos) {
        dir = filename.substr(0, slash + 1);
    }

    string result, line;
    while (getline(fileStream, line)) {
        size_t pos = line.find_first_not_of(" \t");
        if (pos != string::npos && line.compare(pos, 8, "#include") == 0) {
            size_t open = line.find('"', pos);
            size_t close = line.find('"', open + 1);
            if (open != string::npos && close != string::npos) {
                string included = dir + line.substr(open + 1, close - open - 1);
                result += getFileContents(included);
                continue;
            }
        }
        result += line;
        result += '\n';
    }

    return result;
}

GLuint BaseShaderProgram::createShaderFromSource(string &source, GLenum type) {
    GLint compileStatus;
    GLuint shader = glCreateShader(type);
    const char *cSource = source.c_str();

    glShaderSource(shader, 1, &cSource, NULL);

    glCompileShader(shader);

    glGetShaderiv(shader, GL_COMPILE_STATUS, &compileStatus);

    if (compileStatus != GL_TRUE) {

        string info = getShaderInfo(shader);

        glDeleteShader(shader);
        throw GLException(info);
    }

    return shader;
}

GLuint BaseShaderProgram::createShaderFromFile(string &filename, GLenum type) {
    string source = getFileContents(filename);

    return createShaderFromSource(source, type);
}

string BaseShaderProgram::getShaderInfo(GLuint shader) {
    GLint infoLength = 0;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLength);

    string info;
    info.resize(infoLength);

    glGetShaderInfoLog(shader, infoLength, &infoLength, &info[0]);

    return info;
}

string BaseShaderProgram::getProgramInfo(GLuint program) {
    GLint infoLength = 0;
    glGetProgramiv(program, GL_INFO_LOG_LENGTH, &infoLength);

    string info;
    info.resize(infoLength);

    glGetProgramInfoLog(program, infoLength, &infoLength, &info[0]);

    return info;
}
