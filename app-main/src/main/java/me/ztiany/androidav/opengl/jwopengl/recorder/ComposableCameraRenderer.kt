package me.ztiany.androidav.opengl.jwopengl.recorder

import android.content.Context
import android.graphics.SurfaceTexture
import android.opengl.EGL14
import android.opengl.EGLContext
import android.opengl.GLES11Ext
import androidx.core.content.ContextCompat
import me.ztiany.androidav.opengl.common.EGLBridge
import me.ztiany.androidav.opengl.common.GLRenderer
import me.ztiany.androidav.opengl.common.SurfaceTextureListener
import me.ztiany.androidav.opengl.jwopengl.filter.GLFilter
import me.ztiany.androidav.opengl.jwopengl.filter.NoneEffectFBOFilter
import me.ztiany.androidav.opengl.jwopengl.filter.ScreenFilter
import me.ztiany.androidav.opengl.jwopengl.gles2.GLTexture
import me.ztiany.androidav.opengl.jwopengl.gles2.TextureAttribute
import me.ztiany.androidav.opengl.jwopengl.gles2.delete
import timber.log.Timber
import java.util.concurrent.CopyOnWriteArrayList

/** 录像特效 + 展示 */
class ComposableCameraRenderer(
    private val context: Context,
    private val eglBridge: EGLBridge
) : GLRenderer {

    /** 承载视频的纹理 */
    private var surfaceTexture: SurfaceTexture? = null

    private var glTexture: GLTexture? = null

    private val foundationFBOFilter = NoneEffectFBOFilter()

    private val foundationScreenFilter = ScreenFilter()

    private val effectFilters = CopyOnWriteArrayList<GLFilter>()

    @Volatile private var recorder: Recorder? = null

    @Volatile private var textureSizeReceived = false

    private lateinit var eglContext: EGLContext

    private var attribute: TextureAttribute? = null

    private var currentWorldWidth = 0

    private var currentWorldHeight = 0

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

    fun startRecording(recorder: Recorder) {
        this.recorder?.onStop()
        this.recorder = recorder
        this.recorder?.onStart(eglContext)
    }

    fun stopRecording() {
        val stoppingRecorder = this.recorder
        this.recorder = null
        stoppingRecorder?.onStop()
    }

    override fun onContextInitialized() {
        Timber.d("onSurfaceCreated() called")

        eglContext = EGL14.eglGetCurrentContext()

        val texture = GLTexture.generate(
            GLTexture.NONE,
            0,
            GLES11Ext.GL_TEXTURE_EXTERNAL_OES
        )
        glTexture = texture

        surfaceTexture = SurfaceTexture(texture.id)
        ContextCompat.getMainExecutor(context).execute { dispatchSurfaceTexture() }
        surfaceTexture?.setOnFrameAvailableListener { eglBridge.requestRender() }

        foundationFBOFilter.initProgram()
        foundationScreenFilter.initProgram()
    }

    override fun onSurfaceChanged(width: Int, height: Int) {
        Timber.d("onSurfaceChanged() called with: width = $width, height = $height")
        currentWorldWidth = width
        currentWorldHeight = height

        foundationFBOFilter.setWorldSize(width, height)
        foundationScreenFilter.setWorldSize(width, height)

        effectFilters.forEach { it.setWorldSize(width, height) }
    }

    override fun onDrawFrame(attachment: Any?) {
        if (!textureSizeReceived) {
            return
        }
        val lSurfaceTexture = surfaceTexture ?: return
        val texture = glTexture ?: return

        surfaceTexture?.updateTexImage()

        // draw raw video on fbo
        var glTexture = foundationFBOFilter.onDrawFrame(texture)

        // do effect on fbo
        effectFilters.forEach {
            glTexture = it.onDrawFrame(glTexture)
        }

        // draw fbo on screen.
        glTexture = foundationScreenFilter.onDrawFrame(glTexture)

        // send effect to recorder if need.
        recorder?.run {
            onFrame(TextureWithTime(glTexture, lSurfaceTexture.timestamp))
        }
    }

    override fun onContextDestroy() {
        Timber.d("onSurfaceDestroyed() called")

        foundationFBOFilter.release()
        foundationScreenFilter.release()
        for (filter in effectFilters) {
            filter.release()
        }

        surfaceTexture?.setOnFrameAvailableListener(null)
        surfaceTexture?.let { surfaceTextureListener?.onSurfaceTextureToDestroy(it) }
        surfaceTexture?.release()
        surfaceTexture = null

        glTexture?.delete()
        glTexture = null
    }


    fun setVideoAttribute(attribute: TextureAttribute) {
        Timber.d("setVideoAttribute() called with: attribute = $attribute")
        this.attribute = attribute
        textureSizeReceived = true

        foundationFBOFilter.setTextureAttribute(attribute)
        foundationScreenFilter.setTextureAttribute(attribute)

        effectFilters.forEach {
            it.setTextureAttribute(attribute)
        }
    }

    fun addEffect(glFilter: GLFilter) {
        attribute?.let {
            glFilter.setTextureAttribute(it)
        }
        effectFilters.add(glFilter)
    }

    fun removeEffect(glFilter: GLFilter) {
        effectFilters.remove(glFilter)
    }

    fun removeAllEffect() {
        effectFilters.clear()
    }

}

class TextureWithTime(
    /** 存储了当前帧的纹理 */
    val glTexture: GLTexture,
    /** 单位：nanoseconds */
    val timestamp: Long
)
