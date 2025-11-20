package me.ztiany.androidav.opengl.jwopengl.encoder

import android.view.Surface

interface Encoder {

    val mode: EncoderMode

    fun init(width: Int, height: Int)

    fun start()

    fun stop()

    // for Hard Encoder
    fun getInputSurfaceView(): Surface

    // for Soft Encoder
    fun onFrame()

}


enum class EncoderMode {
    Hard, Soft
}