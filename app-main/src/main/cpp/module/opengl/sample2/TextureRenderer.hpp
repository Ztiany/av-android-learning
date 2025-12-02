#include "../common/GLRenderer.h"
#include "../opengl/glsl2/GlProgram.hpp"
#include "../opengl/glsl2/GlMVPMatrix.hpp"
#include "../opengl/glsl2/GlBO.hpp"
#include "../opengl/glsl2/GlTexture.hpp"

class TextureRenderer : public GLRenderer {
private:
    GlProgram *program = nullptr;
    GlMVPMatrix *mvpMatrix = nullptr;
    GlBO *positionVbo = nullptr;
    GlBO *coordinateVbo = nullptr;
    GlBO *ibo = nullptr;
    GlTexture *texture = nullptr;

public:
    static constexpr int TYPE = 5;

    ~TextureRenderer() override {
        release();
    }

    void onSurfaceCreated() override {
        program = GlProgram::fromAssets(
                "shader/vertex_mvp_separated.glsl",
                "shader/fragment_texture.glsl"
        );

        program->activeAttribute("aPosition");
        program->activeAttribute("aTextureCoordinate");
        program->activeUniform("uModelMatrix");
        program->activeUniform("uViewMatrix");
        program->activeUniform("uProjectionMatrix");
        program->activeUniform("uTexture");

        mvpMatrix = new GlMVPMatrix();

        auto positions = createVertexCoordinateFull4();
        for (auto &val: positions) {
            val *= 0.5F;
        }
        for (auto &val: positions) {
            LOGD("position value: %f", val);
        }
        positionVbo = GlBO::createVBO(
                sizeof(float) * positions.size(),
                positions.data(),
                GL_STATIC_DRAW
        );

        auto indexes = createRectangleIndices();
        ibo = GlBO::createIBO(
                sizeof(unsigned short) * indexes.size(),
                indexes.data(),
                GL_STATIC_DRAW
        );

        auto coordinate = createTextureCoordinateAndroid();
        coordinateVbo = GlBO::createVBO(
                sizeof(float) * coordinate.size(),
                coordinate.data(),
                GL_STATIC_DRAW
        );

        texture = GlTexture::fromFile("image/knight.png");
    }

    void onSurfaceChanged(int width, int height) override {
        glViewport(0, 0, width, height);

        mvpMatrix->setWorldSize(width, height);
        mvpMatrix->setModelSize(texture->getWidth(), texture->getHeight());
        mvpMatrix->lookAtDefault();
        mvpMatrix->projectOrthogonally();
    }

    void onDrawFrame(void *data) override {
        program->startDraw([
                                   model = mvpMatrix->getModelMatrixPtr(),
                                   view = mvpMatrix->getViewMatrixPtr(),
                                   projection = mvpMatrix->getProjectionMatrixPtr(),
                                   texture = this->texture,
                                   positionVbo = this->positionVbo,
                                   textureCoordinateVbo = this->coordinateVbo,
                                   ibo = this->ibo
                           ](GlProgram &program) {
            GlProgram::setBgColor(0, 0, 0, 1);
            GlProgram::clearBuffer();

            program.uniformMatrix4fv("uModelMatrix", model);
            program.uniformMatrix4fv("uViewMatrix", view);
            program.uniformMatrix4fv("uProjectionMatrix", projection);

            positionVbo->bind();
            program.vertexAttribPointerFloat("aPosition", 4, nullptr);
            positionVbo->unbind();

            textureCoordinateVbo->bind();
            program.vertexAttribPointerFloat("aTextureCoordinate", 2, nullptr);
            textureCoordinateVbo->unbind();

            texture->activate(program.activeUniform("uTexture"), 0);

            ibo->bind();
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, nullptr);
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
        if (positionVbo) {
            delete positionVbo;
            positionVbo = nullptr;
        }
        if (coordinateVbo) {
            delete coordinateVbo;
            coordinateVbo = nullptr;
        }
        if (ibo) {
            delete ibo;
            ibo = nullptr;
        }
        if (texture) {
            delete texture;
            texture = nullptr;
        }
    }

};