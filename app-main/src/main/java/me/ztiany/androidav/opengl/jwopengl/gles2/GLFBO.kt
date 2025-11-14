package me.ztiany.androidav.opengl.jwopengl.gles2

import android.opengl.GLES20

class GLFBO(val name: Int) {
    override fun toString(): String {
        return "GLFBO(name=$name)"
    }
}

fun generateFBO(): GLFBO {
    val frameBufferIds = intArrayOf(1)
    GLES20.glGenFramebuffers(1, frameBufferIds, 0)
    return GLFBO(frameBufferIds[0])
}
