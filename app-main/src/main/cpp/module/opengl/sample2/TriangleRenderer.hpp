#include "../common/GLRenderer.h"
#include "../opengl/glsl2/Program.h"

class TriangleRenderer : public GLRenderer {
private:
    Program *program = nullptr;

public:

    static const int TYPE = 2;

    void onSurfaceCreated() override {
        // 设置清屏颜色为深蓝色
        glClearColor(0.0F, 0.0F, 0.3F, 1.0F);
    }

    void onSurfaceChanged(int width, int height) override {
        glViewport(0, 0, width, height);
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
    }

};