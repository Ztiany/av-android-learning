package me.ztiany.androidav.opengl.jwopengl.filter

import androidx.annotation.CallSuper
import me.ztiany.androidav.opengl.jwopengl.gles2.GLProgram
import me.ztiany.androidav.opengl.jwopengl.gles2.GLTexture
import me.ztiany.androidav.opengl.jwopengl.gles2.TextureAttribute
import timber.log.Timber

abstract class BaseGLFilter : GLFilter {

    private var glProgram: GLProgram? = null

    final override fun initProgram() {
        if (glProgram == null) {
            glProgram = onCreateProgram()
        }
    }

    protected abstract fun onCreateProgram(): GLProgram

    override fun setWorldSize(width: Int, height: Int) = Unit

    override fun setTextureAttribute(attribute: TextureAttribute) = Unit

    override fun onDrawFrame(sharedTexture: GLTexture): GLTexture {
        try {
            initProgram()
        } catch (e: Exception) {
            Timber.e("initProgram error: ${e.message}")
        }

        return glProgram?.doDraw(sharedTexture) ?: sharedTexture
    }

    protected abstract fun GLProgram.doDraw(sharedTexture: GLTexture): GLTexture

    @CallSuper
    override fun release() {
        glProgram?.delete()
        glProgram = null
    }

}