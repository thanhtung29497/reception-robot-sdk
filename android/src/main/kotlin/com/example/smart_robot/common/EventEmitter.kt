package com.example.smart_robot.common

open class EventListener<T> {
    open fun onError(error: T) {}
}

/**
 * EventEmitter class, used to register listeners and emit events
 * @param T the type of the listener
 * @param E the type of the event error
 * @property listeners the list of listeners
 */
open class EventEmitter<T : EventListener<E>, E> {
    private val listeners = mutableListOf<T>()

    /**
     * Add a listener to the list
     * @param listener the listener to add
     */
    fun addListener(listener: T) {
        listeners.add(listener)
    }

    /**
     * Remove a listener from the list
     * @param listener the listener to remove
     */
    fun removeListener(listener: T) {
        listeners.remove(listener)
    }

    /**
     * Emit an event to all listeners
     * @param block the block to execute for each listener
     */
    fun emit(block: T.() -> Unit) {
        listeners.forEach {
            it.block()
        }
    }

    /**
     * Emit an error to all listeners
     * @param error the error to emit
     */
    fun emitError(error: E) {
        listeners.forEach {
            it.onError(error)
        }
    }

    /**
     * Clear all listeners
     */
    fun clearListeners() {
        listeners.clear()
    }
}