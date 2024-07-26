package com.example.smart_robot
import android.content.res.AssetManager


class VAD {
    inner class Obj {
        var score = 0f
        var isSpeech = false
    }

    external fun initVADModel(assetMgr: AssetManager?)
    external fun detectVAD(buffer: FloatArray?): Obj?

    companion object {
        init {
            System.loadLibrary("smartrobot")
        }
    }
}

