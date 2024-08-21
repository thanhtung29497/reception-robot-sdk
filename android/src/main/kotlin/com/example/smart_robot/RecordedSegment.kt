package com.example.smart_robot

class RecordedSegment (
    private val audioSegment: FloatArray,
    val type: Type
) {

    enum class Type {
        FIRST,
        CONTINUE,
        END
    }

    fun toMap(): Map<String, Any> {
        return mapOf(
            "audioSegment" to audioSegment,
            "type" to type.name
        )
    }
}