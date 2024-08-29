package com.example.smart_robot.speech

import android.annotation.SuppressLint
import android.content.Context
import android.content.res.AssetManager
import android.media.AudioFormat
import android.util.Log
import com.example.smart_robot.TriggerWord
import com.example.smart_robot.io.audio.AudioRecordingForAIModel
import kotlin.math.abs
import kotlin.math.log10

class TriggerWordDetectionFlow private constructor(
    private val context: Context,
    private val assetManager: AssetManager,
    private val bcThreshold: Float = 0.6f,
    private val convThreshold: Float = 0.6f
) {

    private lateinit var triggerWord: TriggerWord

    private val listeners = mutableListOf<TriggerWordEventListener>()

    private val audioRecorder: AudioRecordingForAIModel by lazy {
        object : AudioRecordingForAIModel(
            context,
            SAMPLE_RATE,
            SAMPLE_CHANNELS,
            SAMPLE_ENCODING,
            SAMPLE_WINDOW_SIZE,
            SAMPLE_WINDOW_STRIDE
        ) {
            override fun onBeforeRecording() {
                clearBuffer()
            }

            override fun onBufferFilled(buffer: FloatArray) {
                try {
                    // run the trigger word detection model flow
                    isTriggerWord(buffer)?.let {
                        // Clear the buffer to avoid multiple trigger word detection
                        clearBuffer()
                        Log.d(TAG, "Trigger word detected")
                        listeners.forEach { listener ->
                            listener.onTriggerWordDetected()
                        }
                    }
                } catch (e: Exception) {
                    listeners.forEach { listener ->
                        listener.onError(TriggerWordError.ErrorRunModel)
                    }
                }
            }
        }
    }



    /**
     * Initialize the trigger word detection model
     */
    fun initModel() {
        // Load the model
        try {
            triggerWord = TriggerWord().apply {
                initModel(assetManager)
            }
        } catch (e: Exception) {
            listeners.forEach { listener ->
                listener.onError(TriggerWordError.ErrorInitModel)
            }
        }
    }

    /**
     * Detect the trigger word using the BC model
     */
    private fun detectBCModel(buffer: FloatArray?): TriggerWord.Obj? {
        return triggerWord.bcModelDetect(buffer)
    }

    /**
     * Detect the trigger word using the Conv model
     */
    private fun detectConvModel(buffer: FloatArray?): TriggerWord.Obj? {
        return triggerWord.convModelDetect(buffer)
    }

    /**
     * run the trigger word detection model flow
     */
    private fun isTriggerWord(buffer: FloatArray): TriggerWord.Obj? {
        // Calculate the dB of the buffer, if the dB is less than the silence threshold, not process the buffer
        val db = 20 * log10(buffer.maxOf { abs(it) })
        if (db < SILENCE_THRESHOLD) {
            return null
        }

        detectBCModel(buffer)?.also {bcResult ->
            Log.d(TAG, "BC score: ${bcResult.score}")
//            if (bcResult.score > 0.2) {
//                val audioWriter = AudioWriter()
//                val filePath = context.filesDir.absolutePath + "/" + System.currentTimeMillis() + "_" + bcResult.score + ".wav"
//                Log.d(TAG, "Writing audio to $filePath")
//                audioWriter.writeWavFile(filePath, SAMPLE_RATE, buffer)
//            }
            if (bcResult.score > bcThreshold) {
                detectConvModel(buffer)?.also {convResult ->
                    Log.d(TAG, "Conv score: ${convResult.score}")
                    if (convResult.score > convThreshold) {
                        return convResult
                    }
                }
            }
        }

        return null
    }


    /**
     * Start listening for the trigger word
     * If the flow is already recording, the listener will be added to the list of listeners
     * @param listener the listener to be notified when the trigger word is detected
     */

    fun startListening(listener: TriggerWordEventListener) {
        if (!audioRecorder.isRecording) {
            try {
                audioRecorder.startRecording()
            } catch (e: SecurityException) {
                listener.onError(TriggerWordError.ErrorAudioPermission)
            } catch (e: Exception) {
                listener.onError(TriggerWordError.ErrorAudioRecord)
            }
        } else {
            Log.w(TAG, "Trigger word flow are already recording")
        }

        listeners.add(listener)
    }

    fun stop() {
        if (audioRecorder.isRecording) {
            audioRecorder.stopRecording()
            listeners.forEach { listener ->
                listener.onEnd()
            }

            listeners.clear()
        } else {
            Log.w(TAG, "Trigger word flow are not recording")
        }
    }

    companion object {
        @JvmField val TAG: String = TriggerWordDetectionFlow::class.java.simpleName
        const val SAMPLE_RATE = 8000 // Hz
        private const val WINDOW_SIZE = 1.0 // seconds
        const val SAMPLE_WINDOW_SIZE = (WINDOW_SIZE * SAMPLE_RATE).toInt()
        private const val WINDOW_STRIDE = 0.3 // seconds
        const val SAMPLE_WINDOW_STRIDE = (WINDOW_STRIDE * SAMPLE_RATE).toInt()
        const val SAMPLE_CHANNELS = AudioFormat.CHANNEL_IN_MONO
        const val SAMPLE_ENCODING = AudioFormat.ENCODING_PCM_FLOAT
        const val SILENCE_THRESHOLD = -25.0 // dB

        @SuppressLint("StaticFieldLeak")
        private var instance : TriggerWordDetectionFlow? = null

        @JvmStatic
        fun getInstance(context: Context, assetManager: AssetManager): TriggerWordDetectionFlow {
            if (instance == null) {
                instance = TriggerWordDetectionFlow(context, assetManager)
            }

            return instance!!
        }
    }


}