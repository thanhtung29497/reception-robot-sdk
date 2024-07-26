//
// Created by tannn on 1/7/24.
//

#ifndef SMARTROBOT_AUDIO_ENGINE_H
#define SMARTROBOT_AUDIO_ENGINE_H

#include <oboe/AudioStream.h>
#include <oboe/Definitions.h>
#include <sndfile.hh>
#include "sound_recording.h"
#include "recording_callback.h"
#include "playing_callback.h"
#include "triggerword_callback.h"
#include "vad_callback.h"
#include <android/asset_manager_jni.h>


class AudioEngine {
public:
    AudioEngine(AAssetManager *mgr);
    ~AudioEngine();

    RecordingCallback recordingCallback = RecordingCallback(&mSoundRecording);
    PlayingCallback playingCallback = PlayingCallback(&mSoundRecording, &sndfileHandle);
    TriggerCallback triggerWordCallback = TriggerCallback(&mSoundRecording, mgr);
    VADCallback vadCallback = VADCallback(&mSoundRecording, mgr);


    void startRecording();
    void stopRecording();
    void startPlayingRecordedStream();
    void stopPlayingRecordedStream();
    void startPlayingFromFile(const char* filaPath);
    void stopPlayingFromFile();
    void writeToFile(const char* filePath);


private:
    const char *TAG = "AudioEngine:: %s";
    int32_t mRecodingDeviceId = oboe::VoiceRecognition;
    int32_t mPlaybackDeviceId = 6;

    oboe::AudioFormat mFormat = oboe::AudioFormat::Float;
    int32_t mSampleRate = 16000;
    int32_t mFramesPerBurst;
    int32_t mInputChannelCount = oboe::ChannelCount::Mono;
    int32_t mOutputChannelCount = oboe::ChannelCount::Mono;

    oboe::AudioApi mAudioApi = oboe::AudioApi::OpenSLES;
    oboe::AudioStream *mRecordingStream = nullptr;
    oboe::AudioStream *mPlaybackStream = nullptr;
    SoundRecording mSoundRecording;
    SndfileHandle sndfileHandle;

    AAssetManager *mgr;

    void openRecordingStream();
    void openPlaybackStreamFromRecordedStreamParameters();
    void openPlaybackStreamFromFileParameters();

    void startStream(oboe::AudioStream *stream);
    void stopStream(oboe::AudioStream *stream);
    void closeStream(oboe::AudioStream *stream);

    oboe::AudioStreamBuilder *setUpRecordingStreamParameters(oboe::AudioStreamBuilder *builder);
    oboe::AudioStreamBuilder *setUpPlaybackStreamParameters(oboe::AudioStreamBuilder *builder,
                                                            oboe::AudioApi audioApi,
                                                            oboe::AudioFormat audioFormat,
                                                            oboe::AudioStreamCallback *audioStreamCallback,
                                                            int32_t deviceId, int32_t sampleRate,
                                                            int channelCount);
};

#endif //SMARTROBOT_AUDIO_ENGINE_H
