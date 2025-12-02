#include "../common/GLRenderer.h"
#include "../opengl/glsl2/GlProgram.hpp"
#include "../opengl/glsl2/GlMVPMatrix.hpp"
#include "../opengl/glsl2/GlBO.hpp"

class TriangleVBORenderer : public GLRenderer {
private:
    GlProgram *program = nullptr;
    GlMVPMatrix *mvpMatrix = nullptr;
    GlBO *vbo = nullptr;

    // 三角形顶点位置数据 (x,y,z,w) + 颜色数据 (r,g,b,a) - 存储在同一个 VBO 中。
    float vboData[24]{
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
        program = GlProgram::fromAssets(
                "shader/vertex_mvp_separated.glsl",
                "shader/fragment_coloring.glsl"
        );

        program->activeAttribute("aPosition");
        program->activeAttribute("aColor");

        program->activeUniform("uModelMatrix");
        program->activeUniform("uViewMatrix");
        program->activeUniform("uProjectionMatrix");

        mvpMatrix = new GlMVPMatrix();

        vbo = GlBO::createVBO(
                sizeof(vboData),
                vboData,
                GL_STATIC_DRAW
        );
    }

    void onSurfaceChanged(int width, int height) override {
        glViewport(0, 0, width, height);
        mvpMatrix->setWorldSize(width, height);
        mvpMatrix->setModelSize(width, height);
        mvpMatrix->lookAtDefault();
        mvpMatrix->projectOrthogonally();
    }

    void onDrawFrame(void *data) override {
        program->startDraw([
                                   model = mvpMatrix->getModelMatrixPtr(),
                                   view = mvpMatrix->getViewMatrixPtr(),
                                   projection = mvpMatrix->getProjectionMatrixPtr(),
                                   vbo = this->vbo
                           ](GlProgram &program) {
            GlProgram::clearBuffer();

            program.uniformMatrix4fv("uModelMatrix", model);
            program.uniformMatrix4fv("uViewMatrix", view);
            program.uniformMatrix4fv("uProjectionMatrix", projection);

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
                    // 当使用 positionVbo 时，这里的指针是一个偏移量，即从偏移位置开始读取数据。
                    (void *) (sizeof(float) * 4),
                    // 每读取完一个顶点的数据，跳过多少字节，才能到达下一个顶点的数据位置。
                    sizeof(float) * 8
            );
            // 解绑 VBO
            vbo->unbind();

            glDrawArrays(GL_TRIANGLES, 0, 3);
        });
    }

    void onSurfaceDestroy() override {
        release();
    }

    void release() {
        if (program) {
            delete program;
            program = nullptr;
        }
        if (mvpMatrix) {
            delete mvpMatrix;
            mvpMatrix = nullptr;
        }
        if (vbo) {
            delete vbo;
            vbo = nullptr;
        }
    }

};