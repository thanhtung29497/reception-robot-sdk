import 'dart:typed_data';

import 'package:flutter_test/flutter_test.dart';
import 'package:smart_robot/smart_robot.dart';
import 'package:smart_robot/smart_robot_platform_interface.dart';
import 'package:smart_robot/smart_robot_method_channel.dart';
import 'package:plugin_platform_interface/plugin_platform_interface.dart';

class MockSmartRobotPlatform
    with MockPlatformInterfaceMixin
    implements SmartRobotPlatform {

  @override
  Future<String?> getPlatformVersion() => Future.value('42');

  @override
  Future<String?> detectFace(String imagePath) {
    // TODO: implement detectModel
    throw UnimplementedError();
  }

  @override
  Future<String?> initFaceDetectModel() {
    // TODO: implement initModel
    throw UnimplementedError();
  }

  @override
  Future<String?> detectTriggerWord(String audioPath) {
    // TODO: implement detectTriggerWord
    throw UnimplementedError();
  }

  @override
  Future<String?> initTriggerWordModel() {
    // TODO: implement initTriggerWordModel
    throw UnimplementedError();
  }

  @override
  Future<String?> detectVAD(String audioPath) {
    // TODO: implement detectVAD
    throw UnimplementedError();
  }

  @override
  Future<String?> initVADModel() {
    // TODO: implement initVADModel
    throw UnimplementedError();
  }

  @override
  Future<void> startTriggerWord() {
    // TODO: implement startRecord
    throw UnimplementedError();
  }

  @override
  Future<void> stopTriggerWord() {
    // TODO: implement stopRecord
    throw UnimplementedError();
  }

  @override
  Future<void> stopVAD() {
    // TODO: implement stopVAD
    throw UnimplementedError();
  }

  @override
  Future<void> playWaveformAudio(Int16List audioData) {
    // TODO: implement playWaveformAudio
    throw UnimplementedError();
  }

  @override
  Future<void> startVAD([int? timeoutInMilliseconds]) {
    // TODO: implement startVAD
    throw UnimplementedError();
  }
}

void main() {
  final SmartRobotPlatform initialPlatform = SmartRobotPlatform.instance;

  test('$MethodChannelSmartRobot is the default instance', () {
    expect(initialPlatform, isInstanceOf<MethodChannelSmartRobot>());
  });

  test('getPlatformVersion', () async {
    SmartRobot smartRobotPlugin = SmartRobot();
    MockSmartRobotPlatform fakePlatform = MockSmartRobotPlatform();
    SmartRobotPlatform.instance = fakePlatform;

    expect(await smartRobotPlugin.getPlatformVersion(), '42');
  });
}
