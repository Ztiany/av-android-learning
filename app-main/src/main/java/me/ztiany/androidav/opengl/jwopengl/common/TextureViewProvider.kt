package me.ztiany.androidav.opengl.jwopengl.common

import android.graphics.SurfaceTexture
import android.view.Surface
import android.view.TextureView
import android.view.View
import timber.log.Timber

class TextureViewProvider(private val textureView: TextureView) : SurfaceProvider {

    private lateinit var surfaceProviderCallback: SurfaceProviderCallback

    private var surface: Surface? = null

    override fun start(surfaceProviderCallback: SurfaceProviderCallback) {
        if (this::surfaceProviderCallback.isInitialized) {
            throw UnsupportedOperationException("You should call this method only once.")
        }
        this.surfaceProviderCallback = surfaceProviderCallback

        textureView.addOnAttachStateChangeListener(object : View.OnAttachStateChangeListener {

            override fun onViewAttachedToWindow(v: View) {
                Timber.d("onViewAttachedToWindow")
            }

            override fun onViewDetachedFromWindow(v: View) {
                Timber.d("onViewDetachedFromWindow")
            }
        })

        textureView.surfaceTextureListener = object : TextureView.SurfaceTextureListener {

            override fun onSurfaceTextureAvailable(surfaceTexture: SurfaceTexture, width: Int, height: Int) {
                Timber.d("onSurfaceTextureAvailable $surfaceTexture")
                this@TextureViewProvider.surface = Surface(surfaceTexture)
                this@TextureViewProvider.surface?.let {
                    surfaceProviderCallback.onSurfaceAvailable(it)
                    surfaceProviderCallback.onSurfaceChanged(it, width, height)
                }
            }

            override fun onSurfaceTextureSizeChanged(surfaceTexture: SurfaceTexture, width: Int, height: Int) {
                Timber.d("onSurfaceTextureSizeChanged $surfaceTexture")
                this@TextureViewProvider.surface?.let {
                    surfaceProviderCallback.onSurfaceChanged(it, width, height)
                }
            }

            override fun onSurfaceTextureDestroyed(surfaceTexture: SurfaceTexture): Boolean {
                Timber.d("onSurfaceTextureDestroyed $surfaceTexture")
                surfaceProviderCallback.onSurfaceDestroyed()
                return true
            }

            override fun onSurfaceTextureUpdated(surfaceTexture: SurfaceTexture) {

            }

        }
    }

    override fun stop() {

    }

}