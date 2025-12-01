#include "../common/GLRenderer.h"
#include "../opengl/glsl2/Program.hpp"
#include "../opengl/glsl2/GlMVPMatrix.hpp"

class TriangleRenderer : public GLRenderer {
private:
    Program *program = nullptr;
    GlMVPMatrix *mvpMatrix = nullptr;

    float positions[12] = {
            0.0F, 0.5F, 0.0F, 1,  // top
            -0.5F, -0.5F, 0.0F, 1, // bottom left
            0.5F, -0.5F, 0.0F, 1, // bottom right
    };//x,y,z,w

    float colors[12] = {
            1.0F, 0.0F, 0.0F, 1.0F,
            0.0F, 1.0F, 0.0F, 1.0F,
            0.0F, 0.0F, 1.0F, 1.0F,
    };//r,g,b,a
public:

    static const int TYPE = 2;

    void onSurfaceCreated() override {
        program = Program::fromAssets(
                "shader/vertex_mvp_separated.glsl",
                "shader/fragment_coloring.glsl"
        );

        program->activeAttribute("aPosition");
        program->activeAttribute("aColor");

        program->activeUniform("uModelMatrix");
        program->activeUniform("uViewMatrix");
        program->activeUniform("uProjectionMatrix");

        mvpMatrix = new GlMVPMatrix();
    }

    void onSurfaceChanged(int width, int height) override {
        glViewport(0, 0, width, height);
        // 设置视口大小。
        mvpMatrix->setWorldSize((float) width, (float) height);
        // 这里是绘制三角形，所以模型大小设置为视口大小。
        mvpMatrix->setModelSize((float) width, (float) height);
        // 摆放相机
        mvpMatrix->lookAtDefault();
        // 正交投影
        mvpMatrix->projectOrthogonally();
    }

    void onDrawFrame(void *data) override {
        program->startDraw([
                                   model = mvpMatrix->getModelMatrixPtr(),
                                   view = mvpMatrix->getViewMatrixPtr(),
                                   projection = mvpMatrix->getProjectionMatrixPtr(),
                                   positions = this->positions,
                                   colors = this->colors
                           ](Program &program) {

            program.uniformMatrix4fv("uModelMatrix", model);
            program.uniformMatrix4fv("uViewMatrix", view);
            program.uniformMatrix4fv("uProjectionMatrix", projection);

            program.vertexAttribPointerFloat("aPosition", 4, positions);
            program.vertexAttribPointerFloat("aColor", 4, colors);

            glDrawArrays(GL_TRIANGLES, 0, 3);
        });
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