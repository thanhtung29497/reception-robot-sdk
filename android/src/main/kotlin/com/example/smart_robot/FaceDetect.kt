package com.example.smart_robot

import android.content.res.AssetManager
import android.graphics.Bitmap

class FaceDetect {

    companion object {
        init {
            System.loadLibrary("smartrobot")
        }
    }

    inner class Obj {
        var x = 0f
        var y = 0f
        var w = 0f
        var h = 0f
    }

    external fun initModel(assetManager: AssetManager)

    external fun detectModel(bitmap: Bitmap): Array<Obj>
}
