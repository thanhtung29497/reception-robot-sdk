import 'package:smart_robot/audio_event.dart';

abstract class AudioEventListener {
  void onTriggerWordDetected();
  void onVADRecording(VADEvent event);
  void onVADEnd();
  void onVADTimeout();
}