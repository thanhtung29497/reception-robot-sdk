package com.example.smart_robot

import java.io.FileOutputStream
import java.nio.ByteBuffer
import java.nio.ByteOrder

class AudioWriter {
    fun writeWavFile(filePath: String, sampleRate: Int, floatArray: FloatArray) {
        try {
            val outputStream = FileOutputStream(filePath)
            writeWavHeader(outputStream, sampleRate, floatArray.size * 2)
            writeFloatArrayToWav(outputStream, floatArray)
            outputStream.close()
        } catch (e: Exception) {
            e.printStackTrace()
        }
    }

    private fun writeWavHeader(outputStream: FileOutputStream, sampleRate: Int, bufferSize: Int) {
        val channels = 1
        val byteRate = 16 * sampleRate * channels / 8
        val totalDataLen = 36 + bufferSize
        val totalAudioLen = totalDataLen - 36

        val header = ByteBuffer.allocate(44)
        header.order(ByteOrder.LITTLE_ENDIAN)

        header.put("RIFF".toByteArray()) // RIFF header
        header.putInt(totalDataLen) // Total length of file
        header.put("WAVE".toByteArray()) // WAVE header
        header.put("fmt ".toByteArray()) // fmt header
        header.putInt(16) // Length of fmt chunk
        header.putShort(1.toShort()) // Audio format (1 for PCM)
        header.putShort(channels.toShort()) // Number of channels
        header.putInt(sampleRate) // Sample rate
        header.putInt(byteRate) // Byte rate
        header.putShort((2 * 8).toShort()) // Block align
        header.putShort(16.toShort()) // Bits per sample
        header.put("data".toByteArray()) // data header
        header.putInt(totalAudioLen) // Size of data section

        outputStream.write(header.array())
    }

    private fun writeFloatArrayToWav(outputStream: FileOutputStream, floatArray: FloatArray) {
        val shortArray = floatArrayToShortArray(floatArray)
        val byteArray = shortArrayToByteArray(shortArray)

        outputStream.write(byteArray)
    }

    private fun floatArrayToShortArray(floatArray: FloatArray): ShortArray {
        val shortArray = ShortArray(floatArray.size)
        for (i in floatArray.indices) {
            shortArray[i] = (floatArray[i] * Short.MAX_VALUE).toInt().toShort()
        }
        return shortArray
    }

    private fun shortArrayToByteArray(shortArray: ShortArray): ByteArray {
        val byteArray = ByteArray(shortArray.size * 2)
        val buffer = ByteBuffer.wrap(byteArray)
        buffer.order(ByteOrder.LITTLE_ENDIAN)
        for (value in shortArray) {
            buffer.putShort(value)
        }
        return byteArray
    }

}