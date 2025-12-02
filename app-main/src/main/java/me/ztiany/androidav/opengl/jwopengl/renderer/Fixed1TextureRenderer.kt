package me.ztiany.androidav.opengl.jwopengl.renderer

import android.opengl.GLES20
import me.ztiany.androidav.R
import me.ztiany.lib.avbase.utils.loadBitmap
import me.ztiany.androidav.opengl.jwopengl.gles2.*
import me.ztiany.androidav.opengl.common.GLRenderer
import timber.log.Timber

class Fixed1TextureRenderer : GLRenderer {

    private var glProgram: GLProgram? = null

    private var glTexture: GLTexture? = null

    private val glMVPMatrix by lazy { GLMVPMatrix() }

    /** 矩形的坐标 */
    private val vertexVbo = generateVBOBuffer(
        newVertexCoordinateFull3().map {
            it * 0.8F
        }.toFloatArray()
    )

    /** 纹理坐标 */
    private val textureCoordinateBuffer = generateVBOBuffer(newTextureCoordinateAndroid())

    override fun onContextInitialized() {
        val program = try {
            GLProgram.fromAssets(
                "shader/vertex_mvp.glsl",
                "shader/fragment_texture.glsl"
            )
        } catch (e: Exception) {
            Timber.e(e)
            return
        }
        glProgram = program

        with(program) {
            activeAttribute("aPosition")
            activeAttribute("aTextureCoordinate")
            activeUniform("uTexture")
            activeUniform("uMVPModelMatrix")
        }

        glTexture = GLTexture.generateFromBitmap(
            program.uniformHandle("uTexture"),
            0,
            loadBitmap(R.drawable.beautiful_girl1)
        )
    }

    override fun onSurfaceChanged(width: Int, height: Int) {
        GLES20.glViewport(0, 0, width, height)
        glMVPMatrix.setWorldSize(width, height)
        glTexture?.let {
            with(glMVPMatrix) {
                setModelSize(it.width, it.height)
                lookAtNormally()
                adjustToOrthogonal()
                combineMVP()
            }
        }
    }

    override fun onDrawFrame(attachment: Any?) {
        glProgram?.startDraw {
            clearColorBuffer()
            glTexture?.activeTexture()
            uniformMatrix4fv("uMVPModelMatrix", glMVPMatrix.mvpMatrix)
            vertexAttribPointerFloat("aPosition", 3, vertexVbo)
            vertexAttribPointerFloat("aTextureCoordinate", 2, textureCoordinateBuffer)
            GLES20.glDrawArrays(GLES20.GL_TRIANGLE_STRIP, 0, 4)
        }
    }

    override fun onContextDestroy() {
        Timber.d("Fixed1TextureRenderer onContextDestroy")
        glProgram?.delete()
        glProgram = null

        glTexture?.delete()
        glTexture = null
    }

}