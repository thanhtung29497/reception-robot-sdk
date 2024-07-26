//
// Created by tannn on 1/8/24.
//

#ifndef SMARTROBOT_TRIGGERWORD_CALLBACK_H
#define SMARTROBOT_TRIGGERWORD_CALLBACK_H

#include <thread>
#include "sound_recording.h"
#include <android/asset_manager_jni.h>
#include "rkai.h"

class TriggerCallback {
private:
    const char* TAG = "TriggerCallback:: %s";
    SoundRecording* mSoundRecording = nullptr;
    int mSampleRate = 8000;
    int mNumChannels = 1;
    // window size
    unsigned int mWindowKernelSize = 1; // seconds
    float mWindowStride = 0.3 ; // seconds
    float mWindowOverlap = 0.7; // seconds

    int currentStartIdx = 0;
    int currentEndIdx = mSampleRate;

    int isRunning = false;
    int isTriggered = 0;
    int isNotifiedTrigger = 0;

    rkai_handle_t mRkaiTriggerBCHandle = nullptr;
    rkai_handle_t mRkaiTriggerConvHandle = nullptr;
    rkai_melspectrogram_config_t mTriggerWordModelConfigBc;
    rkai_melspectrogram_config_t mTriggerWordModelConfigConv;



public:
    TriggerCallback() = default;
    explicit TriggerCallback(SoundRecording* soundRecording, AAssetManager *mgr){
        mSoundRecording = soundRecording;
        mRkaiTriggerBCHandle = rkai_create_handle();
        mRkaiTriggerConvHandle = rkai_create_handle();
        rkai_ret_t ret;
        ret = rkai_init_trigger_word_android_bc_model(mRkaiTriggerBCHandle, mgr);
        if (ret != RKAI_RET_SUCCESS) {
            LOG_ERROR("Failed to init trigger word bc model");
        }
        ret = rkai_init_trigger_word_android_conv_model(mRkaiTriggerConvHandle, mgr);
        if (ret != RKAI_RET_SUCCESS) {
            LOG_ERROR("Failed to init trigger word conv model");
        }
        mTriggerWordModelConfigBc = rkai_get_bc_config();
        mTriggerWordModelConfigConv = rkai_get_conv_config();
    };

    int getIsTriggered() {
        return isTriggered;
    };

    void runTriggerThread();

    void start();

    void stop();

};

#endif //SMARTROBOT_TRIGGERWORD_CALLBACK_H
