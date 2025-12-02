#include "Glsl2Util.h"
#include "../common/resources.h"
#include "../common/log.h"
#include "stb_image_aug.h"

GLuint compileShader(GLenum shaderType, const char *sourceCode) {
    GLuint shader_to_ret = glCreateShader(shaderType);
    glShaderSource(shader_to_ret, 1, &sourceCode, nullptr);
    glCompileShader(shader_to_ret);

    GLint compile_result = GL_TRUE;
    glGetShaderiv(shader_to_ret, GL_COMPILE_STATUS, &compile_result);
    if (compile_result == GL_FALSE) {
        char szLog[1024] = {0};
        GLsizei logLen = 0;
        glGetShaderInfoLog(shader_to_ret, 1024, &logLen, szLog);
        LOGE("compileShader %s, code:%s", szLog, sourceCode);
        glDeleteShader(shader_to_ret);
        shader_to_ret = 0;
    }

    return shader_to_ret;
}

GLuint createProgramFromShaders(GLuint vertexShader, GLuint fragmentShader) {
    GLuint program_to_ret = glCreateProgram();

    glAttachShader(program_to_ret, vertexShader);
    glAttachShader(program_to_ret, fragmentShader);

    glLinkProgram(program_to_ret);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    GLint link_result = GL_TRUE;
    glGetProgramiv(program_to_ret, GL_LINK_STATUS, &link_result);
    if (GL_FALSE == link_result) {
        char szLog[1024] = {0};
        GLsizei logLen = 0;
        glGetProgramInfoLog(program_to_ret, 1024, &logLen, szLog);
        LOGE("createProgramFromShaders %s\n", szLog);
        glDeleteProgram(program_to_ret);
        program_to_ret = 0;
    }
    return program_to_ret;
}

GLuint createProgramFromShaderCode(
        const char *vertexShaderCode,
        const char *fragmentShaderCode
) {

    GLuint vertexShader = compileShader(
            GL_VERTEX_SHADER,
            (char *) vertexShaderCode);

    GLuint fragmentShader = compileShader(
            GL_FRAGMENT_SHADER,
            (char *) fragmentShaderCode
    );

    GLuint program = createProgramFromShaders(vertexShader, fragmentShader);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return program;
}

GLuint createProgramFromAssets(
        const char *vertexShaderPath,
        const char *fragmentShaderPath
) {
    size_t fileSize = 0;

    unsigned char *assetContent = loadAssetContent(vertexShaderPath, fileSize);
    GLuint vertexShader = compileShader(
            GL_VERTEX_SHADER,
            (char *) assetContent
    );
    delete[] assetContent;

    assetContent = loadAssetContent(fragmentShaderPath, fileSize);
    GLuint fragmentShader = compileShader(
            GL_FRAGMENT_SHADER,
            (char *) assetContent
    );
    delete[] assetContent;

    GLuint program = createProgramFromShaders(vertexShader, fragmentShader);
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return program;
}

GLuint createTexture2D(void *pixel, int width, int height, GLint gpuFormat, GLenum cpuFormat) {
    // 生成一个纹理对象
    GLuint texture;
    glGenTextures(1, &texture);
    // 选中这个纹理对象，后续对纹理的操作都会作用到这个纹理对象上。
    glBindTexture(GL_TEXTURE_2D, texture);

    // 设置纹理缩放过滤
    //  GL_NEAREST: 使用纹理中坐标最接近的一个像素的颜色作为需要绘制的像素颜色
    //  GL_LINEAR: 使用纹理中坐标最接近的若干个颜色，通过加权平均算法得到需要绘制的像素颜色，速度较慢，但视觉效果好
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);// 最大
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);// 最小
    // 纹理坐标的范围是 0-1。超出这一范围的坐标将被 OpenGL 根据 GL_TEXTURE_WRAP 参数的值进行处理
    //  GL_TEXTURE_WRAP_S, GL_TEXTURE_WRAP_T 分别为 x，y 方向。
    //  GL_REPEAT：平铺。
    //  GL_MIRRORED_REPEAT：纹理坐标是奇数时使用镜像平铺。
    //  GL_CLAMP_TO_EDGE：坐标超出部分被截取成 0、1，边缘拉伸。
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);// S 轴
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);// T 轴

    glTexImage2D(
            // 给 GL_TEXTURE_2D 发送数据，而此时 GL_TEXTURE_2D 绑定的是上面的 texture。
            GL_TEXTURE_2D,
            // 给 0 号 Level 发送，一般 2D 绘图就是 0，3D 才会用到多个 Level。
            0,
            gpuFormat,
            width,
            height,
            0,
            cpuFormat,
            // 每个点分量的数据类型。
            GL_UNSIGNED_BYTE,
            // 数据
            pixel
    );
    // 解绑纹理对象。
    glBindTexture(GL_TEXTURE_2D, 0);
    return texture;
}

TextureResult createTextureFromFile(const char *path, bool fromAssets) {
    size_t fileSize = 0;
    unsigned char *content = nullptr;
    if (fromAssets) {
        content = loadAssetContent(path, fileSize);
    } else {
        content = loadFileContent(path, fileSize);
    }
    if (content == nullptr || fileSize == 0) {
        LOGE("createTextureFromFile: failed to load file %s", path);
        return {};
    }

    int image_width, image_height, channel_count;
    unsigned char *pixel = nullptr;
    if (strcmp(path + (strlen(path) - 4), ".png") == 0) {
        pixel = stbi_png_load_from_memory(
                content,
                static_cast<int>(fileSize),
                &image_width,
                &image_height,
                &channel_count,
                0
        );
    } else if (strcmp(path + (strlen(path) - 4), ".bmp") == 0) {
        pixel = stbi_bmp_load_from_memory(
                content,
                static_cast<int>(fileSize),
                &image_width,
                &image_height,
                &channel_count,
                0
        );
    } else {
        delete[] content;
        return {};
    }
    LOGD("createTextureFromFile: size = %dx%d, channel = %d, size = %d",
         image_width, image_height, channel_count, fileSize
    );
    GLint pixelFormat = channel_count == 3 ? GL_RGB : GL_RGBA;
    GLuint texture = createTexture2D(
            pixel,
            image_width,
            image_height,
            pixelFormat,
            pixelFormat
    );
    delete[] pixel;
    delete[] content;
    return {image_width, image_height, channel_count, texture};
}


std::array<float, 16> createVertexCoordinateFull4() {
    return {
            -1.0F, -1.0F, 0.0F, 1.0F,  // left-bottom
            1.0F, -1.0F, 0.0F, 1.0F,  // right-bottom
            -1.0F, 1.0F, 0.0F, 1.0F,  // left-top
            1.0F, 1.0F, 0.0F, 1.0F   // right-top
    };
}

std::array<float, 12> createVertexCoordinateFull3() {
    return {
            -1.0F, -1.0F, 0.0F,  // left-bottom
            1.0F, -1.0F, 0.0F,  // right-bottom
            -1.0F, 1.0F, 0.0F,  // left-top
            1.0F, 1.0F, 0.0F   // right-top
    };
}

std::array<unsigned short, 6> createRectangleIndices() {
    return {
            0, 1, 2,
            2, 3, 1
    };
}

std::array<float, 8> createTextureCoordinateStandard() {
    return {
            0.0F, 0.0F, // left-bottom
            1.0F, 0.0F, // right-bottom
            0.0F, 1.0F, // left-top
            1.0F, 1.0F  // right-top
    };
}

std::array<float, 8> createTextureCoordinateAndroid() {
    return {
            0.0F, 1.0F, // left-bottom
            1.0F, 1.0F, // right-bottom
            0.0F, 0.0F, // left-top
            1.0F, 0.0F  // right-top
    };
}