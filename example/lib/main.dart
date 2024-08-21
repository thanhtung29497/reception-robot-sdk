import 'dart:ffi';
import 'dart:io';
import 'package:flutter/material.dart';
import 'dart:async';
import 'package:flutter/services.dart';
import 'package:path_provider/path_provider.dart';
import 'package:record/record.dart';
import 'package:smart_robot/smart_robot.dart';

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

    _triggerWordSubscription = _smartRobotPlugin.speechDetectEvent.listen((event) async {
      try {
        if (event == "start_vad") {
          _showAlertDialog(context);
        } else {
          print("Event type: ${event['type']}");
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
                  // if (await record.hasPermission()) {
                  //   final Directory tempDir = await getTemporaryDirectory();
                  //   await record.start(
                  //       const RecordConfig(
                  //         sampleRate: 16000,
                  //         numChannels: 1,
                  //         encoder: AudioEncoder.wav,
                  //       ),
                  //       path:
                  //           '${tempDir.path}/${DateTime.now().millisecondsSinceEpoch}.wav');
                  //   // record.startStream(const RecordConfig(
                  //   //   sampleRate: 16000,
                  //   //   numChannels: 1,
                  //   //   encoder: AudioEncoder.wav,
                  //   // ));
                  //   // final stream = await record.startStream(const RecordConfig(encoder: AudioEncoder.aacLc
                  //   // ));
                  //   // stream.listen((data) {
                  //   //   print(data);
                  //   // });
                  //   await Future.delayed(const Duration(milliseconds: 10000));
                  //   String? path = await record.stop();
                  //   if (path != null) {
                  //     // try {
                  //     //   final waveformData = await _playerController
                  //     //       .extractWaveformData(path: path);
                  //     //   print(waveformData);
                  //     // } catch (e) {
                  //     //   print(e);
                  //     // }
                  //     // File file = File(path);
                  //     // print(file.readAsBytesSync());
                  //     // final a = await _smartRobotPlugin.detectTriggerWordModel(path);
                  //
                  //     print(path);
                  //
                  //     // String? result = await _speechProcessingPlugin.processAudio(path);
                  //   }
                  // } else {
                  //   print('No permission');
                  // }
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
