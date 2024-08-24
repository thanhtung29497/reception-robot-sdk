package com.example.smart_robot.event

class AudioEvent (
    val type: Type,
    val data: EventData? = null
) : EventData() {

    enum class Type {
        TRIGGER_WORD_DETECTED,
        VAD_RECORDING,
        VAD_TIMEOUT
    }

    override fun toMap(): Map<String, Any?> {
        return mapOf(
            "type" to type.name,
            "data" to data?.toMap()
        )
    }

    companion object {
        fun triggerWordDetected() = AudioEvent(Type.TRIGGER_WORD_DETECTED).toMap()
        fun vadRecording(data: RecordedSegment) = AudioEvent(Type.VAD_RECORDING, data).toMap()
        fun vadTimeout() = AudioEvent(Type.VAD_TIMEOUT).toMap()
    }
}