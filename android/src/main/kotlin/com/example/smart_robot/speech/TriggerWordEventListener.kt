package com.example.smart_robot.speech

import com.example.smart_robot.common.EventListener

abstract class TriggerWordEventListener : EventListener<TriggerWordError>() {
    abstract fun onTriggerWordDetected()
    abstract fun onEnd()
}

enum class TriggerWordError {
    ErrorAudioRecord,
    ErrorAudioPermission,
    ErrorInitModel,
    ErrorRunModel
}