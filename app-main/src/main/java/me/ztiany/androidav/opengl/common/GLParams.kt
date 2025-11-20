package me.ztiany.androidav.opengl.common

import android.os.Parcelable
import kotlinx.parcelize.Parcelize

@Parcelize
data class GLParams(
    val title: String,
    val layoutID: Int
) : Parcelable