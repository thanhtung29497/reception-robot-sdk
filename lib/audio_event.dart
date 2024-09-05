class AudioEventType {
  static const triggerWordDetected = 'TRIGGER_WORD_DETECTED';
  static const vadRecording = 'VAD_RECORDING';
  static const vadTimeout = 'VAD_TIMEOUT';
  static const vadEnd = 'VAD_END';
}

enum VADEventType {
  firstFrame,
  continuousFrame,
  lastFrame
}

class VADEvent {
  final VADEventType type;
  final List<int> audioSegment;
  final bool isLast;

  VADEvent(this.type, this.audioSegment, this.isLast);

  factory VADEvent.fromJson(Map<dynamic, dynamic> json) {
    final type = json['type'];
    final eventType = VADEventType.values[type];

    return VADEvent(
      eventType,
      List<int>.from(json['audioSegment']),
      eventType == VADEventType.lastFrame,
    );
  }
}