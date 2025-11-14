package me.ztiany.androidav.opengl.jwopengl.encoder

import android.view.Surface

abstract class AbstractEncoder : Encoder {

    override fun getInputSurfaceView(): Surface {
        throw UnsupportedOperationException("not implemented.")
    }

    override fun onFrame() {

    }

}