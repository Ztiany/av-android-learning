package me.ztiany.androidav.opengl.jwopengl.renderer

import android.opengl.GLES20
import me.ztiany.androidav.R
import me.ztiany.lib.avbase.utils.loadBitmap
import me.ztiany.androidav.opengl.jwopengl.gles2.*
import me.ztiany.androidav.opengl.common.GLRenderer
import timber.log.Timber

class Fixed2TextureRenderer : GLRenderer {

    private var glProgram: GLProgram? = null

    private var glTexture: GLTexture? = null

    private val mvpMatrix by lazy { GLMVPMatrix() }

    /** 矩形的坐标 */
    private val vertexVbo = generateVBOBuffer(newVertexCoordinateFull3().map {
        it * 0.5F
    }.toFloatArray())

    /** 纹理坐标 */
    private val textureCoordinateBuffer = generateVBOBuffer(newTextureCoordinateAndroid())

    override fun onContextInitialized() {
        val program = try {
            GLProgram.fromAssets(
                "shader/vertex_mvp_separated.glsl",
                "shader/fragment_texture.glsl"
            )
        } catch (e: Exception) {
            return
        }
        glProgram = program

        with(program) {
            activeAttribute("aPosition")
            activeAttribute("aTextureCoordinate")
            activeUniform("uTexture")
            activeUniform("uModelMatrix")
            activeUniform("uViewMatrix")
            activeUniform("uProjectionMatrix")
        }

        glTexture = GLTexture.generateFromBitmap(
            program.uniformHandle("uTexture"),
            0,
            loadBitmap(R.drawable.knight)
        )
    }

    override fun onSurfaceChanged(width: Int, height: Int) {
        GLES20.glViewport(0, 0, width, height)
        mvpMatrix.setWorldSize(width, height)

        glTexture?.let {
            with(mvpMatrix) {
                setModelSize(it.width, it.height)
                lookAtNormally()
                adjustToOrthogonal()
            }
        }
    }

    override fun onDrawFrame(attachment: Any?) {
        glProgram?.startDraw {
            clearColorBuffer()
            glTexture?.activeTexture()
            uniformMatrix4fv("uModelMatrix", mvpMatrix.modelMatrix)
            uniformMatrix4fv("uViewMatrix", mvpMatrix.viewMatrix)
            uniformMatrix4fv("uProjectionMatrix", mvpMatrix.projectionMatrix)
            vertexAttribPointerFloat("aPosition", 3, vertexVbo)
            vertexAttribPointerFloat("aTextureCoordinate", 2, textureCoordinateBuffer)
            GLES20.glDrawArrays(GLES20.GL_TRIANGLE_STRIP, 0, 4)
        }
    }

    override fun onContextDestroy() {
        Timber.d("Fixed2TextureRenderer onContextDestroy")

        glProgram?.delete()
        glProgram = null

        glTexture?.delete()
        glTexture = null
    }

}