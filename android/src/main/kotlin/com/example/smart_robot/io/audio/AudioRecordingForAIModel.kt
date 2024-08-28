package com.example.smart_robot.io.audio

import android.Manifest
import android.content.Context
import android.content.pm.PackageManager
import android.media.AudioFormat
import android.media.AudioRecord
import android.media.MediaRecorder
import android.util.Log
import androidx.core.app.ActivityCompat
import kotlinx.coroutines.DelicateCoroutinesApi
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.GlobalScope
import kotlinx.coroutines.launch
import kotlinx.coroutines.withContext

class AudioRecordingForAIModel (
    private val context: Context,
    private val sampleRate: Int = 44100,
    private val channelConfig: Int = AudioFormat.CHANNEL_IN_MONO,
    private val audioFormat: Int = AudioFormat.ENCODING_PCM_FLOAT,
    private val sampleWindowSize: Int,
    private val sampleWindowStride: Int,
    private val process: (FloatArray) -> Unit
) {

    private val bufferSize: Int by lazy {
        AudioRecord.getMinBufferSize(
            sampleRate,
            channelConfig,
            audioFormat
        )
    }

    private var audioRecord: AudioRecord? = null
    var isRecording = false
        private set
    private var audioBuffer: ArrayList<Float> = arrayListOf()

    /**
     * Check if the app has the required permission
     * @return true if the app has the required permission, null otherwise
     */
    private fun checkPermission(): Boolean? {
        if (ActivityCompat.checkSelfPermission(
                context,
                Manifest.permission.RECORD_AUDIO
            ) != PackageManager.PERMISSION_GRANTED
        ) {
            Log.w(TAG, "Permission not granted")
            return null
        }

        return true
    }

    @OptIn(DelicateCoroutinesApi::class)
    fun startRecording() {
        checkPermission() ?: run {
            Log.w(TAG, "Permission not granted")
            throw SecurityException("Permission not granted")
        }

        try {
            audioRecord = AudioRecord(
                MediaRecorder.AudioSource.MIC,
                sampleRate,
                channelConfig,
                audioFormat,
                bufferSize
            ).apply {
                GlobalScope.launch(Dispatchers.Default) {
                    isRecording = true
                    startRecording()
                    recordingCoroutine()
                }
            }
        } catch (e: Exception) {
            Log.e(TAG, "Error initializing AudioRecord", e)
            throw e
        }
    }

    private suspend fun recordingCoroutine() = withContext(Dispatchers.Default) {
        audioRecord?.apply {
            val tempBuffer = FloatArray(sampleWindowStride)

            while (isRecording) {
                val byteRead = read(tempBuffer, 0, tempBuffer.size, AudioRecord.READ_BLOCKING)
                if (byteRead > 0) {
                    audioBuffer.addAll(tempBuffer.toList())

                    if (audioBuffer.size >= sampleWindowSize) {
                        val audioWindow = audioBuffer.subList(0, sampleWindowSize).toFloatArray()
                        audioBuffer = ArrayList(audioBuffer.subList(sampleWindowStride, audioBuffer.size))

                        process(audioWindow)
                    }
                }
            }
        }
    }

    fun clearBuffer() {
        audioBuffer.clear()
    }

    fun stopRecording() {
        audioRecord?.stop()
        audioRecord?.release()
        audioRecord = null
        isRecording = false
    }

    companion object {
        @JvmField val TAG: String = AudioRecordingForAIModel::class.java.simpleName
    }



}