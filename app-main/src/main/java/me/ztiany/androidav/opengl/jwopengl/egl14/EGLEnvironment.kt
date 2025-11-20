package me.ztiany.androidav.opengl.jwopengl.egl14

import android.os.Handler
import android.os.HandlerThread
import android.os.Message
import android.view.Surface
import me.ztiany.androidav.opengl.common.GLRenderer
import me.ztiany.androidav.opengl.common.RenderMode
import me.ztiany.androidav.opengl.common.SurfaceProvider
import me.ztiany.androidav.opengl.common.SurfaceProviderCallback
import timber.log.Timber

private const val LOG_INTERVAL = 10 * 1000L

class EGLEnvironment(
    private val surfaceProvider: SurfaceProvider,
    private val eglAttribute: EGLAttribute
) {

    companion object {
        private const val MSG_EGL_INIT = 10
        private const val MSG_EGL_SURFACE_NEW = 11
        private const val MSG_EGL_SURFACE_REFRESH = 12
        private const val MSG_EGL_SURFACE_DESTROYED = 13
        private const val MSG_EGL_RELEASE = 14

        private const val MSG_RENDERER_SURFACE_CREATED = 101
        private const val MSG_RENDERER_SURFACE_CHANGED = 102
        private const val MSG_RENDERER_DRAW = 103
    }

    private val eglThread = HandlerThread("EGLEnvironment")

    private val eglCore = EGLCore()

    private val glRenderers = mutableListOf<GLRenderer>()

    private lateinit var glRenderer: GLRenderer

    private lateinit var eglHandler: Handler

    @Volatile private var surfaceAvailable = false

    @Volatile var renderMode = RenderMode.Continuously

    private var previousDrawFrameTime = 0L

    fun bindRenderers(vararg renderers: GLRenderer) {
        if (renderers.isEmpty()) {
            throw IllegalArgumentException("at least one renderer is required.")
        }
        glRenderers.clear()
        glRenderers.addAll(renderers)
        glRenderer = renderers[0]
    }

    fun switchRenderer(glRenderer: GLRenderer) {
        val index = glRenderers.indexOf(glRenderer)
        if (index == -1) {
            throw IllegalArgumentException("the renderer has not been bound.")
        }
        switchRenderer(index)
    }

    @Suppress("MemberVisibilityCanBePrivate")
    fun switchRenderer(index: Int) {
        if (index < 0 || index >= glRenderers.size) {
            throw IllegalArgumentException("invalid renderer index.")
        }
        glRenderer = glRenderers[index]
        requestRender()
    }

    fun start() {
        if (glRenderers.isEmpty()) {
            throw IllegalStateException("no renderer has been bound.")
        }

        if (this::eglHandler.isInitialized) {
            throw IllegalStateException("renderer has already been set.")
        }

        eglThread.start()
        eglHandler = Handler(eglThread.looper, ::handleMessage)
        eglHandler.sendEmptyMessage(MSG_EGL_INIT)

        surfaceProvider.start(surfaceProviderCallback)
    }

    private val surfaceProviderCallback = object : SurfaceProviderCallback {
        override fun onSurfaceAvailable(surface: Surface) {
            Timber.d("onSurfaceAvailable() called with: surface = $surface")
            surfaceAvailable = true
            eglHandler.sendMessage(Message.obtain().apply {
                what = MSG_EGL_SURFACE_NEW
                obj = surface
            })
        }

        override fun onSurfaceChanged(surface: Surface, width: Int, height: Int) {
            Timber.d("onSurfaceChanged() called with: surface = $surface, width = $width, height = $height")
            eglHandler.sendMessage(Message.obtain().apply {
                what = MSG_EGL_SURFACE_REFRESH
                arg1 = width
                arg2 = height
            })
        }

        override fun onSurfaceDestroyed() {
            Timber.d("onSurfaceDestroyed() called")
            surfaceAvailable = false
            eglHandler.removeCallbacksAndMessages(null)
            eglHandler.sendEmptyMessage(MSG_EGL_SURFACE_DESTROYED)
        }
    }

    private fun handleMessage(message: Message): Boolean {
        if (message.what >= MSG_RENDERER_SURFACE_CREATED) {
            handleRendererMessage(message)
        } else {
            handleEGLMessage(message)
        }
        return true
    }

    private fun handleEGLMessage(message: Message) {
        when (message.what) {
            MSG_EGL_INIT -> {
                Timber.d("handleEGLMessage MSG_EGL_INIT")
                eglCore.makeEglContext(eglAttribute.sharedContext)
            }

            MSG_EGL_SURFACE_NEW -> {
                Timber.d("handleEGLMessage MSG_EGL_NEW_SURFACE")
                eglCore.makeEglWindowSurface(message.obj as Surface)
                eglCore.makeCurrent()
                eglHandler.sendEmptyMessage(MSG_RENDERER_SURFACE_CREATED)
            }

            MSG_EGL_SURFACE_REFRESH -> {
                Timber.d("handleEGLMessage MSG_EGL_SURFACE_REFRESH")
                //egl doesn't need to do anything.
                //...
                //notify renderer the surface size.
                eglHandler.sendMessage(Message.obtain().apply {
                    what = MSG_RENDERER_SURFACE_CHANGED
                    arg1 = message.arg1
                    arg2 = message.arg2
                })

                // start drawing if needed.
                checkIfDrawContinuously(true)
            }

            MSG_EGL_SURFACE_DESTROYED -> {
                Timber.d("handleEGLMessage MSG_EGL_SURFACE_DESTROYED")
                glRenderers.forEach { it.onContextDestroy() }
                eglCore.destroySurface()
            }

            MSG_EGL_RELEASE -> {
                Timber.d("handleEGLMessage MSG_EGL_SURFACE_RELEASE")
                eglCore.release()
            }
        }
    }

    private fun handleRendererMessage(message: Message) {
        when (message.what) {
            MSG_RENDERER_SURFACE_CREATED -> {
                Timber.d("handleEGLMessage MSG_RENDERER_SURFACE_CREATED. eglActive=${eglCore.isActive()}, surfaceAvailable=$surfaceAvailable")
                if (eglCore.isActive() && surfaceAvailable) {
                    glRenderers.forEach { it.onContextInitialized() }
                }
            }

            MSG_RENDERER_SURFACE_CHANGED -> {
                Timber.d("handleEGLMessage MSG_RENDERER_SURFACE_CHANGED. eglActive=${eglCore.isActive()}, surfaceAvailable=$surfaceAvailable")
                if (eglCore.isActive() && surfaceAvailable) {
                    glRenderers.forEach { it.onSurfaceChanged(message.arg1, message.arg2) }
                }
            }

            MSG_RENDERER_DRAW -> {
                if (System.currentTimeMillis() - previousDrawFrameTime >= LOG_INTERVAL) {
                    Timber.d("handleEGLMessage MSG_RENDERER_DRAW. eglActive=${eglCore.isActive()}, surfaceAvailable=$surfaceAvailable")
                    previousDrawFrameTime = System.currentTimeMillis()
                }

                if (eglCore.isActive() && surfaceAvailable) {
                    // Here, only the current glRenderer is asked to draw.
                    glRenderer.onDrawFrame(message.obj)
                    eglCore.swapBuffers()
                }
                checkIfDrawContinuously(false)
            }
        }
    }

    private fun checkIfDrawContinuously(first: Boolean) {
        if (renderMode !== RenderMode.Continuously) {
            return
        }
        if (first) {
            eglHandler.sendEmptyMessage(MSG_RENDERER_DRAW)
        } else {
            /* TODO: optimize the delay time. */
            eglHandler.sendEmptyMessageDelayed(MSG_RENDERER_DRAW, 16)
        }
    }

    fun requestRender(attach: Any? = null) {
        if (surfaceAvailable && eglCore.isActive()) {
            eglHandler.sendMessage(Message.obtain().apply {
                what = MSG_RENDERER_DRAW
                obj = attach
            })
        }
    }

    fun setPresentationTime(nanoseconds: Long) {
        eglCore.setPresentationTime(nanoseconds)
    }

    fun release() {
        surfaceProvider.stop()
        eglHandler.removeCallbacksAndMessages(null)
        //A TextureView's onDestroy is called after an Activity's onDestroy.
        if (surfaceAvailable) {
            eglHandler.sendEmptyMessage(MSG_EGL_SURFACE_DESTROYED)
        }
        eglHandler.sendEmptyMessage(MSG_EGL_RELEASE)
        eglThread.quitSafely()
    }

}