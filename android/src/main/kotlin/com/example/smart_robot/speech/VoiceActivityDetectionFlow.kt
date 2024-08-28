package com.example.smart_robot.speech

import android.content.Context
import android.content.res.AssetManager
import com.example.smart_robot.VAD

class VoiceActivityDetectionFlow private constructor(
    private val context: Context,
    private val assetManager: AssetManager,
) {

    private lateinit var vadModel: VAD
}