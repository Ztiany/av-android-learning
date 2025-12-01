#ifndef AV_ANDROID_LEARNING_GLVBO_HPP
#define AV_ANDROID_LEARNING_GLVBO_HPP

#include <GLES2/gl2.h>

class GlVBO {
private:
    GLuint vboId;
    GLsizeiptr vboSize;

    explicit GlVBO(GLuint vboId, GLsizeiptr size) {
        this->vboId = vboId;
        this->vboSize = size;
    }

public:
    ~GlVBO() {
        glDeleteBuffers(1, &vboId);
    }

    /**
     * 创建一个 VBO 对象并初始化数据。
     * @param size 缓冲区大小。
     * @param data 缓冲区数据指针。可以传入 nullptr 来创建空缓冲区。之后可以使用 setData 或 updateData 方法更新数据。
     * @param usage 使用模式，如 GL_STATIC_DRAW、GL_DYNAMIC_DRAW 等。
     * @return 返回创建的 GlVBO 对象指针。
     */
    static GlVBO *create(GLsizeiptr size, const void *data, GLenum usage) {
        // 在显卡中创建 1 个 vbo，vbo 的编号放到 vbo 中。
        GLuint vboId = 0;
        glGenBuffers(1, &vboId);

        // 将创建的 vbo 设置到显卡卡槽上，GL_ARRAY_BUFFER 是一种卡槽（还有其他卡槽，让 GL_ARRAY_BUFFER 这个卡槽指向 vbo）
        // 此时 OpenGL 的当前状态，GL_ARRAY_BUFFER 卡槽指向分配的 vbo。后续对 GL_ARRAY_BUFFER 的操作，都是对 vboId 进行操作。
        glBindBuffer(GL_ARRAY_BUFFER, vboId);

        // 向显卡申请 size 大小的内存，data 是初始化数据，usage 是使用模式
        glBufferData(GL_ARRAY_BUFFER, size, data, usage);

        // 操作完成后，解绑 GL_ARRAY_BUFFER 卡槽
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        return new GlVBO(vboId, size);
    }

    /**
     * 设置 VBO 数据。如果创建 VBO 时，data 传入了 nullptr，则可以使用此方法设置数据。
     * @param size 缓冲区大小。
     * @param data 缓冲区数据指针。
     * @param usage 使用模式，如 GL_STATIC_DRAW、GL_DYNAMIC_DRAW 等。
     */
    void setData(GLsizeiptr size, const void *data, GLenum usage) const {
        glBindBuffer(GL_ARRAY_BUFFER, vboId);
        glBufferData(GL_ARRAY_BUFFER, size, data, usage);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    GLsizeiptr size() const {
        return vboSize;
    }

    /**
     * 更新 VBO 数据的一部分。
     * @param size 需要更新的数据大小。
     * @param data 新的数据指针。
     * @param offset 偏移量，默认从缓冲区开头开始更新。
     */
    void updateData(GLsizeiptr size, const void *data, GLintptr offset = 0) const {
        glBindBuffer(GL_ARRAY_BUFFER, vboId);
        glBufferSubData(GL_ARRAY_BUFFER, offset, size, data);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    void bind() const {
        glBindBuffer(GL_ARRAY_BUFFER, vboId);
    }

#pragma clang diagnostic push
#pragma ide diagnostic ignored "readability-convert-member-functions-to-static"

    void unbind() {
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

#pragma clang diagnostic pop

};

#endif //AV_ANDROID_LEARNING_GLVBO_HPP
