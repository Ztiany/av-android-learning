#ifndef AV_ANDROID_LEARNING_GLTEXTURE_HPP
#define AV_ANDROID_LEARNING_GLTEXTURE_HPP

#include "Glsl2Util.h"
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <GLES2/gl2platform.h>

class GlTexture {
private:
    GLuint textureId = 0;
    int width = 0;
    int height = 0;

    GlTexture(
            GLuint textureId,
            int width,
            int height
    ) {
        this->textureId = textureId;
        this->width = width;
        this->height = height;
    }

public:
    ~GlTexture() {
        if (textureId != 0) {
            glDeleteTextures(1, &textureId);
            textureId = 0;
        }
    }

    int getWidth() const {
        return width;
    }

    int getHeight() const {
        return height;
    }

    void activate(GLint handle, GLint index = 0) const {
        glActiveTexture(GL_TEXTURE0 + index);
        glBindTexture(GL_TEXTURE_2D, textureId);
        glUniform1i(handle, index);
    }

    static void deactivate() {
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    static GlTexture *fromFile(const char *path, bool fromAssets = true) {
        TextureResult result = createTextureFromFile(path, fromAssets);
        if (result.textureId == 0) {
            return nullptr;
        }
        return new GlTexture(
                result.textureId,
                result.width,
                result.height
        );
    }

    static GlTexture *fromPixels(
            void *pixels,
            int width,
            int height,
            GLint gpuFormat,
            GLenum cpuFormat
    ) {
        GLuint textureId = createTexture2D(
                pixels,
                width,
                height,
                gpuFormat,
                cpuFormat
        );
        if (textureId == 0) {
            return nullptr;
        }
        return new GlTexture(
                textureId,
                width,
                height
        );
    }

};

#endif //AV_ANDROID_LEARNING_GLTEXTURE_HPP