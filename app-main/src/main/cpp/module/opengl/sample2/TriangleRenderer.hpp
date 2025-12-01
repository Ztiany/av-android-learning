#include "../common/GLRenderer.h"
#include "../opengl/glsl2/GlProgram.hpp"
#include "../opengl/glsl2/GlMVPMatrix.hpp"

class TriangleRenderer : public GLRenderer {
private:
    GlProgram *program = nullptr;           // 着色器程序指针
    GlMVPMatrix *mvpMatrix = nullptr;     // MVP 矩阵管理器指针

    // 三角形顶点位置数据 (x,y,z,w) - 齐次坐标
    float positions[12] = {
            0.0F, 0.5F, 0.0F, 1,  // 顶部顶点
            -0.5F, -0.5F, 0.0F, 1, // 左下顶点
            0.5F, -0.5F, 0.0F, 1, // 右下顶点
    };

    // 顶点颜色数据 (r,g,b,a) - RGBA格式
    float colors[12] = {
            1.0F, 0.0F, 0.0F, 1.0F,  // 红色
            0.0F, 1.0F, 0.0F, 1.0F,  // 绿色
            0.0F, 0.0F, 1.0F, 1.0F,  // 蓝色
    };

public:
    static constexpr int TYPE = 2;  // 渲染器类型标识符

    ~TriangleRenderer() override {
        release();
    }

    void onSurfaceCreated() override {
        // 从 assets 加载着色器程序并编译链接
        program = GlProgram::fromAssets(
                "shader/vertex_mvp_separated.glsl",      // 顶点着色器路径
                "shader/fragment_coloring.glsl"          // 片元着色器路径
        );

        // 激活顶点属性变量位置，缓存其位置句柄
        program->activeAttribute("aPosition");  // 位置属性
        program->activeAttribute("aColor");     // 颜色属性

        // 激活 uniform 变量位置，缓存其位置句柄
        program->activeUniform("uModelMatrix");      // 模型矩阵
        program->activeUniform("uViewMatrix");       // 视图矩阵
        program->activeUniform("uProjectionMatrix"); // 投影矩阵

        // 创建 MVP 矩阵管理器实例
        mvpMatrix = new GlMVPMatrix();
    }

    void onSurfaceChanged(int width, int height) override {
        // 设置 OpenGL 视口为整个窗口区域
        glViewport(0, 0, width, height);

        // 设置世界坐标系大小为视口大小
        mvpMatrix->setWorldSize((float) width, (float) height);

        // 将模型大小设置为视口大小，使三角形填满整个视口
        // 对于简单三角形来说，这是一种合理的缩放策略
        mvpMatrix->setModelSize((float) width, (float) height);

        // 使用默认相机位置设置视图矩阵
        // 相机位于Z轴正方向，看向原点
        mvpMatrix->lookAtDefault();

        // 设置正交投影矩阵
        // 对于 2D 图形通常使用正交投影，保持平行线平行
        mvpMatrix->projectOrthogonally();
    }

    void onDrawFrame(void *data) override {
        // 使用 Lambda 表达式作为绘制回调
        // C++11 引入的 Lambda 语法: [捕获列表](参数列表) -> 返回类型 { 函数体 }
        program->startDraw(
                [
                        // 捕获列表: 按值捕获局部变量，使其在lambda体内可用
                        model = mvpMatrix->getModelMatrixPtr(),
                        view = mvpMatrix->getViewMatrixPtr(),
                        projection = mvpMatrix->getProjectionMatrixPtr(),
                        positions = const_cast<float *>(this->positions),
                        colors = const_cast<float *>(this->colors)](GlProgram &program) {
                    // 清空缓冲区
                    program.clearBuffer();

                    // 将矩阵数据传递给着色器 uniform 变量
                    program.uniformMatrix4fv("uModelMatrix", model);      // 模型矩阵
                    program.uniformMatrix4fv("uViewMatrix", view);        // 视图矩阵
                    program.uniformMatrix4fv("uProjectionMatrix", projection); // 投影矩阵

                    // 设置顶点属性指针
                    program.vertexAttribPointerFloat(
                            "aPosition",
                            4,
                            positions
                    ); // 位置属性，4 个分量(x,y,z,w)
                    program.vertexAttribPointerFloat(
                            "aColor",
                            4,
                            colors
                    );     // 颜色属性，4 个分量(r,g,b,a)

                    // 执行绘制命令: 绘制三角形
                    // GL_TRIANGLES: 将顶点按照三个一组的方式绘制成三角形
                    glDrawArrays(GL_TRIANGLES, 0, 3);
                });
    }

    void onSurfaceDestroy() override {
        release();
    }

    void release() {
        // 资源清理: 释放动态分配的内存
        if (program) {
            delete program;     // 删除着色器程序对象
            program = nullptr;   // 避免悬垂指针
        }
        if (mvpMatrix) {
            delete mvpMatrix;   // 删除 MVP 矩阵对象
            mvpMatrix = nullptr; // 避免悬垂指针
        }
    }

};