package com.netease.ndk_gif

import android.graphics.Bitmap

open class GifHandler(path: String) : Thread() {
    //    ndkGif  native  结构体的地址
    //
    //    存放在 java   为了方便传参
    private val gifAddr: Long
    val width: Int
        get() = getWidth(gifAddr)
    val height: Int
        get() = getHeight(gifAddr)

    init {
        //        加载  信使  GifFileType
        this.gifAddr = loadPath(path)
    }

    fun updateFrame(bitmap: Bitmap): Int {
        return updateFrame(gifAddr, bitmap)
    }

    //初始化   调用
    private external fun loadPath(path: String): Long

    external fun getWidth(ndkGif: Long): Int
    external fun getHeight(ndkGif: Long): Int
    //    隔一段事件 调用一次
    external fun updateFrame(ndkGif: Long, bitmap: Bitmap): Int

    override fun run() {

    }

    companion object {
        init {
            System.loadLibrary("native-lib")
        }
    }
}
