package com.example.smart_robot.speech

import android.annotation.SuppressLint
import android.content.Context
import android.content.res.AssetManager
import android.media.AudioFormat
import android.util.Log
import com.example.smart_robot.VAD
import com.example.smart_robot.io.audio.AudioRecordingForAIModel
import kotlin.math.abs
import kotlin.math.log10

class VoiceActivityDetectionFlow private constructor(
    private val context: Context,
    private val assetManager: AssetManager
) {

    private lateinit var vadModel: VAD
    private val listeners = mutableListOf<VoiceActivityEventListener>()
    private var noSpeechYet = true
    private var silenceTimeInFrames = 0
    private var timeoutInMilliseconds: Int? = (TIMEOUT_IN_SECONDS * 1000).toInt()

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
                noSpeechYet = true
                silenceTimeInFrames = 0
                clearBuffer()
            }

            override fun onBufferFilled(buffer: FloatArray) {
                try {
                    // run the trigger word detection model flow
                    val isSpeech = isSpeechActivity(buffer)

                    when {
                        isSpeech && noSpeechYet -> {
                            noSpeechYet = false
                            silenceTimeInFrames = 0
                            listeners.forEach { listener ->
                                listener.onFirstVADDetected(buffer)
                            }
                        }

                        isSpeech && !noSpeechYet ->
                            listeners.forEach { listener ->
                                listener.onVADDetected(buffer.toList().subList(
                                    SAMPLE_WINDOW_SIZE - SAMPLE_WINDOW_STRIDE, SAMPLE_WINDOW_SIZE
                                ).toFloatArray())
                            }

                        !isSpeech && !noSpeechYet -> {
                            noSpeechYet = true
                            silenceTimeInFrames += SAMPLE_WINDOW_SIZE
                            listeners.forEach { listener ->
                                listener.onLastVADDetected()
                            }
                        }

                        // not speech and no speech yet
                        else -> {
                            silenceTimeInFrames += SAMPLE_WINDOW_STRIDE
                        }
                    }

                    if (!isSpeech) {
                        val silenceTimeInMilliseconds = (silenceTimeInFrames.toFloat() / SAMPLE_RATE * 1000)
//                            Log.d(VoiceActivityDetectionFlow.TAG, "Silence time: %.2f".format(silenceTimeInMilliseconds))
                        if (timeoutInMilliseconds != null &&
                            silenceTimeInMilliseconds >= timeoutInMilliseconds!!) {
                            Log.d(VoiceActivityDetectionFlow.TAG, "Stop VAD due to timeout")
                            listeners.forEach { listener ->
                                listener.onVADTimeout()
                            }
                            stop()
                        }
                    }

                } catch (e: Exception) {
                    Log.e(VoiceActivityDetectionFlow.TAG, "Error running the VAD model", e)
                    listeners.forEach { listener ->
                        listener.onVADError(VADError.ErrorRunModel)
                    }
                }
            }
        }
    }

    /**
     * Initialize the voice activity detection model
     */
    fun initModel() {
        // Load the model
        vadModel = VAD().apply {
            initVADModel(assetManager)
            Log.d(TAG, "VAD model initialized")
        }
    }

    private fun isSpeechActivity(buffer: FloatArray): Boolean {
        val db = 20 * log10(buffer.maxOf { abs(it) })
        if (db < SILENCE_THRESHOLD) {
            return false
        }

        vadModel.detectVAD(buffer)?.apply {
            Log.d(TAG, "VAD Score: $score")
            return@isSpeechActivity isSpeech
        }

        return false
    }

    /**
     * Start the voice activity detection flow
     * @param listener the listener to listen to the events
     * @param timeoutInMilliseconds the maximum time of silence before the flow ends, leave it null to disable the timeout
     */
    fun startRecording(listener: VoiceActivityEventListener, timeoutInMilliseconds: Int? = null) {
        if (!audioRecorder.isRecording) {
            try {
                this.timeoutInMilliseconds = timeoutInMilliseconds
                audioRecorder.startRecording()
            } catch (e: SecurityException) {
                Log.e(TAG, "Error starting the VAD flow", e)
                listener.onVADError(VADError.ErrorAudioPermission)
            } catch (e: Exception) {
                Log.e(TAG, "Error starting the VAD flow", e)
                listener.onVADError(VADError.ErrorAudioRecord)
            }
        } else {
            Log.w(TriggerWordDetectionFlow.TAG, "VAD flow are already recording")
        }

        listeners.add(listener)
    }

    /**
     * Stop the voice activity detection flow
     */
    fun stop() {
        if (audioRecorder.isRecording) {
            audioRecorder.stopRecording()
            listeners.forEach { listener ->
                listener.onVADEnd()
            }
            listeners.clear()
        } else {
            Log.w(TriggerWordDetectionFlow.TAG, "VAD flow are not recording")
        }
    }

    companion object {
        @JvmField val TAG: String = VoiceActivityDetectionFlow::class.java.simpleName
        const val SAMPLE_RATE = 16000 // Hz
        const val SAMPLE_CHANNELS = AudioFormat.CHANNEL_IN_MONO
        const val SAMPLE_ENCODING = AudioFormat.ENCODING_PCM_FLOAT
        private const val WINDOW_SIZE = 1.0 // seconds
        const val SAMPLE_WINDOW_SIZE = (WINDOW_SIZE * SAMPLE_RATE).toInt()
        private const val WINDOW_STRIDE = 0.3 // seconds
        const val SAMPLE_WINDOW_STRIDE = (WINDOW_STRIDE * SAMPLE_RATE).toInt()
        private const val TIMEOUT_IN_SECONDS = 10.0 // seconds
        private const val SILENCE_THRESHOLD = -25.0f // dB

        @SuppressLint("StaticFieldLeak")
        private var instance : VoiceActivityDetectionFlow? = null

        @JvmStatic
        fun getInstance(context: Context, assetManager: AssetManager): VoiceActivityDetectionFlow {
            if (instance == null) {
                instance = VoiceActivityDetectionFlow(context, assetManager)
            }

            return instance!!
        }
    }
}