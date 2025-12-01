#ifndef AV_ANDROID_LEARNING_PROGRAM_H
#define AV_ANDROID_LEARNING_PROGRAM_H

#include "Glsl2Util.h"
#include <GLES2/gl2.h>
#include <unordered_map>
#include <string>

using HandleCache = std::unordered_map<std::string, GLint>;

/**
 * @brief 封装 OpenGL 着色器程序的类。
 */
class GlProgram {
private:
    GLuint programId;

    HandleCache glAttributeHandleCache = {};
    HandleCache glUniformHandleCache = {};

    explicit GlProgram(GLuint programId) {
        this->programId = programId;
    }

public:
    ~GlProgram() {
        glDeleteProgram(programId);
    }

    static GlProgram *fromAssets(const char *vertexShaderPath, const char *fragmentShaderPath) {
        GLuint id = createProgramFromAssets(
                vertexShaderPath,
                fragmentShaderPath
        );
        return new GlProgram(id);
    }

    static GlProgram *fromShaders(const char *vertexShaderCode, const char *fragmentShaderCode) {
        GLuint id = createProgramFromShaderCode(
                vertexShaderCode,
                fragmentShaderCode
        );
        return new GlProgram(id);
    }

#pragma clang diagnostic push
#pragma ide diagnostic ignored "readability-convert-member-functions-to-static"

    /**
     * 设置背景颜色。
     * @param red 红色，范围 [0.0, 1.0]
     * @param green  绿色，范围 [0.0, 1.0]
     * @param blue  蓝色，范围 [0.0, 1.0]
     * @param alpha  透明度，范围 [0.0, 1.0]，0.0 表示完全透明，1.0 表示完全不透明
     */
    void setBgColor(float red, float green, float blue, float alpha) {
        glClearColor(red, green, blue, alpha);
    }

    void drawArraysStrip(int count) {
        glDrawArrays(GL_TRIANGLE_STRIP, 0, count);
    }

    /**
     * 清除颜色缓冲区。相当于用预设的背景颜色填充整个颜色缓冲区。
     */
    void clearColorBuffer() {
        glClear(GL_COLOR_BUFFER_BIT);
    }

    /**
     * 清除颜色缓冲区、深度缓冲区和模板缓冲区。
     */
    void clearBuffer() {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    }

#pragma clang diagnostic pop


    GLint activeAttribute(const std::string &name) {
        if (glAttributeHandleCache.find(name) != glAttributeHandleCache.end()) {
            return glAttributeHandleCache[name];
        }
        GLint handle = glGetAttribLocation(programId, name.c_str());
        glAttributeHandleCache[name] = handle;
        return handle;
    }

    GLint activeUniform(const std::string &name) {
        if (glUniformHandleCache.find(name) != glUniformHandleCache.end()) {
            return glUniformHandleCache[name];
        }
        GLint handle = glGetUniformLocation(programId, name.c_str());
        glUniformHandleCache[name] = handle;
        return handle;
    }

    void startDraw(const std::function<void(GlProgram &)> &onDraw) {
        glUseProgram(programId);

        // 启用所有顶点属性数组
        for (const auto &[name, location]: glAttributeHandleCache) {
            glEnableVertexAttribArray(location);
        }

        // 执行用户绘制逻辑，传递当前对象引用
        onDraw(*this);

        // 清理：RAII的"Scope Guard"模式
        for (const auto &[name, location]: glAttributeHandleCache) {
            glDisableVertexAttribArray(location);
        }

        glUseProgram(0);
    }


    void vertexAttribPointerFloat(
            const std::string &name,
            GLint elementsPerVertex,
            const void *pointer,
            const int stride = -1
    ) {
        int realStride = stride;
        if (realStride == -1) {
            realStride = static_cast<GLsizei>(elementsPerVertex * sizeof(float));
        }
        glVertexAttribPointer(
                activeAttribute(name),
                elementsPerVertex,
                GL_FLOAT,
                GL_FALSE,
                realStride,
                pointer
        );
    }

    void vertexAttribPointerInt(
            const std::string &name,
            GLint elementsPerVertex,
            const void *pointer,
            const int stride = -1
    ) {
        int realStride = stride;
        if (realStride == -1) {
            realStride = static_cast<GLsizei>(elementsPerVertex * sizeof(int));
        }
        glVertexAttribPointer(
                activeAttribute(name),
                elementsPerVertex,
                GL_INT,
                GL_FALSE,
                realStride,
                pointer
        );
    }

    void uniformMatrix4fv(const std::string &name, const GLfloat *matrix) {
        glUniformMatrix4fv(
                activeUniform(name),
                1,
                GL_FALSE,
                matrix
        );
    }

    void uniform1f(const std::string &name, GLfloat v0) {
        glUniform1f(activeUniform(name), v0);
    }

    void uniform1i(const std::string &name, GLint v0) {
        glUniform1i(activeUniform(name), v0);
    }

};

#endif //AV_ANDROID_LEARNING_PROGRAM_H