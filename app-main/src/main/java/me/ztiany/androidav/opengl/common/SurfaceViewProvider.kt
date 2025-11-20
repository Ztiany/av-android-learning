package me.ztiany.androidav.opengl.common

import android.view.SurfaceHolder
import android.view.SurfaceView
import android.view.View
import timber.log.Timber

class SurfaceViewProvider(private val surfaceView: SurfaceView) : SurfaceProvider {

    private lateinit var surfaceProviderCallback: SurfaceProviderCallback

    override fun start(surfaceProviderCallback: SurfaceProviderCallback) {
        if (this::surfaceProviderCallback.isInitialized) {
            throw UnsupportedOperationException("You should call this method only once.")
        }
        this.surfaceProviderCallback = surfaceProviderCallback

        surfaceView.addOnAttachStateChangeListener(object : View.OnAttachStateChangeListener {

            override fun onViewAttachedToWindow(v: View) {
                Timber.d("onViewAttachedToWindow")
            }

            override fun onViewDetachedFromWindow(v: View) {
                Timber.d("onViewDetachedFromWindow")
            }
        })

        surfaceView.holder.addCallback(object : SurfaceHolder.Callback2 {
            override fun surfaceCreated(holder: SurfaceHolder) {
                Timber.d("surfaceCreated")
                surfaceProviderCallback.onSurfaceAvailable(holder.surface)
            }

            override fun surfaceChanged(
                holder: SurfaceHolder,
                format: Int,
                width: Int,
                height: Int
            ) {
                Timber.d(
                    "surfaceChanged() called with: holder = %s, format = %d, width = %d, height = %d",
                    holder,
                    format,
                    width,
                    height
                )
                surfaceProviderCallback.onSurfaceChanged(holder.surface, width, height)
            }

            override fun surfaceDestroyed(holder: SurfaceHolder) {
                Timber.d("surfaceDestroyed")
                surfaceProviderCallback.onSurfaceDestroyed()
            }

            override fun surfaceRedrawNeeded(holder: SurfaceHolder) {
                Timber.d("surfaceRedrawNeeded")
            }
        })
    }

    override fun stop() {

    }

}