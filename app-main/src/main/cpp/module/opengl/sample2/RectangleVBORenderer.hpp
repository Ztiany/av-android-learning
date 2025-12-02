#include "../common/GLRenderer.h"
#include "../opengl/glsl2/GlProgram.hpp"
#include "../opengl/glsl2/GlMVPMatrix.hpp"
#include "../opengl/glsl2/GlBO.hpp"

class RectangleVBORenderer : public GLRenderer {
private:
    GlProgram *program = nullptr;
    GlMVPMatrix *mvpMatrix = nullptr;
    GlBO *vbo = nullptr;
    GlBO *ibo = nullptr;

    // 四个顶点位置数据 (x,y,z,w) + 颜色数据 (r,g,b,a) - 存储在同一个 VBO 中。
    float vboData[32]{
            -0.5F, 0.5F, 0.0F, 1,  // 左上角
            1.0F, 0.0F, 0.0F, 1.0F,  // 红色

            0.5F, 0.5F, 0.0F, 1,  // 右上角
            1.0F, 1.0F, 1.0F, 1.0F,  // 白色

            -0.5F, -0.5F, 0.0F, 1, // 左下顶点
            0.0F, 1.0F, 0.0F, 1.0F,  // 绿色

            0.5F, -0.5F, 0.0F, 1, // 右下顶点
            0.0F, 0.0F, 1.0F, 1.0F,  // 蓝色
    };

    unsigned short indexes[6]{
            0, 1, 2,
            1, 3, 2
    };

public:
    static constexpr int TYPE = 4;  // 渲染器类型标识符

    ~RectangleVBORenderer() override {
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

        ibo = GlBO::createIBO(
                sizeof(indexes),
                indexes,
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
                                   vbo = this->vbo,
                                   ibo = this->ibo
                           ](GlProgram &program) {
            GlProgram::clearBuffer();

            program.uniformMatrix4fv("uModelMatrix", model);
            program.uniformMatrix4fv("uViewMatrix", view);
            program.uniformMatrix4fv("uProjectionMatrix", projection);

            vbo->bind();
            program.vertexAttribPointerFloat(
                    "aPosition",
                    4,
                    nullptr,
                    sizeof(float) * 8
            );
            program.vertexAttribPointerFloat(
                    "aColor",
                    4,
                    (void *) (sizeof(float) * 4),
                    sizeof(float) * 8
            );
            vbo->unbind();

            ibo->bind();
            glDrawElements(
                    GL_TRIANGLES,
                    6,
                    GL_UNSIGNED_SHORT,
                    nullptr
            );
            ibo->unbind();
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
        if (ibo) {
            delete ibo;
            ibo = nullptr;
        }
    }

};