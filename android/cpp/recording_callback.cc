//
// Created by tannn on 1/8/24.
//

#include "logging_macros.h"
#include "recording_callback.h"

oboe::DataCallbackResult RecordingCallback::onAudioReady(oboe::AudioStream *audioStream, void *audioData, int32_t numFrames) {
    //LOGD(TAG, std::to_string(numFrames).c_str());
    return processRecordingFrames(audioStream, audioData, numFrames * audioStream->getChannelCount());
}

oboe::DataCallbackResult RecordingCallback::processRecordingFrames(oboe::AudioStream *audioStream, void *audioData,
                                                                   int32_t numFrames) {
    int32_t framesWritten = mSoundRecording->write(static_cast<float *>(audioData), numFrames);
//    if (framesWritten < numFrames) {
//        return oboe::DataCallbackResult::Stop;
//    }
    return oboe::DataCallbackResult::Continue;
}
