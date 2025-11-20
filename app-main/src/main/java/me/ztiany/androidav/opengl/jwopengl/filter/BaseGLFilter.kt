package me.ztiany.androidav.opengl.jwopengl.filter

import androidx.annotation.CallSuper
import me.ztiany.androidav.opengl.jwopengl.gles2.GLProgram
import me.ztiany.androidav.opengl.jwopengl.gles2.GLTexture
import me.ztiany.androidav.opengl.jwopengl.gles2.TextureAttribute

abstract class BaseGLFilter : GLFilter {

    private lateinit var _glProgram: GLProgram

    protected val glProgram: GLProgram
        get() = _glProgram

    override fun initProgram() {
        if (!this::_glProgram.isInitialized) {
            _glProgram = onCreateProgram()
        }
    }

    protected abstract fun onCreateProgram(): GLProgram

    override fun setWorldSize(width: Int, height: Int) = Unit

    override fun setTextureAttribute(attribute: TextureAttribute) = Unit

    override fun onDrawFrame(sharedTexture: GLTexture): GLTexture {
        initProgram()
        return doDraw(sharedTexture)
    }

    protected abstract fun doDraw(sharedTexture: GLTexture): GLTexture

    @CallSuper
    override fun release() {
        if (this::_glProgram.isInitialized) {
            _glProgram.delete()
        }
    }

}