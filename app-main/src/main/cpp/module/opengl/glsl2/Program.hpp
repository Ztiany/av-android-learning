#ifndef AV_ANDROID_LEARNING_PROGRAM_H
#define AV_ANDROID_LEARNING_PROGRAM_H

#include "Glsl2Util.h"
#include <GLES2/gl2.h>

/**
 * @brief 封装 OpenGL 着色器程序的类。
 */
class Program {
private:
    GLuint programId;

    explicit Program(GLuint programId) {
        this->programId = programId;
    }

public:
    ~Program() {
        glDeleteProgram(programId);
    }

    static Program *fromAssets(const char *vertexShaderPath, const char *fragmentShaderPath) {
        GLuint id = createProgramFromAssets(
                vertexShaderPath,
                fragmentShaderPath
        );
        return new Program(id);
    }

    static Program *fromShaders(const char *vertexShaderCode, const char *fragmentShaderCode) {
        GLuint id = createProgramFromShaderCode(
                vertexShaderCode,
                fragmentShaderCode
        );
        return new Program(id);
    }

    static void resetProgram() {
        glUseProgram(0);
    }

    void useProgram() const {
        glUseProgram(programId);
    }

};

#endif //AV_ANDROID_LEARNING_PROGRAM_H