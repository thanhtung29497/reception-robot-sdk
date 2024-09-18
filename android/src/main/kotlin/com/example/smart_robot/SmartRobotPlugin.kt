package com.example.smart_robot

import android.content.Context
import android.content.res.AssetManager
import android.graphics.Bitmap
import android.graphics.BitmapFactory
import android.media.AudioRecord
import android.media.AudioTrack
import android.net.Uri
import android.util.Log
import com.example.smart_robot.event.AudioEvent
import com.example.smart_robot.event.RecordedSegment
import com.example.smart_robot.speech.TriggerWordDetectionFlow
import com.example.smart_robot.speech.TriggerWordError
import com.example.smart_robot.speech.TriggerWordEventListener
import com.example.smart_robot.speech.VADError
import com.example.smart_robot.speech.VoiceActivityDetectionFlow
import com.example.smart_robot.speech.VoiceActivityEventListener
import com.example.smart_robot.utils.AudioUtils
import io.flutter.embedding.engine.plugins.FlutterPlugin
import io.flutter.plugin.common.EventChannel
import io.flutter.plugin.common.MethodCall
import io.flutter.plugin.common.MethodChannel
import io.flutter.plugin.common.MethodChannel.MethodCallHandler
import io.flutter.plugin.common.MethodChannel.Result
import io.flutter.plugin.common.PluginRegistry
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.GlobalScope
import kotlinx.coroutines.launch
import org.json.JSONStringer
import java.io.File


/** SmartRobotPlugin */
class SmartRobotPlugin : FlutterPlugin, MethodCallHandler, EventChannel.StreamHandler {
    /// The MethodChannel that will the communication between Flutter and native Android
    ///
    /// This local reference serves to register the plugin with the Flutter Engine and unregister it
    /// when the Flutter Engine is detached from the Activity
    private lateinit var channel: MethodChannel

    private lateinit var eventChannel: EventChannel

    private var eventSink: EventChannel.EventSink? = null

    private lateinit var faceDetect: FaceDetect

    private lateinit var triggerWord: TriggerWord

    private lateinit var triggerWordDetectionFlow: TriggerWordDetectionFlow

    private lateinit var voiceActivityDetectionFlow: VoiceActivityDetectionFlow

    private lateinit var vad: VAD

    private lateinit var context: Context

    private var triggerWordAudioRecord: AudioRecord? = null

    private var vadAudioRecord: AudioRecord? = null

    private var audioTrack : AudioTrack? = null

    private var isSpeaking = false


    override fun onAttachedToEngine(flutterPluginBinding: FlutterPlugin.FlutterPluginBinding) {
        channel = MethodChannel(flutterPluginBinding.binaryMessenger, "smart_robot")
        channel.setMethodCallHandler(this)
        eventChannel = EventChannel(flutterPluginBinding.binaryMessenger, "smart_robot_event")
        eventChannel.setStreamHandler(this)

        faceDetect = FaceDetect()

        context = flutterPluginBinding.applicationContext
        assetManager = flutterPluginBinding.applicationContext.assets

        triggerWordDetectionFlow = TriggerWordDetectionFlow.getInstance(context, assetManager)
        voiceActivityDetectionFlow = VoiceActivityDetectionFlow.getInstance(context, assetManager)
        handleSpeechEvent()
    }

    companion object {
        @JvmField val TAG: String = SmartRobotPlugin::class.java.simpleName

        private lateinit var assetManager: AssetManager
        private const val VAD_TIMEOUT_IN_MILLISECONDS = 30000

        @JvmStatic
        fun registerWith(registrar: PluginRegistry.Registrar) {
            val channel = MethodChannel(registrar.messenger(), "smart_robot")
            channel.setMethodCallHandler(SmartRobotPlugin())
            assetManager = registrar.context().assets
        }

        init {
            System.loadLibrary("smartrobot")
        }
    }

    private fun handleSpeechEvent() {
        triggerWordDetectionFlow.addListener(object: TriggerWordEventListener() {
            override fun onTriggerWordDetected() {
                GlobalScope.launch(Dispatchers.Main) {
                    eventSink?.success(AudioEvent.triggerWordDetected())
                }
            }

            override fun onError(error: TriggerWordError) {
                Log.w(TAG, "Trigger word error: $error")
            }

            override fun onEnd() {
                Log.d(TAG, "Trigger word flow ended")
            }
        })

        voiceActivityDetectionFlow.addListener(object: VoiceActivityEventListener() {
            override fun onFirstVADDetected(segment: FloatArray) {
                // Sending event to flutter are marked with @UiThread so it must be called from the main thread
                GlobalScope.launch(Dispatchers.Main) {
                    eventSink?.success(
                        AudioEvent.vadRecording(
                            RecordedSegment(
                                AudioUtils.shortArrayToByteArray(AudioUtils.pcmFloatTo16(segment)),
                                RecordedSegment.Type.FIRST
                            )
                        )
                    )
                }
            }

            override fun onVADDetected(segment: FloatArray) {
                // Sending event to flutter are marked with @UiThread so it must be called from the main thread
                GlobalScope.launch(Dispatchers.Main) {
                    eventSink?.success(AudioEvent.vadRecording(
                        RecordedSegment(
                            AudioUtils.shortArrayToByteArray(AudioUtils.pcmFloatTo16(segment)),
                            RecordedSegment.Type.CONTINUE
                        )
                    ))
                }
            }

            override fun onError(error: VADError) {
                Log.w(TAG, "VAD Error: $error")
            }

            override fun onLastVADDetected() {
                GlobalScope.launch(Dispatchers.Main) {
                    eventSink?.success(AudioEvent.vadRecording(
                        RecordedSegment(
                            byteArrayOf(),
                            RecordedSegment.Type.LAST
                        )
                    ))
                }
            }

            override fun onVADTimeout() {
                GlobalScope.launch(Dispatchers.Main) {
                    eventSink?.success(AudioEvent.vadTimeout())
                }
            }

            override fun onVADEnd() {
                GlobalScope.launch(Dispatchers.Main) {
                    eventSink?.success(AudioEvent.vadEnd())
                }
            }

        })
    }

    override fun onMethodCall(call: MethodCall, result: Result) {
        when (call.method) {
            "getPlatformVersion" -> {
                result.success("Android ${android.os.Build.VERSION.RELEASE}")
            }

            Constants.initFaceDetect -> {
                initFaceDetectModel()
                result.success("success")
            }

            Constants.detectFace -> {
                val filePath = call.argument<String>("imagePath")
                detectFace(result, filePath)
            }

            Constants.initTriggerWord -> {
                triggerWordDetectionFlow.initModel()
                result.success("success")
            }

            Constants.initVAD -> {
                voiceActivityDetectionFlow.initModel()
                result.success("success")
            }

            Constants.startVAD -> {
                val timeoutInMilliseconds = call.argument<Int>("timeoutInMilliseconds")

                voiceActivityDetectionFlow.startRecording(timeoutInMilliseconds ?: VAD_TIMEOUT_IN_MILLISECONDS)

                result.success("success")
            }

            Constants.startTriggerWord -> {
                triggerWordDetectionFlow.startListening()
            }

            Constants.stopTriggerWord -> {
                triggerWordDetectionFlow.stop()
            }

            Constants.stopVAD -> {
                voiceActivityDetectionFlow.stop()
                result.success("success")
            }
            Constants.playWaveform -> {
                val audio = call.argument<ArrayList<Short>>("audio")
                playWaveformAudio(audio?.toShortArray())
            }

            else -> {
                result.notImplemented()
            }
        }
    }

    override fun onDetachedFromEngine(binding: FlutterPlugin.FlutterPluginBinding) {
        channel.setMethodCallHandler(null)
        eventChannel.setStreamHandler(null)
        vadAudioRecord?.release()
        audioTrack?.stop()
        audioTrack?.release()
        triggerWordAudioRecord?.release()
        voiceActivityDetectionFlow.clearListeners()
        triggerWordDetectionFlow.clearListeners()
    }

    private fun decodeUri(selectedImage: Uri): Bitmap {

        // Decode image size
        val o = BitmapFactory.Options()
        o.inJustDecodeBounds = true
        BitmapFactory.decodeStream(context.contentResolver.openInputStream(selectedImage), null, o)

        // The new size we want to scale to

        // The new size we want to scale to
        val REQUIRED_SIZE = 400

        // Find the correct scale value. It should be the power of 2.

        // Find the correct scale value. It should be the power of 2.
        var width_tmp = o.outWidth
        var height_tmp = o.outHeight
        var scale = 1
        while (true) {
            if (width_tmp / 2 < REQUIRED_SIZE
                || height_tmp / 2 < REQUIRED_SIZE
            ) {
                break
            }
            width_tmp /= 2
            height_tmp /= 2
            scale *= 2
        }

        // Decode with inSampleSize

        // Decode with inSampleSize
        val o2 = BitmapFactory.Options()
        o2.inSampleSize = scale
        return BitmapFactory.decodeStream(
            context.contentResolver.openInputStream(selectedImage),
            null,
            o2
        )!!
    }

    private fun initFaceDetectModel() {
        faceDetect.initModel(assetManager)
    }

    private fun detectFace(result: Result, filePath: String?) {
        if (filePath != null) {
            Log.d("imagePath", filePath)
        } else {
            Log.d("imagePath", "null")
        }
        val file = File(filePath)
        if (file.exists()) {
            val bitmap: Bitmap = decodeUri(Uri.parse("file://$filePath"))
            val faceDetected = faceDetect.detectModel(bitmap)
            val jsonString = JSONStringer().apply {
                array()
                for (face in faceDetected) {
                    `object`()
                    key("x").value(face.x)
                    key("y").value(face.y)
                    key("w").value(face.w)
                    key("h").value(face.h)
                    endObject()
                }
                endArray()
            }.toString()

            result.success(jsonString)

        } else {
            Log.d("imagePath", "file not exists")
        }

    }

    private fun detectTriggerWord(result: Result, filePath: String?) {
        val waveFormExtractor = WaveformExtractor(
            context = context,
            methodChannel = channel,
            expectedPoints = 100,
            key = "audioPath",
            result = result,
            path = filePath ?: "",
            extractorCallBack = object : ExtractorCallBack {
                override fun onProgress(value: ArrayList<Float>) {
//                    Log.d("TriggerWord", value.toString())
//                    println(value)
                    val fileName = "newFile.txt"

                    // Create a File object with the specified file name
                    val file = File(context.filesDir, fileName)

                    try {
                        // Create a new file
                        if (file.createNewFile()) {
                            println("File created successfully at: ${file.absolutePath}")
                        } else {
                            println("File already exists.")
                        }
                    } catch (e: Exception) {
                        println("Error creating file: $e")
                    }

                    file.printWriter().use { out ->
                        value.forEach { out.println(it) }
                    }
                    Log.d("TriggerWord", file.absolutePath)

                    val triggerWord = triggerWord.convModelDetect(value.toFloatArray())
                    result.success(triggerWord?.score?.toString() ?: "null")
                }
            }
        )
        waveFormExtractor.startDecode()
        waveFormExtractor.stop()
    }

//    private fun detectVAD(result: Result, filePath: String?) {
//        val waveFormExtractor = WaveformExtractor(
//            context = context,
//            methodChannel = channel,
//            expectedPoints = 100,
//            key = "audioPath",
//            result = result,
//            path = filePath ?: "",
//            extractorCallBack = object : ExtractorCallBack {
//                override fun onProgress(value: ArrayList<Float>) {
//                    Log.d("VAD", value.toString())
//                    println(value)
//                    val triggerWord = vad.detectVAD(value.toFloatArray())
//                    result.success(triggerWord?.score?.toString() ?: "null")
//                }
//            }
//        )
//        waveFormExtractor.startDecode()
//        waveFormExtractor.stop()
//    }

    private fun playWaveformAudio(audio : ShortArray?) {
        if (audio == null ) {
            return;
        }
        isSpeaking = true
        audioTrack?.play()
        audioTrack?.write(audio, 0, audio.size, AudioTrack.WRITE_BLOCKING)
        audioTrack?.stop()
        isSpeaking = false
    }

    override fun onListen(arguments: Any?, events: EventChannel.EventSink?) {
        eventSink = events
    }

    override fun onCancel(arguments: Any?) {
        eventSink = null
    }
}
