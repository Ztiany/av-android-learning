package me.ztiany.androidav.opengl.jwopengl.renderer

import android.content.Context
import android.graphics.SurfaceTexture
import android.opengl.GLES11Ext
import android.opengl.GLES20
import android.opengl.Matrix
import androidx.core.content.ContextCompat
import me.ztiany.androidav.opengl.common.EGLBridge
import me.ztiany.androidav.opengl.common.GLRenderer
import me.ztiany.androidav.opengl.common.SurfaceTextureListener
import me.ztiany.androidav.opengl.jwopengl.gles2.*
import timber.log.Timber

class CameraRenderer(
    private val context: Context,
    private val eglBridge: EGLBridge
) : GLRenderer {

    private val glMVPMatrix = GLMVPMatrix()

    private var glProgram: GLProgram? = null

    private var glTexture: GLTexture? = null

    /** 用于修正相机的方向 */
    private var displayOrientation = 0

    private var isMirror = false

    /** 承载视频的纹理 */
    private var surfaceTexture: SurfaceTexture? = null

    /** 矩形的坐标 */
    private val vertexVbo = generateVBOBuffer(newVertexCoordinateFull3())

    /** 纹理坐标 */
    private val textureCoordinateBuffer = generateVBOBuffer(newTextureCoordinateAndroid())

    private var surfaceTextureListener: SurfaceTextureListener? = null

    fun listenToSurfaceTexture(surfaceTextureListener: SurfaceTextureListener?) {
        this.surfaceTextureListener = surfaceTextureListener
        dispatchSurfaceTexture()
    }

    private fun dispatchSurfaceTexture() {
        surfaceTexture?.let { st ->
            surfaceTextureListener?.onSurfaceTextureAvailable(st)
        }
    }

    override fun onContextInitialized() {
        Timber.d("onSurfaceCreated")

        val program = try {
            GLProgram.fromAssets(
                "shader/vertex_mvp.glsl",
                "shader/fragment_camera.glsl"
            )
        } catch (e: Exception) {
            Timber.e(e, "GLProgram create fail,check shader file")
            return
        }
        glProgram = program

        with(program) {
            activeAttribute("aPosition")
            activeAttribute("aTextureCoordinate")
            activeUniform("uTexture")
            activeUniform("uMVPModelMatrix")
        }

        val texture = GLTexture.generate(
            program.uniformHandle("uTexture"),
            0,
            GLES11Ext.GL_TEXTURE_EXTERNAL_OES
        )
        glTexture = texture

        surfaceTexture = SurfaceTexture(texture.id)
        ContextCompat.getMainExecutor(context).execute { dispatchSurfaceTexture() }
        surfaceTexture?.setOnFrameAvailableListener { eglBridge.requestRender() }
    }

    override fun onSurfaceChanged(width: Int, height: Int) {
        Timber.d("onSurfaceChanged")
        GLES20.glViewport(0, 0, width, height)
        glMVPMatrix.setWorldSize(width, height)
        adjustMatrix()
    }

    override fun onDrawFrame(attachment: Any?) {
        glProgram?.startDraw {
            clearColorBuffer()
            glTexture?.activeTexture()
            surfaceTexture?.updateTexImage()
            uniformMatrix4fv("uMVPModelMatrix", glMVPMatrix.mvpMatrix)
            vertexAttribPointerFloat("aPosition", 3, vertexVbo)
            vertexAttribPointerFloat("aTextureCoordinate", 2, textureCoordinateBuffer)
            drawArraysStrip(4/* 4 个顶点 */)
        }
    }

    override fun onContextDestroy() {
        Timber.d("onSurfaceDestroy")
        glProgram?.release()
        glProgram = null

        surfaceTexture?.setOnFrameAvailableListener(null)
        surfaceTexture?.let { surfaceTextureListener?.onSurfaceTextureToDestroy(it) }
        surfaceTexture = null

        glTexture?.deleteTexture()
        glTexture = null
    }

    fun setVideoAttribute(width: Int, height: Int, displayOrientation: Int, isMirror: Boolean) {
        Timber.d("setVideoAttribute")
        this.displayOrientation = displayOrientation
        this.isMirror = isMirror
        glMVPMatrix.setModelSize(width, height)
        adjustMatrix()
    }

    private fun adjustMatrix() {
        glMVPMatrix.lookAtNormally()
        glMVPMatrix.adjustToOrthogonal()
        glMVPMatrix.combineMVP()
        // 绕着 Z 轴旋转
        if (!isMirror) {
            // 后摄，一般情况下相机的画面被逆时针转了 90 度，因此这里也将顶点坐标转同样的角度，再去纹理采样
            // 注意【顶点是先插值，然后我们利用矩阵再将顶点修正到正确的采样进行位置】。
            Matrix.rotateM(glMVPMatrix.mvpMatrix, 0, -this.displayOrientation.toFloat(), 0F, 0F, 1F)
        } else {
            Matrix.scaleM(glMVPMatrix.mvpMatrix, 0, -1F, 1F, 1F)
            Matrix.rotateM(glMVPMatrix.mvpMatrix, 0, this.displayOrientation.toFloat(), 0F, 0F, 1F)
        }
    }

}