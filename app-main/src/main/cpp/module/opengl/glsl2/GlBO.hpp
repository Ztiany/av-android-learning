#ifndef AV_ANDROID_LEARNING_GLBO_HPP
#define AV_ANDROID_LEARNING_GLBO_HPP

#include <GLES2/gl2.h>

class GlBO {
private:
    GLuint vboId;
    GLsizeiptr vboSize;
    GLsizeiptr vboType;

    explicit GlBO(GLuint vboId, GLsizeiptr size, GLenum type) {
        this->vboId = vboId;
        this->vboSize = size;
        this->vboType = type;
    }

    static GlBO *createBO(
            GLsizeiptr size,
            const void *data,
            GLenum usage,
            GLenum type
    ) {
        // 在显卡中创建 1 个 positionVbo，positionVbo 的编号放到 positionVbo 中。
        GLuint vboId = 0;
        glGenBuffers(1, &vboId);

        // 将创建的 positionVbo 设置到显卡卡槽上，让 type 对应的卡槽指向 positionVbo，此时 OpenGL 的当前状态，
        // type 对应的卡槽指向分配的 positionVbo。后续对 type 对应的卡槽的操作，都是对 vboId 进行操作。
        glBindBuffer(type, vboId);

        // 向显卡申请 size 大小的内存，data 是初始化数据，usage 是使用模式。
        glBufferData(type, size, data, usage);

        // 操作完成后，解绑 type 卡槽。
        glBindBuffer(type, 0);
        return new GlBO(vboId, size, type);
    }

public:
    ~GlBO() {
        glDeleteBuffers(1, &vboId);
    }

    /**
     * 创建一个 VBO 对象并初始化数据。
     * @param size 缓冲区大小。
     * @param data 缓冲区数据指针。可以传入 nullptr 来创建空缓冲区。之后可以使用 setData 或 updateData 方法更新数据。
     * @param usage 使用模式，如 GL_STATIC_DRAW、GL_DYNAMIC_DRAW 等。
     * @return 返回创建的 GlBO 对象指针。
     */
    static GlBO *createVBO(
            GLsizeiptr size,
            const void *data,
            GLenum usage
    ) {
        // GL_ARRAY_BUFFER 是一种卡槽，不同的卡槽有不同的用途。通常我们创建顶点数据缓冲区时，使用 GL_ARRAY_BUFFER 卡槽。
        return createBO(size, data, usage, GL_ARRAY_BUFFER);
    }

    static GlBO *createIBO(
            GLsizeiptr size,
            const void *data,
            GLenum usage
    ) {
        // GL_ELEMENT_ARRAY_BUFFER 是一种卡槽，通常我们创建索引数据缓冲区时，使用 GL_ELEMENT_ARRAY_BUFFER 卡槽。
        return createBO(size, data, usage, GL_ELEMENT_ARRAY_BUFFER);
    }

    /**
     * 设置 VBO 数据。如果创建 VBO 时，data 传入了 nullptr，则可以使用此方法设置数据。
     * @param size 缓冲区大小。
     * @param data 缓冲区数据指针。
     * @param usage 使用模式，如 GL_STATIC_DRAW、GL_DYNAMIC_DRAW 等。
     */
    void setData(GLsizeiptr size, const void *data, GLenum usage) const {
        glBindBuffer(vboType, vboId);
        glBufferData(vboType, size, data, usage);
        glBindBuffer(vboType, 0);
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
        glBindBuffer(vboType, vboId);
        glBufferSubData(vboType, offset, size, data);
        glBindBuffer(vboType, 0);
    }

    void bind() const {
        glBindBuffer(vboType, vboId);
    }

    void unbind() const {
        glBindBuffer(vboType, 0);
    }

};

#endif //AV_ANDROID_LEARNING_GLBO_HPP
