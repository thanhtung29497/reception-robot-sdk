import 'dart:convert';
import 'dart:developer';
import 'dart:io';
import 'package:flutter/material.dart';
import 'dart:async';
import 'package:flutter/services.dart';
import 'package:record/record.dart';
import 'package:smart_robot/audio_event.dart';
import 'package:smart_robot/audio_event_listener.dart';
import 'package:smart_robot/smart_robot.dart';
import 'package:fluttertoast/fluttertoast.dart';
import 'package:web_socket_channel/io.dart';
import 'package:web_socket_channel/web_socket_channel.dart';

void main() {
  runApp(const MaterialApp(home: MyApp()));
}

class MyApp extends StatefulWidget {
  const MyApp({super.key});

  @override
  State<MyApp> createState() => _MyAppState();
}

class _MyAppState extends State<MyApp> implements AudioEventListener {
  String _platformVersion = 'Unknown';
  final _smartRobotPlugin = SmartRobot();
  final record = AudioRecorder();
  bool isSpeech = false;
  String path = "";
  IOWebSocketChannel? channel;

  void _showAlertDialog(BuildContext context, String title, String content) {
    showDialog(
      context: context,
      builder: (BuildContext context) {
        return AlertDialog(
          title: Text(title),
          content: Text(content),
          actions: [
            TextButton(
              onPressed: () {
                Navigator.of(context).pop();
              },
              child: const Text('OK'),
            ),
          ],
        );
      },
    );
  }

  @override
  void onTriggerWordDetected() {
    Fluttertoast.showToast(
        msg: "Trigger word detected",
        toastLength: Toast.LENGTH_LONG,
        gravity: ToastGravity.TOP,
        fontSize: 24.0,
        backgroundColor: const Color(0xCCFFFFFF),
        textColor: Colors.black,
    );
    // _smartRobotPlugin.stopTriggerWord();
    // _smartRobotPlugin.startVAD(30000);
  }

  @override
  void onSpeaking(VADEvent event) {
    if (channel != null) {
      final data = base64.encode(event.audioSegment);

      final message = jsonEncode({
        "data": data,
        "flag": event.type.index,
      });

      channel!.sink.add(message);
    }
  }

  @override
  void onSpeechEnd() {
    if (channel != null) {
      channel!.sink.add(jsonEncode({
        "data": "",
        "flag": 2,
      }));
    }
  }

  @override
  void onSilenceTimeout() {
    Fluttertoast.showToast(msg: "VAD Timeout");
    _smartRobotPlugin.stopVAD();
    // _smartRobotPlugin.startTriggerWord();
  }

  @override
  void initState() {
    super.initState();
    initPlatformState();
    initWebSocket();
    _smartRobotPlugin.addAudioEventListener(this);
  }

  Future<void> deleteFile(String path) async {
    final File file = File(path);
    if (file.existsSync()) {
      file.deleteSync();
      print('Audio file deleted: $path');
    }
  }

  Future<void> initWebSocket() async {
    try {

      // replace host, deviceToken and lang with your own
      const host = "ws://180.93.182.208:8888/api/v1/chatbot/streaming";
      const deviceToken = "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJzdWIiOiJTUjAwMSJ9.zwygxNNeu1MH6pF3UZohspd8i8Ca6IYtE3jmoHDNBqQ";
      const lang = "vi";

      // connect to websocket
      final wsUrl = Uri.parse("$host?token=$deviceToken&lang=$lang");
      channel = IOWebSocketChannel.connect(wsUrl);
      await channel?.ready;

      print("Websocket server connected!");

      // listen to websocket
      channel?.stream.listen((event) {
        print("Event from websocket: $event");
        // display speech-to-text result here
      }, onError: (error) {
        print("Error from websocket: $error");
      }, onDone: () {
        print("Websocket is done");
      });
    } on WebSocketChannelException catch (e) {
      print("Error when connect to websocket: $e");
    }
  }

  // Platform messages are asynchronous, so we initialize in an async method.
  Future<void> initPlatformState() async {
    String platformVersion;
    // Platform messages may fail, so we use a try/catch PlatformException.
    // We also handle the message potentially returning null.
    try {
      platformVersion = await _smartRobotPlugin.getPlatformVersion() ??
          'Unknown platform version';
    } on PlatformException {
      platformVersion = 'Failed to get platform version.';
    }

    try {
      platformVersion = await _smartRobotPlugin.initTriggerWordModel() ??
          'Unknown platform version';
    } on PlatformException {
      platformVersion = 'Failed to init model.';
    }

    try {
      platformVersion = await _smartRobotPlugin.initVADModel() ??
          'Unknown platform version';
    } on PlatformException {
      platformVersion = 'Failed to init model.';
    }

    // If the widget was removed from the tree while the asynchronous platform
    // message was in flight, we want to discard the reply rather than calling
    // setState to update our non-existent appearance.
    if (!mounted) return;

    setState(() {
      _platformVersion = platformVersion;
    });
  }

  @override
  void dispose() {
    super.dispose();
    _smartRobotPlugin.removeAudioEventListener(this);
    channel?.sink.close();
  }

  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      home: Scaffold(
        appBar: AppBar(
          title: const Text('Plugin example app'),
        ),
        body: Center(
          child: Column(
            children: [
              Text('Running on: $_platformVersion\n'),
              InkWell(
                onTap: () => _smartRobotPlugin.startTriggerWord(),
                child: Container(
                    height: 50,
                    width: 200,
                    decoration: const BoxDecoration(
                      borderRadius: BorderRadius.all(Radius.circular(10)),
                      color: Colors.blueAccent,
                    ),
                    child: const Center(
                      child: Text(
                        'Start Trigger Word',
                        style: TextStyle(
                          color: Colors.white,
                          fontSize: 20,
                        ),
                      ),
                    )),
              ),
              const SizedBox(
                height: 20,
              ),
              InkWell(
                onTap: () async {
                  await _smartRobotPlugin.stopTriggerWord();
                },
                child: Container(
                    height: 50,
                    width: 200,
                    decoration: const BoxDecoration(
                      borderRadius: BorderRadius.all(Radius.circular(10)),
                      color: Colors.blueAccent,
                    ),
                    child: const Center(
                      child: Text(
                        'Stop Trigger Word',
                        style: TextStyle(
                          color: Colors.white,
                          fontSize: 20,
                        ),
                      ),
                    )),
              ),
              const SizedBox(
                height: 20,
              ),
              InkWell(
                onTap: () async {
                  await _smartRobotPlugin.startVAD(30000);
                },
                child: Container(
                    height: 50,
                    width: 200,
                    decoration: const BoxDecoration(
                      borderRadius: BorderRadius.all(Radius.circular(10)),
                      color: Colors.blueAccent,
                    ),
                    child: const Center(
                      child: Text(
                        'Start VAD',
                        style: TextStyle(
                          color: Colors.white,
                          fontSize: 20,
                        ),
                      ),
                    )),
              ),
              const SizedBox(
                height: 20,
              ),
              InkWell(
                onTap: () async {
                  await _smartRobotPlugin.stopVAD();
                },
                child: Container(
                    height: 50,
                    width: 200,
                    decoration: const BoxDecoration(
                      borderRadius: BorderRadius.all(Radius.circular(10)),
                      color: Colors.blueAccent,
                    ),
                    child: const Center(
                      child: Text(
                        'Stop VAD',
                        style: TextStyle(
                          color: Colors.white,
                          fontSize: 20,
                        ),
                      ),
                    )),
              ),
            ],
          ),
        ),
      ),
    );
  }
}
