import 'dart:convert';
import 'dart:ffi';
import 'dart:io';
import 'package:flutter/material.dart';
import 'dart:async';
import 'package:flutter/services.dart';
import 'package:path_provider/path_provider.dart';
import 'package:record/record.dart';
import 'package:smart_robot/smart_robot.dart';
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

class _MyAppState extends State<MyApp> {
  String _platformVersion = 'Unknown';
  final _smartRobotPlugin = SmartRobot();
  final record = AudioRecorder();
  StreamSubscription? _triggerWordSubscription;
  bool isSpeech = false;
  String path = "";
  IOWebSocketChannel? channel;

  void _showAlertDialog(BuildContext context) {
    showDialog(
      context: context,
      builder: (BuildContext context) {
        return AlertDialog(
          title: const Text('Trigger Word Detected'),
          content: const Text('Start detecting VAD'),
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
  void initState() {
    super.initState();
    initPlatformState();
    initWebSocket();

    _triggerWordSubscription = _smartRobotPlugin.speechDetectEvent.listen((event) async {
      try {
        if (event == "start_vad") {
          _showAlertDialog(context);
        } else {
          // print("Event type: ${event['type']}");
          print("Event data type: ${event["data"]["type"]}");
          // print("Event audioSegment: ${event['data']['audioSegment']}");

          if (channel != null) {
            final flag = event["data"]["type"];
            final data = base64.encode(event["data"]["audioSegment"]);

            final message = jsonEncode({
                "data": data,
                "flag": flag == 2 ? 1 : flag,
              });

            channel!.sink.add(message);

            if (flag == 2) {
              channel!.sink.add(jsonEncode({
                "data": "",
                "flag": 2,
              }));
            }
          }
        }
      } catch (e) {
        print(e);
      }

    });
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
                onTap: () async {
                  await _smartRobotPlugin.startRecord();
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
                  await _smartRobotPlugin.startVAD();
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
