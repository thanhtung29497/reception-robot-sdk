import 'dart:typed_data';

import 'package:flutter/foundation.dart';
import 'package:flutter/services.dart';

import 'smart_robot_platform_interface.dart';

/// An implementation of [SmartRobotPlatform] that uses method channels.
class MethodChannelSmartRobot extends SmartRobotPlatform {
  /// The method channel used to interact with the native platform.
  @visibleForTesting
  final methodChannel = const MethodChannel('smart_robot');

  @override
  Future<String?> getPlatformVersion() async {
    final version = await methodChannel.invokeMethod<String>('getPlatformVersion');
    return version;
  }

  @override
  Future<String?> initFaceDetectModel() async {
    final result = await methodChannel.invokeMethod<String>('initFaceDetectModel');
    return result;
  }

  @override
  Future<String?> detectFace(String imagePath) async {
    final result = await methodChannel.invokeMethod<String>('detectFace', {
      'imagePath': imagePath,
    } );
    return result;
  }


  @override
  Future<String?> initTriggerWordModel() async {
    final result = await methodChannel.invokeMethod<String>('initTriggerWordModel');
    return result;
  }

  @override
  Future<String?> detectTriggerWord(String audioPath) async {
    final result = await methodChannel.invokeMethod<String?>('detectTriggerWord', {
      'audioPath': audioPath,
    } );
    return result;
  }

  @override
  Future<String?> initVADModel() async {
    final result = await methodChannel.invokeMethod<String>('initVADModel');
    return result;
  }

  @override
  Future<String?> detectVAD(String audioPath) async {
    final result = await methodChannel.invokeMethod<String?>('detectVAD', {
      'audioPath': audioPath,
    } );
    return result;
  }

  @override
  Future<void> startRecord() async {
    await methodChannel.invokeMethod<void>('startRecord');
  }

  @override
  Future<void> stopTriggerWord() async {
    await methodChannel.invokeMethod<void>('stopTriggerWord');
  }

  @override
  Future<void> stopVAD() async {
    await methodChannel.invokeMethod<void>('stopVAD');
  }

  @override
  Future<void> playWaveformAudio(Int16List audioData) async {
    await methodChannel.invokeMethod<void>('playWaveformAudio',
        {'audio': audioData}
    );
  }
}
