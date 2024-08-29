package com.example.smart_robot.speech

abstract class TriggerWordEventListener {
    abstract fun onTriggerWordDetected()
    abstract fun onError(error: TriggerWordError)
    abstract fun onEnd()
}

enum class TriggerWordError {
    ErrorAudioRecord,
    ErrorAudioPermission,
    ErrorInitModel,
    ErrorRunModel
}