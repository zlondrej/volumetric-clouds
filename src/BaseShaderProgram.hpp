#pragma once

#include <GL/gl.h>
#include <string>

namespace pgp {

    using std::string;

    class BaseShaderProgram {
    private:
        GLuint program;
    public:
        BaseShaderProgram();

        ~BaseShaderProgram() {
            deleteProgram();
        }
        GLuint getProgram();

    protected:
        virtual GLuint compile() = 0;

        inline void deleteProgram() {
            if (program) {
                glDeleteProgram(program);
                program = 0;
            }
        }
        string getFileContents(string &filename);
        GLuint createShaderFromSource(string &source, GLenum shaderType);
        GLuint createShaderFromFile(string &filename, GLenum shaderType);
    };

}