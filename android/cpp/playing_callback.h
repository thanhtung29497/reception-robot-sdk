//
// Created by tannn on 1/8/24.
//

#ifndef SMARTROBOT_PLAYING_CALLBACK_H
#define SMARTROBOT_PLAYING_CALLBACK_H

#include <oboe/Definitions.h>
#include <oboe/AudioStream.h>
#include "sound_recording.h"
#include "logging_macros.h"

class PlayingCallback : public oboe::AudioStreamCallback {
private:
    const char* TAG = "PlayingCallback:: %s";
    SoundRecording* mSoundRecording = nullptr;
    SndfileHandle* mFileHandle = nullptr;
    bool isPlaybackFromFile = false;

public:
    PlayingCallback() = default;

    explicit PlayingCallback(SoundRecording* recording, SndfileHandle* sndfileHandle) {
        mSoundRecording = recording;
        mFileHandle = sndfileHandle;
    };

    bool isPlayingFromFile() { return isPlaybackFromFile; };

    void setPlaybackFromFile(bool isFile) { isPlaybackFromFile = isFile; };

    oboe::DataCallbackResult
    onAudioReady(oboe::AudioStream *audioStream, void *audioData, int32_t numFrames);

    oboe::DataCallbackResult
    processPlaybackFrames(oboe::AudioStream *audioStream, void *audioData, int32_t numFrames);

};

#endif //SMARTROBOT_PLAYING_CALLBACK_H
