#include "../common/GLRenderer.h"
#include "../opengl/glsl2/Program.hpp"
#include "../opengl/glsl2/GlMVPMatrix.hpp"

class TriangleRenderer : public GLRenderer {
private:
    Program *program = nullptr;
    GlMVPMatrix *mvpMatrix = nullptr;

public:

    static const int TYPE = 2;

    void onSurfaceCreated() override {
        program = Program::fromAssets(
                "shader/vertex_mvp_separated.glsl",
                "shaders/fragment_color.glsl"
        );

        mvpMatrix = new GlMVPMatrix();
    }

    void onSurfaceChanged(int width, int height) override {
        glViewport(0, 0, width, height);
        // 设置视口大小。
        mvpMatrix->setWorldSize((float) width, (float) height);
        // 这里是绘制三角形，所以模型大小设置为视口大小。
        mvpMatrix->setModelSize((float) width, (float) height);
    }

    void onDrawFrame(void *data) override {
        // 清除颜色缓冲区
        glClear(GL_COLOR_BUFFER_BIT);

        // 这里可以添加绘制三角形的代码
    }

    void onSurfaceDestroy() override {
        if (program) {
            delete program;
            program = nullptr;
        }
        if (mvpMatrix) {
            delete mvpMatrix;
            mvpMatrix = nullptr;
        }
    }

};