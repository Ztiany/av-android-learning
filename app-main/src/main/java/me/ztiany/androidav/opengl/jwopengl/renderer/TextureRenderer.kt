package me.ztiany.androidav.opengl.jwopengl.renderer

import android.opengl.GLES20
import me.ztiany.androidav.R
import me.ztiany.lib.avbase.utils.loadBitmap
import me.ztiany.androidav.opengl.jwopengl.gles2.*
import me.ztiany.androidav.opengl.common.GLRenderer
import timber.log.Timber

class TextureRenderer : GLRenderer {

    private var glProgram: GLProgram? = null

    private var glTexture: GLTexture? = null

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
                "shader/vertex_base.glsl",
                "shader/fragment_texture.glsl"
            )
        } catch (e: Exception) {
            return
        }
        program.activeAttribute("aPosition")
        program.activeAttribute("aTextureCoordinate")
        program.activeUniform("uTexture")
        glProgram = program

        glTexture = GLTexture.generateFromBitmap(
            program.uniformHandle("uTexture"),
            0,
            loadBitmap(R.drawable.beautiful_girl1)
        )
    }

    override fun onSurfaceChanged(width: Int, height: Int) {
        GLES20.glViewport(0, 0, width, height)
    }

    override fun onDrawFrame(attachment: Any?) {
        glProgram?.startDraw {
            clearColorBuffer()
            glTexture?.activeTexture()
            vertexAttribPointerFloat("aPosition", 3, vertexVbo)
            vertexAttribPointerFloat("aTextureCoordinate", 2, textureCoordinateBuffer)
            GLES20.glDrawArrays(GLES20.GL_TRIANGLE_STRIP, 0, 4/*4 个点*/)
        }
    }

    override fun onContextDestroy() {
        Timber.d("TextureRenderer onContextDestroy.")
        glProgram?.delete()
        glProgram = null

        glTexture?.delete()
        glTexture = null
    }

}