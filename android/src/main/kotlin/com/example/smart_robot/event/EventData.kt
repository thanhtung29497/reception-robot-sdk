package com.example.smart_robot.event

abstract class EventData {
    abstract fun toMap(): Map<String, Any>
}

