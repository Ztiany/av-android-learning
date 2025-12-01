#ifndef AV_ANDROID_LEARNING_GLSL2UTIL_H
#define AV_ANDROID_LEARNING_GLSL2UTIL_H

#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <GLES2/gl2platform.h>

GLuint compileShader(GLenum shaderType, const char *sourceCode);

GLuint createProgramFromShaders(GLuint vertexShader, GLuint fragmentShader);

GLuint createProgramFromShaderCode(
        const char *vertexShaderCode,
        const char *fragmentShaderCode
);

GLuint createProgramFromAssets(
        const char *vertexShaderPath,
        const char *fragmentShaderPath
);

#endif