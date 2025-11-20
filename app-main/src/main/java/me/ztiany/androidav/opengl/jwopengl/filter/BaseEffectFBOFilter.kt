package me.ztiany.androidav.opengl.jwopengl.filter

import me.ztiany.androidav.opengl.jwopengl.gles2.*
import timber.log.Timber

abstract class BaseEffectFBOFilter : BaseGLFilter() {

    private var glFBO: GLFBOWithTexture? = null

    /** 矩形的坐标 */
    protected val vertexVbo = generateVBOBuffer(newVertexCoordinateFull3())

    /** 纹理坐标 */
    protected val textureCoordinateBuffer = generateVBOBuffer(newTextureCoordinateStandard())

    protected var textureWidth = 0
        private set

    protected var textureHeight = 0
        private set

    override fun setWorldSize(width: Int, height: Int) = Unit

    override fun setTextureAttribute(attribute: TextureAttribute) {
        this.textureWidth = attribute.width
        this.textureHeight = attribute.height
    }

    final override fun GLProgram.doDraw(sharedTexture: GLTexture): GLTexture {
        return getFBO(this).use {
            drawOnFBO(sharedTexture)
        }.texture
    }

    abstract fun GLProgram.drawOnFBO(sharedTexture: GLTexture)

    private fun getFBO(glProgram: GLProgram): GLFBOWithTexture {
        var fbo = glFBO

        if (fbo != null && (fbo.texture.width != textureWidth || fbo.texture.height != textureHeight)) {
            fbo.delete()
            fbo = null
        }

        if (fbo == null) {
            Timber.d("create new fbo $textureWidth x $textureHeight.")

            val glTexture = GLTexture.generateFBOTexture(
                glProgram.uniformHandle("uTexture"),
                0,
                //use the real texture size.
                textureWidth,
                textureHeight
            )
            fbo = GLFBOWithTexture.generate(glTexture)
            glFBO = fbo
        }

        return fbo
    }

    override fun release() {
        super.release()
        glFBO?.delete()
        glFBO = null
    }

}