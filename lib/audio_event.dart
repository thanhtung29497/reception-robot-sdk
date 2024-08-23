class AudioEventType {
  static const triggerWordDetected = 'TRIGGER_WORD_DETECTED';
  static const vadRecording = 'VAD_RECORDING';
  static const vadTimeout = 'VAD_TIMEOUT';
}

enum VADEventType {
  FIRST,
  CONTINUE,
  LAST
}

class VADEvent {
  final VADEventType type;
  final List<int> audioSegment;
  final bool isLast;

  VADEvent(this.type, this.audioSegment, this.isLast);

  factory VADEvent.fromJson(Map<dynamic, dynamic> json) {
    final type = json['type'];
    final eventType = VADEventType.values[type];

    // If the event type is LAST, we separate into two events: CONTINUE and LAST
    // The last CONTINUE event will contain the last audio segment
    // The LAST event only contains the flag
    return VADEvent(
      eventType == VADEventType.LAST ? VADEventType.CONTINUE : eventType,
      List<int>.from(json['audioSegment']),
      eventType == VADEventType.LAST,
    );
  }
}