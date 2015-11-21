#pragma once

#include <GL/glew.h>
#include <string>

namespace pgp {

    using std::string;

    class BaseShaderProgram {
        GLuint program;
    public:
        BaseShaderProgram();
        GLuint getProgram();

    protected:
        virtual GLuint compile() = 0;
        string getFileContents(string filename);
        GLuint createShaderFromSource(string source, GLenum shaderType);
        GLuint createShaderFromFile(string filename, GLenum shaderType);
    };

}