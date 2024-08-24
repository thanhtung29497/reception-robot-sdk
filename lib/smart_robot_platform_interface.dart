import 'dart:typed_data';

import 'package:plugin_platform_interface/plugin_platform_interface.dart';

import 'smart_robot_method_channel.dart';

abstract class SmartRobotPlatform extends PlatformInterface {
  /// Constructs a SmartRobotPlatform.
  SmartRobotPlatform() : super(token: _token);

  static final Object _token = Object();

  static SmartRobotPlatform _instance = MethodChannelSmartRobot();

  /// The default instance of [SmartRobotPlatform] to use.
  ///
  /// Defaults to [MethodChannelSmartRobot].
  static SmartRobotPlatform get instance => _instance;

  /// Platform-specific implementations should set this with their own
  /// platform-specific class that extends [SmartRobotPlatform] when
  /// they register themselves.
  static set instance(SmartRobotPlatform instance) {
    PlatformInterface.verifyToken(instance, _token);
    _instance = instance;
  }

  Future<String?> getPlatformVersion() {
    throw UnimplementedError('platformVersion() has not been implemented.');
  }

  Future<String?> initFaceDetectModel() {
    throw UnimplementedError('initFaceDetectModel() has not been implemented.');
  }

  Future<String?> initTriggerWordModel() {
    throw UnimplementedError('initTriggerWordModel() has not been implemented.');
  }

  Future<String?> detectFace(String imagePath) {
    throw UnimplementedError('detectFace() has not been implemented.');
  }

  Future<String?> detectTriggerWord(String audioPath) {
    throw UnimplementedError('detectTriggerWord() has not been implemented.');
  }

  Future<String?> initVADModel() {
    throw UnimplementedError('initVADModel() has not been implemented.');
  }

  Future<String?> detectVAD(String audioPath) {
    throw UnimplementedError('detectVAD() has not been implemented.');
  }

  Future<void> startTriggerWord() {
    throw UnimplementedError('startRecord() has not been implemented.');
  }

  Future<void> stopTriggerWord() {
      throw UnimplementedError('stopTriggerWord() has not been implemented.');
  }

  Future<void> startVAD([int? timeoutInMilliseconds]) {
    throw UnimplementedError('startVAD() has not been implemented.');
  }

  Future<void> stopVAD() {
    throw UnimplementedError('stopVAD() has not been implemented.');
  }

  Future<void> playWaveformAudio(Int16List audioData) {
    throw UnimplementedError('playWaveformAudio() has not been implemented.');
  }
}
