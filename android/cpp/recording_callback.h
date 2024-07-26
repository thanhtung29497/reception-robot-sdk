//
// Created by tannn on 1/8/24.
//

#ifndef SMARTROBOT_RECORDING_CALLBACK_H
#define SMARTROBOT_RECORDING_CALLBACK_H

#include <oboe/AudioStream.h>
#include <oboe/AudioStream.h>
#include "sound_recording.h"


class RecordingCallback : public oboe::AudioStreamCallback {
private:
    const char* TAG = "RecordingCallback:: %s";
    SoundRecording* mSoundRecording = nullptr;
public:
    RecordingCallback() = default;
    explicit RecordingCallback(SoundRecording* soundRecording){
        mSoundRecording = soundRecording;
    };

    oboe::DataCallbackResult onAudioReady(oboe::AudioStream *audioStream, void *audioData, int32_t numFrames);

    oboe::DataCallbackResult processRecordingFrames(oboe::AudioStream *audioStream, void *audioData, int32_t numFrames);
};
#endif //SMARTROBOT_RECORDING_CALLBACK_H
