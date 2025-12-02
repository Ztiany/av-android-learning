#ifndef AV_ANDROID_LEARNING_GLSL2UTIL_H
#define AV_ANDROID_LEARNING_GLSL2UTIL_H

#include <array>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <GLES2/gl2platform.h>

/**
 * 编译着色器代码。
 * @param shaderType  GL_VERTEX_SHADER 或 GL_FRAGMENT_SHADER。
 * @param sourceCode  着色器源码字符串。
 * @return
 */
GLuint compileShader(GLenum shaderType, const char *sourceCode);

/**
 * 通过顶点着色器和片元着色器创建一个程序对象。
 * @param vertexShader 顶点着色器对象。
 * @param fragmentShader 片元着色器对象。
 * @return 程序对象。
 */
GLuint createProgramFromShaders(GLuint vertexShader, GLuint fragmentShader);

/**
 * 通过顶点着色器代码和片元着色器代码创建一个程序对象。
 * @param vertexShaderCode 顶点着色器代码。
 * @param fragmentShaderCode 片元着色器代码。
 * @return 程序对象。
 */
GLuint createProgramFromShaderCode(
        const char *vertexShaderCode,
        const char *fragmentShaderCode
);

/**
 * 从 assets 加载顶点着色器和片元着色器，编译并链接成一个程序对象。
 * @param vertexShaderPath 顶点着色器在 assets 中的路径。
 * @param fragmentShaderPath 片元着色器在 assets 中的路径。
 * @return 程序对象。
 */
GLuint createProgramFromAssets(
        const char *vertexShaderPath,
        const char *fragmentShaderPath
);

/**
 * 创建一个 2D 纹理对象。
 * @param pixel 纹理像素数据。
 * @param width 纹理宽度。
 * @param height 纹理高度。
 * @param gpuFormat GPU 端使用的纹理格式，如 GL_RGB、GL_RGBA。
 * @param cpuFormat CPU 端使用的像素数据格式，如 GL_RGB、GL_RGBA。
 * @return 纹理对象 ID。
 */
GLuint createTexture2D(void *pixel, int width, int height, GLint gpuFormat, GLenum cpuFormat);

struct TextureResult {
    int width = 0;
    int height = 0;
    int channels = 0;
    GLuint textureId = 0;
};

/**
 * 从文件创建一个 2D 纹理对象。
 * @param path 文件路径。
 * @param fromAssets 是否从 assets 加载。
 * @return 纹理结果，包括宽度、高度、通道数和纹理 ID。
 */
TextureResult createTextureFromFile(const char *path, bool fromAssets = true);

/**
 * 返回一个矩形的顶点数组，每个点 4 个元素 (x, y, z, w)。
 * @return  矩形顶点数组，共 16 个元素，类型为 float。
 */
std::array<float, 16> createVertexCoordinateFull4();

/**
 * 返回一个矩形的顶点数组，每个点 3 个元素 (x, y, z)。
 * @return 矩形顶点数组，共 12 个元素，类型为 float。
 */
std::array<float, 12> createVertexCoordinateFull3();

/**
 * 返回矩形的索引数组（6 个索引，构成 2 个三角形）。
 * @return 矩形索引数组，共 6 个元素，类型为 unsigned short。
 */
std::array<unsigned short, 6> createRectangleIndices();

/**
 * 返回标准的纹理坐标数组（每个点 s, t）。
 * @return 纹理坐标数组，共 8 个元素，类型为 float。
 */
std::array<float, 8> createTextureCoordinateStandard();

/**
 * 返回Android（OpenGL ES）惯用的纹理坐标数组（Y 轴翻转）。
 * @return 纹理坐标数组，共 8 个元素，类型为 float。
 */
std::array<float, 8> createTextureCoordinateAndroid();

#endif