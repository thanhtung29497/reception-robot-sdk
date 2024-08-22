package com.example.smart_robot.event

class RecordedSegment (
    private val audioSegment: ByteArray,
    val type: Type
) : EventData() {

    enum class Type(val value: Int) {
        FIRST(0),
        CONTINUE(1),
        LAST(2)
    }

    override fun toMap(): Map<String, Any> {
        return mapOf(
            "audioSegment" to audioSegment,
            "type" to type.value
        )
    }
}