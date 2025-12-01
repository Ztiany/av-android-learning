#ifndef AV_ANDROID_LEARNING_GLMVPMATRIX_HPP
#define AV_ANDROID_LEARNING_GLMVPMATRIX_HPP

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <cmath>

/**
 * OpenGL Model-View-Projection 矩阵管理类，使用 GLM 库进行矩阵计算，支持正交投影适配。
 */
class GlMVPMatrix {
private:
    /** Camera 在 Z 轴的位置 */
    static constexpr float DEFAULT_CAMERA_Z = 5.0F;
    /** 视点原点（Camera 位置）到近平面的距离 */
    static constexpr float DEFAULT_NEAR = 0.1F;
    /** 视点原点（Camera 位置）到远平面的距离 */
    static constexpr float DEFAULT_FAR = 10.0F;

    static constexpr float EPSILON = 1e-6F;
private:
    // 矩阵数据
    glm::mat4 modelMatrix;
    glm::mat4 viewMatrix;
    glm::mat4 projectionMatrix;
    glm::mat4 mvpMatrix;

    // 尺寸数据
    float modelWidth;
    float modelHeight;
    float worldWidth;
    float worldHeight;

    // 状态标志
    bool isDirty;  // 新增：标记矩阵是否需要重新计算

public:
    /**
     * 构造函数 - 使用成员初始化列表。
     */
    GlMVPMatrix() : modelMatrix(1.0f), // GLM 单位矩阵初始化
                    viewMatrix(1.0f),
                    projectionMatrix(1.0f),
                    mvpMatrix(1.0f),
            // 尺寸数据
                    modelWidth(0.0f),
                    modelHeight(0.0f),
                    worldWidth(0.0f),
                    worldHeight(0.0f),
            // 初始状态为干净
                    isDirty(false) {
    }

    /**
     * 设置模型尺寸。
     * @param width 模型宽度
     * @param height 模型高度
     */
    void setModelSize(float width, float height) {
        // 使用浮点数精度安全比较
        if (std::abs(modelWidth - width) > EPSILON ||
            std::abs(modelHeight - height) > EPSILON) {
            modelWidth = width;
            modelHeight = height;
            markDirty();  // 标记需要重新计算
        }
    }

    /**
     * 设置世界（视口）尺寸。
     * @param width 世界宽度
     * @param height 世界高度
     */
    void setWorldSize(float width, float height) {
        if (std::abs(worldWidth - width) > EPSILON ||
            std::abs(worldHeight - height) > EPSILON) {
            worldWidth = width;
            worldHeight = height;
            markDirty();
        }
    }

    /**
     * 使用默认位置摆放相机。
     *
     * 相机位置: (0, 0, DEFAULT_CAMERA_Z)
     * 观察点: (0, 0, 0)
     * 上方向: (0, 1, 0)
     */
    void lookAtDefault() {
        lookAt(0.0f, 0.0f, DEFAULT_CAMERA_Z,
               0.0f, 0.0f, 0.0f,
               0.0f, 1.0f, 0.0f);
    }

    /**
     * 摆放相机。
     * @param eyeX, eyeY, eyeZ 相机位置
     * @param centerX, centerY, centerZ 观察点位置
     * @param upX, upY, upZ 相机上方向向量
     */
    void lookAt(float eyeX, float eyeY, float eyeZ,
                float centerX, float centerY, float centerZ,
                float upX, float upY, float upZ) {
        viewMatrix = glm::lookAt(
                glm::vec3(eyeX, eyeY, eyeZ),
                glm::vec3(centerX, centerY, centerZ),
                glm::vec3(upX, upY, upZ)
        );
        markDirty();
    }

    /**
     * 设置正交投影，保持模型比例不变，使用居中裁剪策略确保内容完整显示。
     * @param near 近裁剪平面
     * @param far 远裁剪平面
     */
    void projectOrthogonally(float near = DEFAULT_NEAR, float far = DEFAULT_FAR) {
        // 安全检查
        if (worldHeight < EPSILON || worldWidth < EPSILON ||
            modelHeight < EPSILON || modelWidth < EPSILON) {
            return;
        }

        const float worldRatio = worldWidth / worldHeight;
        const float modelRatio = modelWidth / modelHeight;

        float left, right, bottom, top;

        // 简化逻辑：统一处理宽高比适配
        if (modelRatio > worldRatio) {
            // 模型比世界宽：高度方向需要缩放
            const float scale = modelRatio / worldRatio;
            left = -1.0f;
            right = 1.0f;
            bottom = -scale;
            top = scale;
        } else {
            // 世界比模型宽：宽度方向需要缩放
            const float scale = worldRatio / modelRatio;
            left = -scale;
            right = scale;
            bottom = -1.0f;
            top = 1.0f;
        }

        // 创建正交投影矩阵
        projectionMatrix = glm::ortho(left, right, bottom, top, near, far);
        markDirty();
    }

    /**
     * 重置模型矩阵为单位矩阵。
     */
    void resetModelToIdentity() {
        // 等价于：
        // [1.0, 0.0, 0.0, 0.0]
        // [0.0, 1.0, 0.0, 0.0]
        // [0.0, 0.0, 1.0, 0.0]
        // [0.0, 0.0, 0.0, 1.0]
        modelMatrix = glm::mat4(1.0f);
        markDirty();
    }

    /**
     * 重置视图矩阵为单位矩阵。
     */
    void resetViewToIdentity() {
        viewMatrix = glm::mat4(1.0f);
        markDirty();
    }

    /**
     * 重置投影矩阵为单位矩阵。
     */
    void resetProjectionToIdentity() {
        projectionMatrix = glm::mat4(1.0f);
        markDirty();
    }

    /**
     * 计算 MVP 矩阵。
     * 顺序: Projection × View × Model
     */
    void computeMVPMatrix() {
        if (isDirty) {
            mvpMatrix = projectionMatrix * viewMatrix * modelMatrix;
            isDirty = false;
        }
    }

    /**
     * 强制重新计算 MVP 矩阵。
     */
    void forceComputeMVPMatrix() {
        mvpMatrix = projectionMatrix * viewMatrix * modelMatrix;
        isDirty = false;
    }

    // Getter 方法 - 添加 const 修饰符

    /**
     * 获取模型矩阵指针 (const)。
     * @return 指向模型矩阵数据的常量指针
     */
    [[nodiscard]]  const float *getModelMatrixPtr() const {
        /*
         * value_ptr 将 GLM 矩阵/向量转换为原始 float 指针，用于与 OpenGL API 交互。
         *
         * 为什么需要 value_ptr？
         *      内存布局兼容：GLM 使用自己的类型系统，但 OpenGL 需要原始的 float 数组。
         *      数据连续性：确保矩阵数据在内存中是连续的。
         *      类型安全：避免手动类型转换的错误。
         */
        return glm::value_ptr(modelMatrix);
    }

    /**
     * 获取视图矩阵指针 (const)。
     * @return 指向视图矩阵数据的常量指针
     */
    [[nodiscard]] const float *getViewMatrixPtr() const {
        return glm::value_ptr(viewMatrix);
    }

    /**
     * 获取投影矩阵指针 (const)。
     * @return 指向投影矩阵数据的常量指针
     */
    [[nodiscard]] const float *getProjectionMatrixPtr() const {
        return glm::value_ptr(projectionMatrix);
    }

    /**
     * 获取 MVP 矩阵的拷贝。
     * @return MVP 矩阵的拷贝
     */
    [[nodiscard]]  const float *getMVPMatrix() const {
        return glm::value_ptr(mvpMatrix);
    }

private:
    /**
     * 标记矩阵状态为脏，需要重新计算。
     */
    void markDirty() {
        isDirty = true;
    }

    /**
     * 检查矩阵是否需要重新计算。
     * @return true 如果需要重新计算
     */
    [[nodiscard]] bool needsUpdate() const {
        return isDirty;
    }
};

#endif //AV_ANDROID_LEARNING_GLMVPMATRIX_HPP