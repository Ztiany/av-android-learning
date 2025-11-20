package me.ztiany.androidav.opengl.jwopengl.filter

import me.ztiany.androidav.opengl.jwopengl.gles2.GLTexture
import me.ztiany.androidav.opengl.jwopengl.gles2.TextureAttribute

interface GLFilter {

    fun initProgram()

    fun setWorldSize(width: Int, height: Int)

    fun setTextureAttribute(attribute: TextureAttribute)

    fun onDrawFrame(sharedTexture: GLTexture): GLTexture

    fun release()

}