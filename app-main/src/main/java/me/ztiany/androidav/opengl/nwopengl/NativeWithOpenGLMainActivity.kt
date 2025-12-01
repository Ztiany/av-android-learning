package me.ztiany.androidav.opengl.nwopengl

import android.os.Bundle
import androidx.appcompat.app.AppCompatActivity
import me.ztiany.lib.avbase.utils.ui.IEntrance
import me.ztiany.lib.avbase.utils.ui.buildLayoutEntrance

private const val RENDER_TYPE_BACKGROUND = 1
private const val RENDER_TYPE_TRIANGLE = 2

/**
 * OpenGL ES 2.0（C++） 学习主入口。
 */
class NativeWithOpenGLMainActivity : AppCompatActivity() {

    private data class CommonItem(
        override val title: String,
        val rendererType: Int
    ) : IEntrance

    private val entrances = listOf(
        CommonItem("绘制背景", RENDER_TYPE_BACKGROUND),
        CommonItem("绘制渐变三角形", RENDER_TYPE_TRIANGLE),
    )

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(
            buildLayoutEntrance(
                this,
                entrances
            ) { _, index ->
                handleClicked(index)
            }
        )
    }

    private fun handleClicked(index: Int) {
        val item = entrances[index]
        NativeWithOpenGLCommonActivity.start(this, item.title, item.rendererType)
    }

}