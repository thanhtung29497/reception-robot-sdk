package com.example.smart_robot.event

class AudioEvent (
    val type: Type,
    val data: EventData
) : EventData() {

    enum class Type {
        TRIGGER_WORD_DETECTED,
        VAD_RECORDING,
        VAD_TIMEOUT
    }

    override fun toMap(): Map<String, Any> {
        return mapOf(
            "type" to type.name,
            "data" to data.toMap()
        )
    }
}