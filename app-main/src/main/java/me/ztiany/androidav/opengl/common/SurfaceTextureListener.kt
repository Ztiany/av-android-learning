package me.ztiany.androidav.opengl.common

import android.graphics.SurfaceTexture

interface SurfaceTextureListener {

    fun onSurfaceTextureAvailable(surfaceTexture: SurfaceTexture)

    fun onSurfaceTextureToDestroy(surfaceTexture: SurfaceTexture)

}