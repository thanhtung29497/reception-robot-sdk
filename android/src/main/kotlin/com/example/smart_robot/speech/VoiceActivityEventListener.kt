package com.example.smart_robot.speech

abstract class VoiceActivityEventListener {
    abstract fun onFirstVADDetected(segment: FloatArray)
    abstract fun onVADDetected(segment: FloatArray)
    abstract fun onVADError(error: VADError)
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