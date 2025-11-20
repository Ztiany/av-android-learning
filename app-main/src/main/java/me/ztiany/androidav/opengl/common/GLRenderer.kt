package me.ztiany.androidav.opengl.common

/** All the methods in this class will be called in a OpenGL Marked Thread. */
interface GLRenderer {

    /**
     * Called when the egl context is created.
     */
    fun onContextInitialized()

    /**
     * Called when the size of window of OpenGL is confirmed or changed.
     */
    fun onSurfaceChanged(width: Int, height: Int)

    /**
     * Called to draw the current frame.
     */
    fun onDrawFrame(attachment: Any? = null)

    /**
     * Called when the egl context is to release.
     */
    fun onContextDestroy()

}