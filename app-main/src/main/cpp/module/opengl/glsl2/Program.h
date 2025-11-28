#ifndef AV_ANDROID_LEARNING_PROGRAM_H
#define AV_ANDROID_LEARNING_PROGRAM_H

#include <GLES2/gl2.h>

/**
 * @brief 封装 OpenGL 着色器程序的类。
 */
class Program {
private:
    GLuint programId;

    explicit Program(GLuint programId);

public:

    ~Program();

    static Program *fromAssets(const char *vertexShaderPath, const char *fragmentShaderPath);

    static Program *fromShaders(const char *vertexShaderCode, const char *fragmentShaderCode);

    void useProgram() const;

    void resetProgram() const;

};

#endif //AV_ANDROID_LEARNING_PROGRAM_H
