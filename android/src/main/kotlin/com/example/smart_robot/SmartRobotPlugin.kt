package com.example.smart_robot

import android.Manifest
import android.content.Context
import android.content.pm.PackageManager
import android.content.res.AssetManager
import android.graphics.Bitmap
import android.graphics.BitmapFactory
import android.media.AudioFormat
import android.media.AudioManager
import android.media.AudioRecord
import android.media.AudioTrack
import android.media.MediaRecorder
import android.net.Uri
import android.util.Log
import androidx.core.app.ActivityCompat
import com.example.smart_robot.event.AudioEvent
import com.example.smart_robot.event.RecordedSegment
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
import java.nio.ByteBuffer
import java.nio.ByteOrder
import kotlin.math.abs
import kotlin.math.log10


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

    private lateinit var vad: VAD

    private lateinit var context: Context

    private var triggerWordAudioRecord: AudioRecord? = null

    private var vadAudioRecord: AudioRecord? = null

    private var audioTrack : AudioTrack? = null

    private var audioWriter: AudioWriter = AudioWriter()

    private var isSpeaking = false


    override fun onAttachedToEngine(flutterPluginBinding: FlutterPlugin.FlutterPluginBinding) {
        channel = MethodChannel(flutterPluginBinding.binaryMessenger, "smart_robot")
        channel.setMethodCallHandler(this)
        eventChannel = EventChannel(flutterPluginBinding.binaryMessenger, "smart_robot_event")
        eventChannel.setStreamHandler(this)

        faceDetect = FaceDetect()
        triggerWord = TriggerWord()
        vad = VAD()
        audioTrack = AudioTrack(
            AudioManager.STREAM_MUSIC,
            22050,
            AudioFormat.CHANNEL_OUT_MONO,
            AudioFormat.ENCODING_PCM_16BIT,
            22050,
            AudioTrack.MODE_STREAM
        )
        context = flutterPluginBinding.applicationContext
        assetManager = flutterPluginBinding.applicationContext.assets
    }

    companion object {
        private lateinit var assetManager: AssetManager

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
                initTriggerWordModel()
                result.success("success")
            }

            Constants.detectTriggerWord -> {
                val filePath = call.argument<String>("audioPath")
                detectTriggerWord(result, filePath)
            }

            Constants.initVAD -> {
                initVADModel()
                result.success("success")
            }

            Constants.detectVAD -> {
                val filePath = call.argument<String>("audioPath")
                detectVAD(result, filePath)
            }

            Constants.startVAD -> {
                startVAD()
                result.success("success")
            }

            Constants.startRecord -> {
                startTriggerWord()
            }

            Constants.stopTriggerWord -> {
                stopTriggerWord()
            }

            Constants.stopVAD -> {
                println("Stop record")
                stopTriggerWord()
                stopVAD()
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

    private fun initTriggerWordModel() {
        triggerWord.initModel(assetManager)
    }

    private fun initVADModel() {
        vad.initVADModel(assetManager)
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

    private fun detectVAD(result: Result, filePath: String?) {
        val waveFormExtractor = WaveformExtractor(
            context = context,
            methodChannel = channel,
            expectedPoints = 100,
            key = "audioPath",
            result = result,
            path = filePath ?: "",
            extractorCallBack = object : ExtractorCallBack {
                override fun onProgress(value: ArrayList<Float>) {
                    Log.d("VAD", value.toString())
                    println(value)
                    val triggerWord = vad.detectVAD(value.toFloatArray())
                    result.success(triggerWord?.score?.toString() ?: "null")
                }
            }
        )
        waveFormExtractor.startDecode()
        waveFormExtractor.stop()
    }

    private fun startTriggerWord() {
        val bcLowThreshold = 0.6
        val bcHighThreshold = 0.8
        val convLowThreshold = 0.6
        val convHighThreshold = 0.8
        var bcThreshold = 0.6
        var convThreshold = 0.8
        var lastBCScore = 0f
        var lastConvScore = 0f
        val bufferSize = AudioRecord.getMinBufferSize(
            8000,
            AudioFormat.CHANNEL_IN_MONO,
            AudioFormat.ENCODING_PCM_FLOAT
        )
        if (ActivityCompat.checkSelfPermission(
                context,
                Manifest.permission.RECORD_AUDIO
            ) != PackageManager.PERMISSION_GRANTED
        ) {
            return
        }

        triggerWordAudioRecord = AudioRecord(
            MediaRecorder.AudioSource.MIC,
            8000,
            AudioFormat.CHANNEL_IN_MONO,
            AudioFormat.ENCODING_PCM_FLOAT,
            bufferSize
        )

        if (triggerWordAudioRecord?.state == AudioRecord.STATE_INITIALIZED) {
            GlobalScope.launch(Dispatchers.IO) {
                print("Start trigger word")
                triggerWordAudioRecord?.startRecording()
                val buffer = FloatArray(8000)
                while (triggerWordAudioRecord?.recordingState == AudioRecord.RECORDSTATE_RECORDING) {
                    val bytesRead: Int =
                        triggerWordAudioRecord?.read(buffer, 0, buffer.size, AudioRecord.READ_BLOCKING) ?: 0
                    if (bytesRead > 0) {
                        val db = 20 * log10(buffer.map { abs(it) }.max())
                        if (abs(db) > 0) {
                            if (lastBCScore > bcLowThreshold) {
                                bcThreshold = bcLowThreshold
                            }
                            triggerWord.bcModelDetect(buffer)?.apply {
                                print("BCScore: $score")

                                if (score > bcThreshold) {
                                    kotlinx.coroutines.withContext(Dispatchers.Main) {
                                        eventSink?.success(AudioEvent.triggerWordDetected())
                                    }
                                }
                            }


                            // Start conv model detect
//                            if (bcScore != null && bcScore.score > bcThreshold) {
//                                if (lastConvScore > convLowThreshold) {
//                                    convThreshold = convLowThreshold
//                                }
//                                val conv = triggerWord.convModelDetect(buffer)
//                                print("Conv Score: ")
//                                println(conv?.score)
//                                if (conv != null) {
//                                    kotlinx.coroutines.withContext(Dispatchers.Main) {
//                                        eventSink?.success("start_vad")
//                                    }
//                                    stopTriggerWord()
//                                    break
//                                }
//                                lastConvScore = conv?.score ?: 0f
//                                if (lastConvScore < convLowThreshold) {
//                                    convThreshold = convHighThreshold
//                                }
//                            }
//                            lastBCScore = bcScore?.score ?: 0f
//                            if (lastBCScore < bcLowThreshold) {
//                                bcThreshold = bcHighThreshold
//                            }
                        } else {
                            continue
                        }
                    }
                }
            }

        }
    }

    // Convert PCM PCM 16-bit to PCM float
    private fun pcm16toFloat(pcms: ByteArray): FloatArray {
        val floaters = FloatArray(pcms.size)
        for (i in pcms.indices) {
            floaters[i] = pcms[i] / 32768.0f
        }
        return floaters
    }

    // Convert PCM float to PCM 16-bit, then convert to ByteArray
    private fun pcmFloatTo16(pcms: FloatArray): ShortArray {
        val shorts = ShortArray(pcms.size)
        for (i in pcms.indices) {
            shorts[i] = (pcms[i] * 32768).toInt().toShort()
        }
        return shorts
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

    private fun startVAD() {
        var audio: ArrayList<Float> = arrayListOf()

        val bufferSize = AudioRecord.getMinBufferSize(
            16000,
            AudioFormat.CHANNEL_IN_MONO,
            AudioFormat.ENCODING_PCM_FLOAT
        )
        if (ActivityCompat.checkSelfPermission(
                context,
                Manifest.permission.RECORD_AUDIO
            ) != PackageManager.PERMISSION_GRANTED
        ) {
            return
        }
        vadAudioRecord = AudioRecord(
            MediaRecorder.AudioSource.MIC,
            16000,
            AudioFormat.CHANNEL_IN_MONO,
            AudioFormat.ENCODING_PCM_FLOAT,
            bufferSize
        )


        if (vadAudioRecord?.state == AudioRecord.STATE_INITIALIZED) {
            GlobalScope.launch(Dispatchers.IO) {
                vadAudioRecord?.startRecording()
                val buffer = FloatArray(4800)
                var isFirstSegment = true

                while (vadAudioRecord?.recordingState == AudioRecord.RECORDSTATE_RECORDING) {
                    val bytesRead: Int =
                        vadAudioRecord?.read(buffer, 0, buffer.size, AudioRecord.READ_BLOCKING) ?: 0

                    if (bytesRead > 0 && !isSpeaking) {
                        audio.addAll(buffer.toList())
                        if (audio.size >= 16000) {
                            val newAudio = audio.subList(0, 16000).toFloatArray()
                            audio = ArrayList(audio.subList(4800, audio.size))

                            print("Running VAD model")

                            vad.detectVAD(newAudio)?.apply {
                                val pcm16Audio = pcmFloatTo16(newAudio)
                                print("VAD Score: $score")
                                val recordedSegment = when {
                                    isSpeech && isFirstSegment -> {
                                        isFirstSegment = false
                                        RecordedSegment(shortArrayToByteArray(pcm16Audio), RecordedSegment.Type.FIRST)
                                    }

                                    isSpeech && !isFirstSegment ->
                                        RecordedSegment(
                                            shortArrayToByteArray(pcm16Audio.toList().subList(11200, 16000).toShortArray()),
                                            RecordedSegment.Type.CONTINUE
                                        )

                                    !isSpeech && !isFirstSegment -> {
                                        isFirstSegment = true
                                        RecordedSegment(
                                            shortArrayToByteArray(pcm16Audio.toList().subList(11200, 16000).toShortArray()),
                                            RecordedSegment.Type.LAST
                                        )
                                    }

                                    else -> null
                                }

                                recordedSegment?.let { segment ->
                                    GlobalScope.launch(Dispatchers.Main) {
                                        eventSink?.success(AudioEvent.vadRecording(segment))
                                    }
                                }

                            }
//                            println(result?.score)
//                            if (result?.isSpeech == true) {
//                                audioSegment.add(newAudio.toList() as ArrayList<Float>)
//
//                                kotlinx.coroutines.withContext(Dispatchers.Main) {
//                                    eventSink?.success(newAudio)
//                                }
//                            } else {
//                                if (audioSegment.isNotEmpty()) {
//                                    val fullChunk = audioSegment[0]
//                                    if (audioSegment.size > 1) {
//                                        for (i in 1 until audioSegment.size) {
//                                            fullChunk.addAll(audioSegment[i].subList(11200, 16000))
//                                        }
//                                    }
//                                    val filePath = context.filesDir.absolutePath + "/" + System.currentTimeMillis() +  ".wav"
//                                    audioWriter.writeWavFile(filePath, 16000, fullChunk.toFloatArray())
//                                    audioSegment.clear()
//                                    kotlinx.coroutines.withContext(Dispatchers.Main) {
//                                        eventSink?.success(fullChunk)
//                                    }
//                                }
//                            }
                        }
                    }
                }
            }

        }

    }

    private fun stopTriggerWord() {
        println("Trigger word state: ")
        println(triggerWordAudioRecord?.state)
        if (triggerWordAudioRecord?.state == AudioRecord.RECORDSTATE_RECORDING
            || triggerWordAudioRecord?.state == AudioRecord.READ_NON_BLOCKING
            || triggerWordAudioRecord?.state == AudioRecord.STATE_INITIALIZED) {
            triggerWordAudioRecord?.stop()
            triggerWordAudioRecord?.release()
        } else {
            return
        }
    }

    private fun stopVAD() {
        println("Stopping VAD Recording: ")
        println(vadAudioRecord?.state)
        if (vadAudioRecord?.state == AudioRecord.RECORDSTATE_RECORDING
            || vadAudioRecord?.state == AudioRecord.READ_NON_BLOCKING
            || vadAudioRecord?.state == AudioRecord.STATE_INITIALIZED) {
            vadAudioRecord?.stop()
            vadAudioRecord?.release()
        } else {
            return
        }

    }

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
