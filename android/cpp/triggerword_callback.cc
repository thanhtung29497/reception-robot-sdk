//
// Created by tannn on 1/8/24.
//

#include "logging_macros.h"
#include "triggerword_callback.h"

void TriggerCallback::runTriggerThread() {
    // If having data in sound recording
    rkai_ret_t ret;
    rkai_audio_t audio_input;
    rkai_audio_t second_audio_input;
    memset(&audio_input, 0, sizeof(rkai_audio_t));
    memset(&second_audio_input, 0, sizeof(rkai_audio_t));
    float *audio_data = (float *) malloc(
            sizeof(float) * mSampleRate * mWindowKernelSize);
    float *second_audio_data = (float *) malloc(
            sizeof(float) * mSampleRate * mWindowKernelSize);
    while (isRunning) {
        if (mSoundRecording->getLength() > 0 &&
            currentStartIdx +  mSampleRate * mWindowKernelSize < mSoundRecording->getLength()) {
            // Get data from sound recording from current start index to current end index
            LOGD(TAG, "Running trigger word detection");
            LOG_INFO("Current start index %d", currentStartIdx);
            mSoundRecording->getData(audio_data, currentStartIdx, currentStartIdx + mSampleRate * mWindowKernelSize);
            mSoundRecording->getData(second_audio_data, currentStartIdx, currentStartIdx + mSampleRate * mWindowKernelSize);

            audio_input.data = audio_data;
            audio_input.sample_rate = mSampleRate;
            audio_input.n_channels = mNumChannels;
            audio_input.n_seconds = mWindowKernelSize;
            audio_input.size = mSampleRate * mWindowKernelSize;
            audio_input.format = RKAI_AUDIO_FORMAT_FLOAT;

            second_audio_input.data = second_audio_data;
            second_audio_input.sample_rate = mSampleRate;
            second_audio_input.n_channels = mNumChannels;
            second_audio_input.n_seconds = mWindowKernelSize;
            second_audio_input.size = mSampleRate * mWindowKernelSize;
            second_audio_input.format = RKAI_AUDIO_FORMAT_FLOAT;

            rkai_trigger_word_result_t trigger_word_result_bc;
            rkai_trigger_word_result_t trigger_word_result_conv;
//            ret = rkai_trigger_word_detect(mRkaiTriggerBCHandle, &audio_input,
//                                     mTriggerWordModelConfigBc, &trigger_word_result_bc, 0.3,
//                                     0.6);
            ret = rkai_trigger_word_detect(mRkaiTriggerBCHandle, &audio_input,
                                           mTriggerWordModelConfigBc,
                                           &trigger_word_result_bc,
                                           0.3, 0.6);
            if (ret != RKAI_RET_SUCCESS) {
                LOG_ERROR("Failed to detect trigger word bc model");
            }
            if (trigger_word_result_bc.pass_low_conf) {
                LOG_INFO("Trigger word bc pass low confidence");
                LOG_INFO("Trigger word bc result %f\n", trigger_word_result_bc.score);
            }
            if(trigger_word_result_bc.pass_high_conf){
                LOG_INFO("Trigger word bc pass high confidence");
                LOG_INFO("Trigger word bc result %f\n", trigger_word_result_bc.score);
            }

            ret = rkai_trigger_word_detect(mRkaiTriggerConvHandle, &second_audio_input,
                                           mTriggerWordModelConfigConv,
                                           &trigger_word_result_conv,
                                           0.6, 0.7);
            if (ret != RKAI_RET_SUCCESS) {
                LOG_ERROR("Failed to detect trigger word conv model");
            }

            LOG_INFO("Trigger word conv result %f\n", trigger_word_result_conv.score);
            if (trigger_word_result_conv.pass_low_conf) {
                LOG_INFO("Trigger word conv pass low confidence");
                LOG_INFO("Trigger word conv result %f\n", trigger_word_result_conv.score);
            }
            if(trigger_word_result_conv.pass_high_conf){
                LOG_INFO("Trigger word conv pass high confidence");
                LOG_INFO("Trigger word conv result %f\n", trigger_word_result_conv.score);
            }
            // Update current start index
            currentStartIdx += (int) (mSampleRate * mWindowKernelSize * mWindowStride);
        } else {
            // Wait for 10ms
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }
    rkai_audio_release(&audio_input);
    rkai_audio_release(&second_audio_input);
}

void TriggerCallback::start() {
    if (!isRunning) {
        LOGD(TAG, "TriggerCallback::start()");
        isRunning = true;
        std::thread triggerThread(&TriggerCallback::runTriggerThread, this);
        triggerThread.detach();
    }
}

void TriggerCallback::stop() {
    isRunning = false;
    currentStartIdx = 0;
    currentEndIdx = mSampleRate;
}

