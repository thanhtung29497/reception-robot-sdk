//
// Created by tannn on 16/01/2024.
//

#include "logging_macros.h"
#include "vad_callback.h"

void VADCallback::runVadThread() {
    // If having data in sound recording
    rkai_ret_t ret;
    rkai_audio_t audio_input;
    memset(&audio_input, 0, sizeof(rkai_audio_t));

    float *audio_data = (float *) malloc(
            sizeof(float) * mSampleRate * mWindowKernelSize);

    while (isRunning) {
        // TODO: Check correct condition
        if (mSoundRecording->getLength() > 0 &&
        currentStartIdx + mSampleRate * mWindowKernelSize < mSoundRecording->getLength()){
            LOGD(TAG, "Running vad detection");
            mSoundRecording->getData(audio_data, currentStartIdx, currentStartIdx + mSampleRate * mWindowKernelSize);
            LOGD(TAG, "Done Get data from sound recording");
            audio_input.data = audio_data;
            audio_input.sample_rate = mSampleRate;
            audio_input.n_channels = mNumChannels;
            audio_input.n_seconds = mWindowKernelSize;
            audio_input.size = mSampleRate * mWindowKernelSize;
            audio_input.format = RKAI_AUDIO_FORMAT_FLOAT;

            rkai_vad_result_t vad_result;
            ret = rkai_vad_detect(mRkaiVadHandle, &audio_input,
                                  mVadModelConfig, &vad_result,
                                  0.3, 0.6);

            if (ret != RKAI_RET_SUCCESS) {
                LOG_ERROR("Failed to detect vad");
            }

            if (vad_result.is_speech) {
                LOG_INFO("VAD result is speech");
                LOG_INFO("VAD result %f\n", vad_result.conf);
            }
            else {
                LOG_INFO("VAD result is not speech");
                LOG_INFO("VAD result %f\n", vad_result.conf);
            }
            // Update current start index
            currentStartIdx += mSampleRate * mWindowKernelSize * mWindowStride;
        }
    }
    rkai_audio_release(&audio_input);
}

void VADCallback::start() {
    if (!isRunning) {
        isRunning = true;
        std::thread t(&VADCallback::runVadThread, this);
        t.detach();
    }
}

void VADCallback::stop() {
    isRunning = false;
    currentStartIdx = 0;
    currentEndIdx = mSampleRate;
}
