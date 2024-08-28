package com.example.smart_robot.speech

abstract class VoiceActivityEventListener {
    abstract fun onVADDetected()
    abstract fun onVADStarted()
    abstract fun onError(error: VADError)
    abstract fun onVADEnd()
    abstract fun onVADTimeout()
}

enum class VADError {
    ErrorAudioRecord,
    ErrorAudioPermission,
    ErrorInitModel,
    ErrorDetectModel
}