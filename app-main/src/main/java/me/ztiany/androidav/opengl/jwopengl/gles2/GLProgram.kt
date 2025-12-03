package me.ztiany.androidav.opengl.jwopengl.gles2

import android.opengl.GLES20
import me.ztiany.lib.avbase.utils.FileUtils
import timber.log.Timber
import java.nio.FloatBuffer

class GLProgram private constructor(
    vertexSource: String,
    fragmentSource: String
) {

    private val programHandle = generateGLProgram(vertexSource, fragmentSource)

    private val attributeMap = mutableMapOf<String, Int>()

    private val enabledAttribute = mutableSetOf<Int>()

    private val uniformMap = mutableMapOf<String, Int>()

    companion object {

        @Throws(IllegalArgumentException::class)
        fun fromAssets(vertexPath: String, fragmentPath: String) = GLProgram(
            FileUtils.loadAssets(vertexPath),
            FileUtils.loadAssets(fragmentPath)
        )
    }

    fun activeAttribute(attributeName: String) {
        val attribLocation = GLES20.glGetAttribLocation(programHandle, attributeName)
        attributeMap[attributeName] = attribLocation
    }

    fun activeUniform(uniformName: String) {
        val attribLocation = GLES20.glGetUniformLocation(programHandle, uniformName)
        uniformMap[uniformName] = attribLocation
    }

    fun setBgColor(red: Float, green: Float, blue: Float, alpha: Float) {
        GLES20.glClearColor(red, green, blue, alpha)
    }

    fun clearColorBuffer() {
        GLES20.glClear(GLES20.GL_COLOR_BUFFER_BIT)
    }

    fun clearBuffer(bufferMask: Int) {
        GLES20.glClear(bufferMask)
    }

    fun startDraw(onDraw: GLProgram.() -> Unit) {
        GLES20.glUseProgram(programHandle)
        enabledAttribute.clear()
        onDraw(this)
        enabledAttribute.forEach {
            GLES20.glDisableVertexAttribArray(it)
        }
        enabledAttribute.clear()
        GLES20.glUseProgram(0)
    }

    fun drawArraysStrip(count: Int) {
        GLES20.glDrawArrays(GLES20.GL_TRIANGLE_STRIP, 0, count)
    }

    /**
     * - elementsPerVertex: 每个点几个 float。
     */
    fun vertexAttribPointerFloat(attribute: String, elementsPerVertex: Int, vbo: FloatBuffer) {
        // glVertexAttribPointer 函数在这里用于向 shader 传递顶点数据。
        val attributeHandle = attributeHandle(attribute)
        GLES20.glVertexAttribPointer(
            attributeHandle,
            elementsPerVertex,
            GLES20.GL_FLOAT,
            false,
            elementsPerVertex * 4/* 每个点 4 个 float，每个 float 4 个 byte */,
            vbo
        )

        GLES20.glEnableVertexAttribArray(attributeHandle)
        enabledAttribute.add(attributeHandle)
    }

    /**
     * - elementsPerVertex: 每个点几个 float。
     */
    fun vertexAttribPointerInt(attribute: String, elementsPerVertex: Int, vbo: FloatBuffer) {
        // glVertexAttribPointer 函数在这里向 shader 传递 int 数据。
        val attributeHandle = attributeHandle(attribute)

        GLES20.glVertexAttribPointer(
            attributeHandle,
            elementsPerVertex,
            GLES20.GL_INT,
            false,
            elementsPerVertex * 4/* 每个点 4 个 float，每个 float 4 个 byte */,
            vbo
        )

        GLES20.glEnableVertexAttribArray(attributeHandle)
        enabledAttribute.add(attributeHandle)
    }

    fun uniformMatrix4fv(uniformName: String, matrix: FloatArray) {
        GLES20.glUniformMatrix4fv(
            uniformHandle(uniformName),
            1,
            false,
            matrix,
            0
        )
    }

    fun uniform1f(uniformName: String, value: Float) {
        GLES20.glUniform1f(uniformHandle(uniformName), value)
    }

    fun uniform1i(uniformName: String, value: Int) {
        GLES20.glUniform1i(uniformHandle(uniformName), value)
    }

    @Suppress("MemberVisibilityCanBePrivate")
    fun attributeHandle(attribute: String) =
        attributeMap[attribute] ?: throw NoSuchElementException()

    fun uniformHandle(attribute: String) = uniformMap[attribute] ?: throw NoSuchElementException()

    fun delete() {
        Timber.d("release() called")
        GLES20.glDeleteProgram(programHandle)
    }

}