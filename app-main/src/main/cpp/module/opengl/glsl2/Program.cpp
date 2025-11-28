#include "Program.h"
#include "Glsl2Util.h"

Program *Program::fromAssets(const char *vertexShaderPath, const char *fragmentShaderPath) {
    GLuint programId = createProgramFromAssets(
            vertexShaderPath,
            fragmentShaderPath
    );
    return new Program(programId);
}

Program *Program::fromShaders(const char *vertexShaderCode, const char *fragmentShaderCode) {
    GLuint programId = createProgramFromShaderCode(
            vertexShaderCode,
            fragmentShaderCode
    );
    return new Program(programId);
}

Program::Program(GLuint programId) {
    this->programId = programId;
}

Program::~Program() {
    glDeleteProgram(programId);
}

void Program::useProgram() const {
    glUseProgram(programId);
}

#pragma clang diagnostic push
#pragma ide diagnostic ignored "readability-convert-member-functions-to-static"

void Program::resetProgram() const {
    glUseProgram(0);
}

#pragma clang diagnostic pop
