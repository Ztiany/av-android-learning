#ifndef AV_ANDROID_LEARNING_PROGRAM_H
#define AV_ANDROID_LEARNING_PROGRAM_H

#include "Glsl2Util.h"
#include <GLES2/gl2.h>
#include <unordered_map>
#include <unordered_set>
#include <string>

/**
 * @brief 封装 OpenGL 着色器程序的类。
 */
class GlProgram {
private:
    GLuint programId;

    std::unordered_map<std::string, GLint> glAttributeHandleCache = {};

    std::unordered_map<std::string, GLint> glUniformHandleCache = {};

    std::unordered_set<GLint> glEnabledAttributes = {};

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
        glEnabledAttributes.clear();

        onDraw(*this);

        for (const auto &item: glEnabledAttributes) {
            glDisableVertexAttribArray(item);
        }
        glEnabledAttributes.clear();
        glUseProgram(0);
    }

    void vertexAttribPointerFloat(
            const std::string &name,
            GLint elementsPerVertex,
            const void *pointer,
            const int stride = -1
    ) {
        setAttribute(name, elementsPerVertex, GL_FLOAT, pointer, stride);
    }

    void vertexAttribPointerUnsignedInt(
            const std::string &name,
            GLint elementsPerVertex,
            const void *pointer,
            const int stride = -1
    ) {
        setAttribute(name, elementsPerVertex, GL_UNSIGNED_INT, pointer, stride);
    }

    void vertexAttribPointerInt(
            const std::string &name,
            GLint elementsPerVertex,
            const void *pointer,
            const int stride = -1
    ) {
        setAttribute(name, elementsPerVertex, GL_INT, pointer, stride);
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

    /**
     * 设置背景颜色。
     * @param red 红色，范围 [0.0, 1.0]
     * @param green  绿色，范围 [0.0, 1.0]
     * @param blue  蓝色，范围 [0.0, 1.0]
     * @param alpha  透明度，范围 [0.0, 1.0]，0.0 表示完全透明，1.0 表示完全不透明。
     */
    static void setBgColor(float red, float green, float blue, float alpha) {
        glClearColor(red, green, blue, alpha);
    }

    /**
     * 清除颜色缓冲区。相当于用预设的背景颜色填充整个颜色缓冲区。
     */
    static void clearColorBuffer() {
        glClear(GL_COLOR_BUFFER_BIT);
    }

    /**
     * 清除颜色缓冲区、深度缓冲区和模板缓冲区。
     */
    static void clearBuffer() {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    }

private:

    void setAttribute(
            const std::string &name,
            GLint elementsPerVertex,
            GLenum type,
            const void *pointer,
            const int stride
    ) {
        GLint index = activeAttribute(name);
        if (index == -1) {
            LOGD("Attribute '%s' not found in shader program %d", name.c_str(), programId);
            return;
        }

        int realStride = stride;
        if (realStride == -1) {
            size_t typeSize = 0;
            switch (type) {
                case GL_FLOAT:
                    typeSize = sizeof(float);
                    break;
                case GL_INT:
                    typeSize = sizeof(int);
                    break;
                case GL_UNSIGNED_INT:
                    typeSize = sizeof(unsigned int);
                    break;
                case GL_SHORT:
                    typeSize = sizeof(short);
                    break;
                case GL_UNSIGNED_SHORT:
                    typeSize = sizeof(unsigned short);
                    break;
                default:
                    typeSize = sizeof(float);
                    break;
            }
            realStride = static_cast<GLsizei>(elementsPerVertex * typeSize);
        }

        /*
         * glVertexAttribPointer：连接水管，告诉 GPU 数据“在哪里”以及“长什么样”。它定义如何从当前绑定的顶点缓冲区中
         *                        读取数据，但不启用数据流。
         * glEnableVertexAttribArray：打开水龙头，激活/启用某个顶点属性通道。告诉 GPU 开始从这个已配置的通道接收数据
         *                            用于渲染。
         *
         * 调用顺序的关系是：必须先“连接水管”，再“打开水龙头”。 这个顺序在逻辑上更合理，因为你需要先定义数据源和格式，然后
         * 才能启用数据流。
         *
         * glVertexAttribPointer 将当前绑定的顶点缓冲区（通过 glBindBuffer(GL_ARRAY_BUFFER, vbo) 绑定）中的
         * 一块数据，与指定的顶点属性位置（location）关联起来。
         *
         * glEnableVertexAttribArray “打开开关”，激活指定的顶点属性数组。只有启用后，GPU 在绘制时（调用
         * glDrawArrays 等）才会从该位置读取数据。
         */
        glVertexAttribPointer(
                index,
                elementsPerVertex,
                type,
                GL_FALSE,
                realStride,
                pointer
        );
        glEnableVertexAttribArray(index);
        glEnabledAttributes.insert(index);
    }
};

#endif //AV_ANDROID_LEARNING_PROGRAM_H