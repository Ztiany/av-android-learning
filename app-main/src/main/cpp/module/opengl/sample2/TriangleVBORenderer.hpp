#include "../common/GLRenderer.h"
#include "../opengl/glsl2/GlProgram.hpp"
#include "../opengl/glsl2/GlMVPMatrix.hpp"
#include "../opengl/glsl2/GlVBO.hpp"

class TriangleVBORenderer : public GLRenderer {
private:
    GlProgram *program = nullptr;           // 着色器程序指针
    GlMVPMatrix *mvpMatrix = nullptr;     // MVP 矩阵管理器指针
    GlVBO *vbo = nullptr;                 // 顶点缓冲对象指针

    // 三角形顶点位置数据 (x,y,z,w) + 颜色数据 (r,g,b,a) - 存储在同一个 VBO 中。
    float vboData[24] = {
            0.0F, 0.5F, 0.0F, 1,  // 顶部顶点
            1.0F, 0.0F, 0.0F, 1.0F,  // 红色

            -0.5F, -0.5F, 0.0F, 1, // 左下顶点
            0.0F, 1.0F, 0.0F, 1.0F,  // 绿色

            0.5F, -0.5F, 0.0F, 1, // 右下顶点
            0.0F, 0.0F, 1.0F, 1.0F,  // 蓝色
    };

public:
    static constexpr int TYPE = 3;  // 渲染器类型标识符

    ~TriangleVBORenderer() override {
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

        // 创建 VBO 并初始化数据
        vbo = GlVBO::create(
                sizeof(vboData),
                vboData,
                GL_STATIC_DRAW
        );
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
        program->startDraw([
                                   // 捕获列表: 按值捕获局部变量，使其在lambda体内可用
                                   model = mvpMatrix->getModelMatrixPtr(),
                                   view = mvpMatrix->getViewMatrixPtr(),
                                   projection = mvpMatrix->getProjectionMatrixPtr(),
                                   vbo = this->vbo
                                   // 参数列表: 接收Program对象的引用
                           ](GlProgram &program) {
            program.clearBuffer();

            // 将矩阵数据传递给着色器 uniform 变量
            program.uniformMatrix4fv("uModelMatrix", model);      // 模型矩阵
            program.uniformMatrix4fv("uViewMatrix", view);        // 视图矩阵
            program.uniformMatrix4fv("uProjectionMatrix", projection); // 投影矩阵

            // 绑定 VBO
            vbo->bind();
            // 设置如何从 VBO 中读取数据：顶点数据
            program.vertexAttribPointerFloat(
                    "aPosition",
                    4,
                    nullptr,
                    sizeof(float) * 8
            );
            // 设置如何从 VBO 中读取数据：颜色数据
            program.vertexAttribPointerFloat(
                    "aColor",
                    // 一个颜色有多少个组成部分，argb 有 4 个组成部分。
                    4,
                    // 当使用 vbo 时，这里的指针是一个偏移量，即从偏移位置开始读取数据。
                    (void *) (sizeof(float) * 4),
                    // 每读取完一个顶点的数据，跳过多少字节，才能到达下一个顶点的数据位置。
                    sizeof(float) * 8
            );
            // 解绑 VBO
            vbo->unbind();

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
        if (vbo) {
            delete vbo;         // 删除 VBO 对象
            vbo = nullptr;      // 避免悬垂指针
        }
    }

};