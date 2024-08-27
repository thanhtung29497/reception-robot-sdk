package com.example.smart_robot.speech

import android.Manifest
import android.annotation.SuppressLint
import android.content.Context
import android.content.pm.PackageManager
import android.content.res.AssetManager
import android.media.AudioFormat
import android.media.AudioRecord
import android.media.MediaRecorder
import android.media.AudioManager
import android.util.Log
import androidx.core.app.ActivityCompat
import com.example.smart_robot.TriggerWord
import kotlinx.coroutines.DelicateCoroutinesApi
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.GlobalScope
import kotlinx.coroutines.launch
import kotlinx.coroutines.withContext
import java.util.concurrent.atomic.AtomicBoolean
import kotlin.math.abs
import kotlin.math.log10

class TriggerWordDetectionFlow private constructor(
    private val context: Context,
    private val assetManager: AssetManager,
    private val bcThreshold: Float = 0.8f,
    private val convThreshold: Float = 0.6f
) {

    private lateinit var triggerWord: TriggerWord

    private val listeners = mutableListOf<TriggerWordEventListener>()

    private val bufferSize: Int by lazy {
        AudioRecord.getMinBufferSize(
            SAMPLE_RATE,
            SAMPLE_CHANNELS,
            SAMPLE_ENCODING
        )
    }

    private var audioRecord: AudioRecord? = null
    private var isRecording = AtomicBoolean(false)


    private suspend fun recordingCoroutine() = withContext(Dispatchers.Default) {
        audioRecord?.apply {
            val buffer = FloatArray(SAMPLE_DURATION * SAMPLE_RATE)
            while (isRecording.get()) {
                read(buffer, 0, buffer.size, AudioRecord.READ_BLOCKING)
                isTriggerWord(buffer)?.let {
                    Log.d(TAG, "Trigger word detected")
                    listeners.forEach { listener ->
                        listener.onTriggerWordDetected()
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
        triggerWord = TriggerWord()
        triggerWord.initModel(assetManager)
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
        val db = 20 * log10(buffer.maxOf { abs(it) })
        if (abs(db) <= 0) {
            return null
        }

        detectBCModel(buffer)?.also {bcResult ->
            Log.d(TAG, "BC score: ${bcResult.score}")
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

    /**
     * Start listening for the trigger word
     * If the flow is already recording, the listener will be added to the list of listeners
     * @param listener the listener to be notified when the trigger word is detected
     */

    fun startListening(listener: TriggerWordEventListener) {
        if (isRecording.compareAndSet(false, true)) {
            startRecording()
        } else {
            Log.w(TAG, "Trigger word flow are already recording")
        }

        listeners.add(listener)
    }

    fun stop() {
        if (isRecording.compareAndSet(true, false)) {
            stopRecording()
        } else {
            Log.w(TAG, "Trigger word flow are not recording")
        }
    }

    @OptIn(DelicateCoroutinesApi::class)
    private fun startRecording() {
        checkPermission() ?: run {
            listeners.forEach { listener ->
                listener.onError(TriggerWordError.ErrorAudioPermission)
            }

            return@startRecording
        }

        try {
            val audioManager: AudioManager = context.getSystemService(Context.AUDIO_SERVICE) as AudioManager
            Log.d(TAG, audioManager.getDevices(AudioManager.GET_DEVICES_INPUTS).toString())

            audioRecord = AudioRecord(
                MediaRecorder.AudioSource.MIC,
                SAMPLE_RATE,
                SAMPLE_CHANNELS,
                SAMPLE_ENCODING,
                bufferSize
            ).apply {
                GlobalScope.launch(Dispatchers.Default) {
                    startRecording()
                    recordingCoroutine()
                }
            }
        } catch (e: Exception) {
            Log.e(TAG, "Failed to start recording", e)
            isRecording.set(false)
            listeners.forEach { listener ->
                listener.onError(TriggerWordError.ErrorAudioRecord)
            }
        }
    }

    private fun stopRecording() {
        audioRecord?.stop()
        audioRecord?.release()
        audioRecord = null
        isRecording.set(false)
        listeners.forEach { listener ->
            listener.onEnd()
        }

        listeners.clear()
    }

    companion object {
        @JvmField val TAG: String = TriggerWordDetectionFlow::class.java.simpleName
        const val SAMPLE_RATE = 8000
        const val SAMPLE_DURATION = 1
        const val SAMPLE_SIZE = 2
        const val SAMPLE_CHANNELS = AudioFormat.CHANNEL_IN_MONO
        const val SAMPLE_ENCODING = AudioFormat.ENCODING_PCM_FLOAT

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