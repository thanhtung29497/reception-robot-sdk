package com.example.smart_robot

import android.content.res.AssetManager

class TriggerWord {
    inner class Obj {
        var score = 0f
        var passLowThreshold = false
        var passHighThreshold = false
    }

    external fun initModel(assetManager: AssetManager)

    external fun bcModelDetect(buffer: FloatArray?): Obj?

    external fun convModelDetect(buffer: FloatArray?): Obj?


    companion object {
        init {
            System.loadLibrary("smartrobot")
        }
    }
}

