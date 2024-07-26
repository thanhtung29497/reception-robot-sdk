//
// Created by tannn on 1/8/24.
//

#include "audio_utils.h"
#include "playing_callback.h"

oboe::DataCallbackResult PlayingCallback::onAudioReady(oboe::AudioStream *audioStream, void *audioData, int32_t numFrames) {
    return processPlaybackFrames(audioStream, audioData, numFrames * audioStream->getChannelCount());
}

oboe::DataCallbackResult PlayingCallback::processPlaybackFrames(oboe::AudioStream *audioStream, void *audioData,
                                                                int32_t numFrames) {
//    LOGD(TAG,"processPlaybackFrames() called");
    fillArrayWithZeros(static_cast<float *>(audioData), numFrames);

    int64_t framesWritten = 0;

    if (!isPlayingFromFile()) {
        framesWritten = mSoundRecording->read(static_cast<float *>(audioData), numFrames);
    } else {
        framesWritten = static_cast<sf_count_t>(mFileHandle->read(static_cast<int16_t *>(audioData), numFrames));
    }

    if (framesWritten == 0){
        LOGW(TAG, "No frames could be read from file");
        audioStream->requestStop();
        return oboe::DataCallbackResult::Stop;
    }
    return oboe::DataCallbackResult::Continue;
}