package com.example.smart_robot.speech

import com.example.smart_robot.common.EventListener

abstract class VoiceActivityEventListener : EventListener<VADError>() {
    abstract fun onFirstVADDetected(segment: FloatArray)
    abstract fun onVADDetected(segment: FloatArray)
    abstract fun onLastVADDetected()
    abstract fun onVADEnd()
    abstract fun onVADTimeout()
}

enum class VADError {
    ErrorAudioRecord,
    ErrorAudioPermission,
    ErrorInitModel,
    ErrorRunModel
}