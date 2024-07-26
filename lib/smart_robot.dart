
import 'dart:typed_data';

import 'package:smart_robot/smart_robot_method_channel.dart';
import 'package:flutter/services.dart';
import 'smart_robot_platform_interface.dart';

class SmartRobot {
  final _eventChannel = const EventChannel('smart_robot_event');

  SmartRobot() {
    SmartRobotPlatform.instance = MethodChannelSmartRobot();
  }

  Future<String?> getPlatformVersion() {
    return SmartRobotPlatform.instance.getPlatformVersion();
  }

  Future<String?> initFaceDetectModel() {
    return SmartRobotPlatform.instance.initFaceDetectModel();
  }

  Future<String?> detectFace(String imagePath) {
    return SmartRobotPlatform.instance.detectFace(imagePath);
  }

  Future<String?> initTriggerWordModel() {
    return SmartRobotPlatform.instance.initTriggerWordModel();
  }

  Future<String?> detectTriggerWordModel(String imagePath) {
    return SmartRobotPlatform.instance.detectTriggerWord(imagePath);
  }

  Future<String?> initVADModel() {
    return SmartRobotPlatform.instance.initVADModel();
  }

  Future<String?> detectVAD(String audioPath) {
    return SmartRobotPlatform.instance.detectVAD(audioPath);
  }

  Future<void> startRecord() {
    return SmartRobotPlatform.instance.startRecord();
  }

  Future<void> stopTriggerWord() {
    return SmartRobotPlatform.instance.stopTriggerWord();
  }

  Future<void> stopVAD() {
    return SmartRobotPlatform.instance.stopVAD();
  }

  Future<void> playWaveformAudio(Int16List audioData) {
    return SmartRobotPlatform.instance.playWaveformAudio(audioData);
  }

  Stream<dynamic> get speechDetectEvent {
    return _eventChannel.receiveBroadcastStream();
  }
}
