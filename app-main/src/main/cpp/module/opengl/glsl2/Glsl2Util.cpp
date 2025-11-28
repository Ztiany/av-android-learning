#include "Glsl2Util.h"
#include "../common/resources.h"
#include "../common/log.h"

GLuint compileShader(GLenum shaderType, const char *sourceCode) {
    GLuint shader_to_ret = glCreateShader(shaderType);
    glShaderSource(shader_to_ret, 1, &sourceCode, nullptr);
    glCompileShader(shader_to_ret);

    GLint compile_result = GL_TRUE;
    glGetShaderiv(shader_to_ret, GL_COMPILE_STATUS, &compile_result);
    if (compile_result == GL_FALSE) {
        char szLog[1024] = {0};
        GLsizei logLen = 0;
        glGetShaderInfoLog(shader_to_ret, 1024, &logLen, szLog);
        LOGE("compileShader %s, code:%s", szLog, sourceCode);
        glDeleteShader(shader_to_ret);
        shader_to_ret = 0;
    }

    return shader_to_ret;
}

GLuint createProgramFromShaders(GLuint vertexShader, GLuint fragmentShader) {
    GLuint program_to_ret = glCreateProgram();

    glAttachShader(program_to_ret, vertexShader);
    glAttachShader(program_to_ret, fragmentShader);

    glLinkProgram(program_to_ret);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    GLint link_result = GL_TRUE;
    glGetProgramiv(program_to_ret, GL_LINK_STATUS, &link_result);
    if (GL_FALSE == link_result) {
        char szLog[1024] = {0};
        GLsizei logLen = 0;
        glGetProgramInfoLog(program_to_ret, 1024, &logLen, szLog);
        LOGE("createProgramFromShaders %s\n", szLog);
        glDeleteProgram(program_to_ret);
        program_to_ret = 0;
    }
    return program_to_ret;
}

GLuint createProgramFromShaderCode(
        const char *vertexShaderCode,
        const char *fragmentShaderCode
) {

    GLuint vertexShader = compileShader(
            GL_VERTEX_SHADER,
            (char *) vertexShaderCode);

    GLuint fragmentShader = compileShader(
            GL_FRAGMENT_SHADER,
            (char *) fragmentShaderCode
    );

    GLuint program = createProgramFromShaders(vertexShader, fragmentShader);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return program;
}

GLuint createProgramFromAssets(
        const char *vertexShaderPath,
        const char *fragmentShaderPath
) {
    size_t fileSize = 0;

    unsigned char *assetContent = loadAssetContent(vertexShaderPath, fileSize);
    GLuint vertexShader = compileShader(
            GL_VERTEX_SHADER,
            (char *) assetContent
    );
    delete[] assetContent;

    assetContent = loadAssetContent(fragmentShaderPath, fileSize);
    GLuint fragmentShader = compileShader(
            GL_FRAGMENT_SHADER,
            (char *) assetContent
    );
    delete[] assetContent;

    GLuint program = createProgramFromShaders(vertexShader, fragmentShader);
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return program;
}