package me.ztiany.androidav.opengl.jwopengl.filter

import android.opengl.GLES20
import me.ztiany.androidav.opengl.jwopengl.gles2.GLProgram
import me.ztiany.androidav.opengl.jwopengl.gles2.GLTexture
import me.ztiany.androidav.opengl.jwopengl.gles2.activeTexture

abstract class BaseEffectFilter : BaseEffectFBOFilter() {

    override fun createAndInitProgram(): GLProgram {
        val glProgram = createGLProgram()

        //vertex
        glProgram.activeAttribute("aPosition")
        glProgram.activeAttribute("aTextureCoordinate")

        //fragment
        glProgram.activeUniform("uTexture")

        return glProgram
    }

    protected abstract fun createGLProgram(): GLProgram

    override fun setWorldSize(width: Int, height: Int) {

    }

    override fun drawOnFBO(sharedTexture: GLTexture) {
        glProgram.startDraw {
            clearColorBuffer()
            GLES20.glViewport(0, 0, textureWidth, textureHeight)
            beforeDraw()
            vertexAttribPointerFloat("aPosition", 3, vertexVbo)
            vertexAttribPointerFloat("aTextureCoordinate", 2, textureCoordinateBuffer)
            //texture【将分享过来的纹理绘制到 FBO 的纹理上】
            sharedTexture.activeTexture(uniformHandle("uTexture"))
            //draw
            drawArraysStrip(4/*4 个顶点*/)
        }
    }

    protected open fun beforeDraw() {

    }

}