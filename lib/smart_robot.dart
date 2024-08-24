
import 'dart:typed_data';

import 'package:smart_robot/smart_robot_method_channel.dart';
import 'package:flutter/services.dart';
import 'audio_event.dart';
import 'audio_event_listener.dart';
import 'smart_robot_platform_interface.dart';

class SmartRobot {
  final _eventChannel = const EventChannel('smart_robot_event');
  final _audioEventListeners = <AudioEventListener>[];

  SmartRobot() {
    SmartRobotPlatform.instance = MethodChannelSmartRobot();
    _handleEvent();
  }

  void _handleEvent() {
    _eventChannel.receiveBroadcastStream().listen((event) {
      if (event is Map<dynamic, dynamic>) {
        switch (event["type"]) {
          case AudioEventType.triggerWordDetected:
            for (final listener in _audioEventListeners) {
              listener.onTriggerWordDetected();
            }
            break;
          case AudioEventType.vadRecording:
            final vadEvent = VADEvent.fromJson(event['data']);
            for (final listener in _audioEventListeners) {
              if (vadEvent.isLast) {
                listener.onSpeechEnd();
              } else {
                listener.onSpeaking(vadEvent);
              }
            }
            break;
          case AudioEventType.vadTimeout:
            for (final listener in _audioEventListeners) {
              listener.onSilenceTimeout();
            }
            break;
        }
      }
    });
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

  Future<void> startTriggerWord() {
    return SmartRobotPlatform.instance.startTriggerWord();
  }

  Future<void> stopTriggerWord() {
    return SmartRobotPlatform.instance.stopTriggerWord();
  }

  Future<void> stopVAD() {
    return SmartRobotPlatform.instance.stopVAD();
  }

  Future<void> startVAD([int? timeoutInMilliseconds]) {
    return SmartRobotPlatform.instance.startVAD(timeoutInMilliseconds);
  }

  Future<void> playWaveformAudio(Int16List audioData) {
    return SmartRobotPlatform.instance.playWaveformAudio(audioData);
  }

  Stream<dynamic> get speechDetectEvent {
    return _eventChannel.receiveBroadcastStream();
  }

  void addAudioEventListener(AudioEventListener listener) {
    _audioEventListeners.add(listener);
  }

  void removeAudioEventListener(AudioEventListener listener) {
    _audioEventListeners.remove(listener);
  }
}
