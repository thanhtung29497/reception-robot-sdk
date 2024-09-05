import 'package:smart_robot/audio_event.dart';

abstract class AudioEventListener {
  void onTriggerWordDetected();
  void onSpeaking(VADEvent event);
  void onSpeechEnd();
  void onSilenceTimeout();
}