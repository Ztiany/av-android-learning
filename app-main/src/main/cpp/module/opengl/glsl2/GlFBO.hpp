#ifndef AV_ANDROID_LEARNING_GLFBO_HPP
#define AV_ANDROID_LEARNING_GLFBO_HPP

#include <GLES2/gl2.h>
#include <android/log.h>
#include <GLES2/gl2ext.h>
#include <stdexcept>
#include <utility>

#include "../../common/log.h"

/**
 * @brief OpenGL 帧缓冲对象( FBO )封装类，用于离屏渲染
 * 
 * 该类封装了 OpenGL 的帧缓冲对象，支持创建颜色纹理缓冲区和深度/模板渲染缓冲区，
 * 并提供了简单的接口来切换渲染目标。
 * 
 * 使用示例:
 *
 * ```cpp
 * GlFBO fbo;
 * fbo.init(1024, 768);  // 初始化1024x768的离屏渲染缓冲区
 * fbo.beginDraw();      // 开始离屏渲染
 * // ... 执行绘制操作 ...
 * fbo.endDraw();        // 结束离屏渲染，切换回默认帧缓冲
 * GLuint textureId = fbo.getColorTextureId();  // 获取颜色纹理ID用于后续渲染
 * ```
 */
class GlFBO {
private:
    GLint frameBufferObjectBeforeSwitch;  // 保存切换前绑定的帧缓冲对象ID
    GLuint frameBufferObject;             // 当前 FBO 的 ID
    GLuint colorBuffer;                   // 颜色缓冲区(纹理)的 ID
    GLuint depthStencilBuffer;            // 深度/模板缓冲区的 ID
    int width;                            // FBO 宽度
    int height;                           // FBO 高度
    bool initialized;                     // 初始化状态标志

    // 辅助函数：检 查OpenGL 错误
    static void checkGLError(const char *operation) {
        GLenum error = glGetError();
        if (error != GL_NO_ERROR) {
            LOGE("OpenGL error after %s: 0x%x", operation, error);
        }
    }

public:
    /**
     * @brief 默认构造函数
     * 
     * 初始化成员变量为安全状态，但不分配OpenGL资源。
     * 实际资源分配在init()方法中进行。
     */
    GlFBO() {
        frameBufferObjectBeforeSwitch = 0;
        frameBufferObject = 0;
        colorBuffer = 0;
        depthStencilBuffer = 0;
        width = 0;
        height = 0;
        initialized = false;
    }

    /**
     * @brief 析构函数
     * 
     * 释放所有已分配的OpenGL资源，遵循RAII原则。
     * 这是现代C++资源管理的核心实践。
     */
    ~GlFBO() {
        cleanup();
    }

    /**
     * @brief 移动构造函数 (C++11)
     * 
     * 支持资源的高效转移，避免不必要的深拷贝。
     * 移动后，源对象处于有效但未初始化状态。
     * 
     * @param other 要移动的源对象
     */
    GlFBO(GlFBO &&other) noexcept {

        frameBufferObjectBeforeSwitch = other.frameBufferObjectBeforeSwitch;
        frameBufferObject = other.frameBufferObject;
        colorBuffer = other.colorBuffer;
        depthStencilBuffer = other.depthStencilBuffer;
        width = other.width;
        height = other.height;
        initialized = other.initialized;

        // 将源对象置于有效但未初始化状态
        other.frameBufferObjectBeforeSwitch = 0;
        other.frameBufferObject = 0;
        other.colorBuffer = 0;
        other.depthStencilBuffer = 0;
        other.width = 0;
        other.height = 0;
        other.initialized = false;
    }

    /**
     * @brief 移动赋值运算符 (C++11)
     * 
     * 支持资源的高效转移，遵循"自我赋值安全"原则。
     * 
     * @param other 要移动的源对象
     * @return 当前对象的引用
     */
    GlFBO &operator=(GlFBO &&other) noexcept {
        if (this != &other) {  // 防止自我赋值
            // 释放当前资源
            cleanup();

            // 转移资源
            frameBufferObjectBeforeSwitch = other.frameBufferObjectBeforeSwitch;
            frameBufferObject = other.frameBufferObject;
            colorBuffer = other.colorBuffer;
            depthStencilBuffer = other.depthStencilBuffer;
            width = other.width;
            height = other.height;
            initialized = other.initialized;

            // 将源对象置于有效但未初始化状态
            other.frameBufferObjectBeforeSwitch = 0;
            other.frameBufferObject = 0;
            other.colorBuffer = 0;
            other.depthStencilBuffer = 0;
            other.width = 0;
            other.height = 0;
            other.initialized = false;
        }
        return *this;
    }

    // 禁用拷贝构造和拷贝赋值，因为OpenGL资源不应该被浅拷贝
    GlFBO(const GlFBO &) = delete;

    GlFBO &operator=(const GlFBO &) = delete;

    /**
     * @brief 初始化FBO资源
     * 
     * 创建帧缓冲对象、颜色纹理缓冲区和深度/模板缓冲区。
     * 该方法只会执行一次，后续调用将被忽略。
     * 
     * @param fboWidth 缓冲区宽度(像素)
     * @param fboHeight 缓冲区高度(像素)
     * @throws std::runtime_error 如果FBO创建失败
     */
    void init(int fboWidth, int fboHeight) {
        // 防止重复初始化
        if (initialized) {
            LOGW("FBO already initialized, skipping re-initialization");
            return;
        }

        // 参数检查
        if (fboWidth <= 0 || fboHeight <= 0) {
            throw std::runtime_error("Invalid FBO dimensions");
        }

        // 保存尺寸
        fboWidth = fboWidth;
        fboHeight = fboHeight;

        // 创建帧缓冲对象(FBO)
        // glGenFramebuffers: 生成指定数量的帧缓冲对象名称
        glGenFramebuffers(1, &frameBufferObject);
        checkGLError("glGenFramebuffers");

        // 创建颜色缓冲区(作为纹理)
        glGenTextures(1, &colorBuffer);
        glBindTexture(GL_TEXTURE_2D, colorBuffer);

        // 设置纹理参数
        // GL_TEXTURE_MAG_FILTER: 纹理放大过滤器 - GL_LINEAR表示线性插值
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        // GL_TEXTURE_MIN_FILTER: 纹理缩小过滤器 - GL_LINEAR表示线性插值
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        // GL_TEXTURE_WRAP_S: 纹理S方向(水平)的环绕模式 - GL_CLAMP_TO_EDGE表示边缘截取
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        // GL_TEXTURE_WRAP_T: 纹理T方向(垂直)的环绕模式 - GL_CLAMP_TO_EDGE表示边缘截取
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        // 分配纹理存储空间
        // glTexImage2D: 指定二维纹理图像
        // 参数说明:
        //   GL_TEXTURE_2D: 目标纹理
        //   0: 详细级别，0表示基本图像级别
        //   GL_RGBA: 内部格式 - 颜色组件为RGBA
        //   fboWidth, fboHeight: 纹理尺寸
        //   0: 边界宽度(必须为0)
        //   GL_RGBA: 像素数据的格式
        //   GL_UNSIGNED_BYTE: 像素数据的数据类型
        //   nullptr: 像素数据指针(为nullptr表示只分配空间不提供初始数据)
        glTexImage2D(
                GL_TEXTURE_2D,
                0,
                GL_RGBA,
                fboWidth,
                fboHeight,
                0,
                GL_RGBA,
                GL_UNSIGNED_BYTE,
                nullptr
        );
        checkGLError("glTexImage2D");

        // 解绑纹理，避免意外修改
        glBindTexture(GL_TEXTURE_2D, 0);

        // 创建深度/模板缓冲区(作为渲染缓冲区)
        // 渲染缓冲区比纹理更高效，但无法在其他着色器中采样
        glGenRenderbuffers(1, &depthStencilBuffer);
        glBindRenderbuffer(GL_RENDERBUFFER, depthStencilBuffer);

        // 分配渲染缓冲区存储空间
        // GL_DEPTH24_STENCIL8_OES: 24位深度缓冲区和8位模板缓冲区打包格式
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8_OES, fboWidth, fboHeight);
        checkGLError("glRenderbufferStorage");

        // 解绑渲染缓冲区
        glBindRenderbuffer(GL_RENDERBUFFER, 0);

        // 设置FBO关联：将颜色缓冲区和深度/模板缓冲区附加到帧缓冲对象
        // 首先保存当前绑定的帧缓冲对象
        glGetIntegerv(GL_FRAMEBUFFER_BINDING, &frameBufferObjectBeforeSwitch);

        // 绑定我们创建的FBO
        glBindFramebuffer(GL_FRAMEBUFFER, frameBufferObject);

        // 附加颜色缓冲区(纹理)到FBO
        // glFramebufferTexture2D: 将纹理图像附加到帧缓冲对象
        // GL_COLOR_ATTACHMENT0: 颜色附加点0，一个FBO可以有多个颜色附加点
        glFramebufferTexture2D(
                GL_FRAMEBUFFER,
                GL_COLOR_ATTACHMENT0,
                GL_TEXTURE_2D,
                colorBuffer,
                0  // mipmap级别，0表示基本级别
        );

        // 附加深度缓冲区到FBO
        // glFramebufferRenderbuffer: 将渲染缓冲区对象附加到帧缓冲对象
        glFramebufferRenderbuffer(
                GL_FRAMEBUFFER,
                GL_DEPTH_ATTACHMENT,
                GL_RENDERBUFFER,
                depthStencilBuffer
        );

        // 附加模板缓冲区到FBO
        // 注意：这里我们使用了同一个渲染缓冲区对象，因为它同时包含深度和模板数据
        glFramebufferRenderbuffer(
                GL_FRAMEBUFFER,
                GL_STENCIL_ATTACHMENT,
                GL_RENDERBUFFER,
                depthStencilBuffer
        );

        // 检查FBO完整性
        GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

        if (GL_FRAMEBUFFER_COMPLETE == status) {
            LOGD("FBO created successfully with size %dx%d, OpenGL version: %s",
                 fboWidth, fboHeight, glGetString(GL_VERSION));
            initialized = true;
        } else {
            LOGE("FBO creation failed with status: 0x%x, OpenGL version: %s",
                 status, glGetString(GL_VERSION));
            // 清理已分配的资源
            cleanup();
            throw std::runtime_error("Framebuffer object is not complete");
        }

        // 切换回之前的帧缓冲对象
        glBindFramebuffer(GL_FRAMEBUFFER, frameBufferObjectBeforeSwitch);
    }

    /**
     * @brief 开始离屏渲染到FBO
     * 
     * 保存当前绑定的帧缓冲对象，并将渲染目标切换到当前FBO。
     * 此后所有的渲染操作将输出到FBO的颜色缓冲区。
     * 
     * 注意：调用此方法后必须调用endDraw()来恢复原来的渲染目标。
     */
    void beginDraw() {
        if (!initialized) {
            LOGE("FBO not initialized, cannot begin drawing");
            return;
        }

        // 保存当前绑定的帧缓冲对象
        glGetIntegerv(GL_FRAMEBUFFER_BINDING, &frameBufferObjectBeforeSwitch);

        // 绑定我们的FBO作为渲染目标
        glBindFramebuffer(GL_FRAMEBUFFER, frameBufferObject);

        // 设置视口大小为FBO大小，确保渲染覆盖整个缓冲区
        glViewport(0, 0, width, height);
    }

    /**
     * @brief 结束离屏渲染
     * 
     * 切换回调用beginDraw()之前绑定的帧缓冲对象。
     * 此后所有的渲染操作将输出到默认的屏幕缓冲区或其他目标。
     */
    void endDraw() const {
        if (!initialized) {
            LOGE("FBO not initialized, cannot end drawing");
            return;
        }

        // 切换回之前保存的帧缓冲对象
        glBindFramebuffer(GL_FRAMEBUFFER, frameBufferObjectBeforeSwitch);
    }

    /**
     * @brief 获取颜色纹理ID
     * 
     * 返回FBO颜色缓冲区的纹理ID，可用于后续的渲染操作，
     * 例如将离屏渲染的结果作为纹理绘制到屏幕上。
     * 
     * @return 颜色纹理的OpenGL ID，如果未初始化则返回0
     */
    GLuint getColorTextureId() const {
        return initialized ? colorBuffer : 0;
    }

    /**
     * @brief 获取FBO的宽度
     * @return 缓冲区宽度(像素)，如果未初始化则返回0
     */
    int getWidth() const {
        return initialized ? width : 0;
    }

    /**
     * @brief 获取FBO的高度
     * @return 缓冲区高度(像素)，如果未初始化则返回0
     */
    int getHeight() const {
        return initialized ? height : 0;
    }

    /**
     * @brief 检查FBO是否已初始化
     * @return 如果FBO已成功初始化则返回true，否则返回false
     */
    bool isInitialized() const {
        return initialized;
    }

private:
    /**
     * @brief 释放所有OpenGL资源
     * 
     * 这个辅助方法用于析构函数和错误清理中，
     * 确保所有分配的OpenGL资源都被正确释放。
     */
    void cleanup() {
        if (initialized) {
            // 删除渲染缓冲区
            if (depthStencilBuffer != 0) {
                glDeleteRenderbuffers(1, &depthStencilBuffer);
                depthStencilBuffer = 0;
            }

            // 删除颜色纹理
            if (colorBuffer != 0) {
                glDeleteTextures(1, &colorBuffer);
                colorBuffer = 0;
            }

            // 删除帧缓冲对象
            if (frameBufferObject != 0) {
                glDeleteFramebuffers(1, &frameBufferObject);
                frameBufferObject = 0;
            }

            // 重置初始化状态
            initialized = false;

            LOGD("FBO resources released");
        }
    }
};

#endif //AV_ANDROID_LEARNING_GLFBO_HPP
