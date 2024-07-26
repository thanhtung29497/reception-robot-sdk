//
// Created by tannn on 16/01/2024.
//

#ifndef SMARTROBOT_VAD_CALLBACK_H
#define SMARTROBOT_VAD_CALLBACK_H

#include <thread>
#include "sound_recording.h"
#include <android/asset_manager_jni.h>
#include "rkai.h"

class VADCallback {
private:
    const char *TAG = "VADCallback:: %s";
    SoundRecording *mSoundRecording = nullptr;
    int mSampleRate = 16000;
    int mNumChannels = 1;

    // window size
    unsigned int mWindowKernelSize = 1; // seconds
    float mWindowStride = 0.3; // seconds
    float mWindowOverlap = 0.7; // seconds

    int currentStartIdx = 0;
    int currentEndIdx = mSampleRate;

    int isRunning = false;
    int isTriggered = 0;
    int isNotifiedTrigger = 0;

    rkai_handle_t mRkaiVadHandle = nullptr;
    rkai_melspectrogram_config_t mVadModelConfig;

public:
    VADCallback() = default;

    explicit VADCallback(SoundRecording *soundRecording, AAssetManager *mgr) {
        mSoundRecording = soundRecording;
        mRkaiVadHandle = rkai_create_handle();
        rkai_ret_t ret;
        ret = rkai_init_vad_android_model(mRkaiVadHandle, mgr);
        if (ret != RKAI_RET_SUCCESS) {
            LOG_ERROR("Failed to init vad model");
        }
        mVadModelConfig = rkai_get_vad_config();
    };

    void runVadThread();

    void start();

    void stop();
};
#endif //SMARTROBOT_VAD_CALLBACK_H
