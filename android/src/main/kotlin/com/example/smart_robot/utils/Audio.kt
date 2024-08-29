package com.example.smart_robot.utils

import java.nio.ByteBuffer
import java.nio.ByteOrder

class AudioUtils {
    companion object {
        /**
         * Convert PCM 16-bit to PCM float
         * @param pcm PCM 16-bit
         * @return PCM float
         */
        fun pcm16toFloat(pcm: ByteArray): FloatArray {
            val floaters = FloatArray(pcm.size)
            for (i in pcm.indices) {
                floaters[i] = pcm[i] / 32768.0f
            }
            return floaters
        }

        /**
         * Convert PCM float to PCM 16-bit
         * @param pcm PCM float
         * @return PCM 16-bit
         */
        fun pcmFloatTo16(pcm: FloatArray): ShortArray {
            val shorts = ShortArray(pcm.size)
            for (i in pcm.indices) {
                shorts[i] = (pcm[i] * 32768).toInt().toShort()
            }
            return shorts
        }

        /**
         * Convert short array to byte array
         * @param shortArray Short array
         * @return Byte array
         */
        fun shortArrayToByteArray(shortArray: ShortArray): ByteArray {
            val byteArray = ByteArray(shortArray.size * 2)
            val buffer = ByteBuffer.wrap(byteArray)
            buffer.order(ByteOrder.LITTLE_ENDIAN)
            for (value in shortArray) {
                buffer.putShort(value)
            }
            return byteArray
        }
    }
}