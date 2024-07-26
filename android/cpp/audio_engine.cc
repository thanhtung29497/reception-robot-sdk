//
// Created by tannn on 1/7/24.
//

#include <stdlib.h>
#include <oboe/Oboe.h>
#include <assert.h>
#include "logging_macros.h"
#include "audio_engine.h"


AudioEngine::AudioEngine(AAssetManager *amgr) {
    assert(mOutputChannelCount == mInputChannelCount);
    mgr = amgr;
}

AudioEngine::~AudioEngine() {
    if (mRecordingStream != nullptr) {
        mRecordingStream->stop();
        mRecordingStream->close();
    }

    if (mPlaybackStream != nullptr) {
        mPlaybackStream->stop();
        mPlaybackStream->close();
    }
}

void AudioEngine::startRecording() {
    LOGD(TAG, "startRecording() called");
    openRecordingStream();
    if (mRecordingStream != nullptr) {
        startStream(mRecordingStream);
    } else {
        LOGE(TAG, "Failed to create recording stream (%p). Restart the app", mRecordingStream);
        closeStream(mRecordingStream);
    }
}


void AudioEngine::stopRecording() {
    LOGD(TAG, "stopRecording() called");
    stopStream(mRecordingStream);
    closeStream(mRecordingStream);
}

void AudioEngine::startPlayingRecordedStream() {
    LOGD(TAG, "startPlayingRecordedStream() called");
    openPlaybackStreamFromRecordedStreamParameters();
    if (mPlaybackStream != nullptr) {
        startStream(mPlaybackStream);
//        triggerWordCallback.start();
        vadCallback.start();
    } else {
        LOGE(TAG, "Failed to create playback stream (%p). Restart the app", mPlaybackStream);
        closeStream(mPlaybackStream);
    }
}

void AudioEngine::stopPlayingRecordedStream() {
    LOGD(TAG, "stopPlayingRecordedStream() called");
    stopStream(mPlaybackStream);
//    triggerWordCallback.stop();
    vadCallback.stop();
    closeStream(mPlaybackStream);
    mSoundRecording.setReadPositionToStart();
}

void AudioEngine::startPlayingFromFile(const char *filePath) {
    LOGD(TAG, "startPlayingFromFile() called");
    sndfileHandle = SndfileHandle(filePath);
    openPlaybackStreamFromFileParameters();
    if (mPlaybackStream != nullptr) {
        startStream(mPlaybackStream);
    } else {
        LOGE(TAG, "Failed to create playback stream (%p). Restart the app", mPlaybackStream);
        closeStream(mPlaybackStream);
    }
}

void AudioEngine::stopPlayingFromFile() {
    LOGD(TAG, "stopPlayingFromFile() called");
    stopStream(mPlaybackStream);
    closeStream(mPlaybackStream);
}

void AudioEngine::writeToFile(const char *filePath) {
    LOGD(TAG, "writeToFile() called");
    mSoundRecording.initiateWritingToFile(filePath, mOutputChannelCount, mSampleRate);
}

void AudioEngine::openRecordingStream() {
    LOGD(TAG, "openRecordingStream() called");
    oboe::AudioStreamBuilder builder;
    setUpRecordingStreamParameters(&builder);
    oboe::Result result = builder.openStream(&mRecordingStream);
    if (result == oboe::Result::OK && mRecordingStream) {
        assert(mRecordingStream->getChannelCount() == mInputChannelCount);
        mSampleRate = mRecordingStream->getSampleRate();
        mFormat = mRecordingStream->getFormat();
        LOGV(TAG, "openRecordingStream(): mSampleRate = ");
        LOGV(TAG, std::to_string(mSampleRate).c_str());

        LOGV(TAG, "openRecordingStream(): mFormat = ");
        LOGV(TAG, oboe::convertToText(mFormat));
    } else {
        LOGE(TAG, "Failed to create recording stream. Error: %s",
             oboe::convertToText(result));
    }
}

void AudioEngine::openPlaybackStreamFromRecordedStreamParameters() {
    LOGD(TAG, "openPlaybackStreamFromRecordedStreamParameters() called");
    oboe::AudioStreamBuilder builder;

    setUpPlaybackStreamParameters(&builder, mAudioApi, mFormat, &playingCallback,
                                  mPlaybackDeviceId, mSampleRate, mOutputChannelCount);
    oboe::Result result = builder.openStream(&mPlaybackStream);
    if (result == oboe::Result::OK && mPlaybackStream) {
        assert(mPlaybackStream->getChannelCount() == mOutputChannelCount);
        mFormat = mPlaybackStream->getFormat();
        mSampleRate = mPlaybackStream->getSampleRate();
        LOGD(TAG, "openPlaybackStreamFromRecordedStreamParameters(): mSampleRate = ");
        LOGD(TAG, std::to_string(mSampleRate).c_str());

        mFramesPerBurst = mPlaybackStream->getFramesPerBurst();
        LOGD(TAG, "openPlaybackStreamFromRecordedStreamParameters(): mFramesPerBurst = ");
        LOGD(TAG, std::to_string(mFramesPerBurst).c_str());

        // Setup the buffer size to the burst size - this will give us the minimum possible latency
        mPlaybackStream->setBufferSizeInFrames(mFramesPerBurst);
    } else {
        LOGE(TAG, "Failed to create playback stream. Error: %s",
             oboe::convertToText(result));
    }
}

void AudioEngine::openPlaybackStreamFromFileParameters() {
    LOGD(TAG, "openPlaybackStreamFromFileParameters() called");
    oboe::AudioStreamBuilder builder;
    mSampleRate = sndfileHandle.samplerate();
    int audioFileFormat = sndfileHandle.format();
    mOutputChannelCount = sndfileHandle.channels();
    LOGD(TAG, "openPlaybackStreamFromFileParameters(): audioFileFormat = ");
    LOGD(TAG, std::to_string(audioFileFormat).c_str());
    LOGD(TAG, "openPlaybackStreamFromFileParameters(): mSampleRate = ");
    LOGD(TAG, std::to_string(mSampleRate).c_str());

    setUpPlaybackStreamParameters(&builder, mAudioApi, mFormat, &playingCallback,
                                  mPlaybackDeviceId, mSampleRate, mOutputChannelCount);
    playingCallback.setPlaybackFromFile(true);

    oboe::Result result = builder.openStream(&mPlaybackStream);
    if (result == oboe::Result::OK && mPlaybackStream) {
        assert(mPlaybackStream->getChannelCount() == mOutputChannelCount);
        assert(mPlaybackStream->getFormat() == mFormat);

        mSampleRate = mPlaybackStream->getSampleRate();
        LOGV(TAG, "openPlaybackStreamFromFileParameters(): mSampleRate = ");
        LOGV(TAG, std::to_string(mSampleRate).c_str());
        mFramesPerBurst = mPlaybackStream->getFramesPerBurst();
        LOGV(TAG, "openPlaybackStreamFromFileParameters(): mFramesPerBurst = ");
        LOGV(TAG, std::to_string(mFramesPerBurst).c_str());
        mPlaybackStream->setBufferSizeInFrames(mFramesPerBurst);
    } else {
        LOGE(TAG, "Failed to create playback stream. Error: %s",
             oboe::convertToText(result));
    }
}

void AudioEngine::startStream(oboe::AudioStream *stream) {
    LOGD(TAG, "startStream() called");
    if(stream){
        oboe::Result result = stream->requestStart();
        if (result != oboe::Result::OK) {
            LOGE(TAG, "Error starting stream. %s", oboe::convertToText(result));
        }
    }
}

void AudioEngine::stopStream(oboe::AudioStream *stream) {
    LOGD(TAG, "stopStream() called");
    if (stream) {
        oboe::Result result = stream->stop(0L);
        if (result != oboe::Result::OK) {
            LOGE(TAG, "Error stopping stream. %s", oboe::convertToText(result));
        }
        LOGW(TAG, "stopStream(): mTotalSamples = ");
        LOGW(TAG, std::to_string(mSoundRecording.getTotalSamples()).c_str());
    }
}

void AudioEngine::closeStream(oboe::AudioStream *stream) {
    LOGD(TAG, "closeStream(): ");
    if (stream) {
        oboe::Result result = stream->close();
        if (result != oboe::Result::OK) {
            LOGE(TAG, "Error closing stream. %s", oboe::convertToText(result));
        } else {
            stream = nullptr;
        }
        LOGW(TAG, "closeStream(): mTotalSamples = ");
        LOGW(TAG, std::to_string(mSoundRecording.getTotalSamples()).c_str());
    }
}

oboe::AudioStreamBuilder *AudioEngine::setUpRecordingStreamParameters(
        oboe::AudioStreamBuilder *builder) {
    LOGD(TAG, "setUpRecordingStreamParameters() called");
    builder->setAudioApi(mAudioApi)
            ->setFormat(mFormat)
            ->setSharingMode(oboe::SharingMode::Exclusive)
            ->setPerformanceMode(oboe::PerformanceMode::LowLatency)
            ->setCallback(&recordingCallback)
            ->setDeviceId(mRecodingDeviceId)
            ->setDirection(oboe::Direction::Input)
            ->setChannelCount(mInputChannelCount)
            ->setSampleRate(mSampleRate);
    return builder;
}

oboe::AudioStreamBuilder *AudioEngine::setUpPlaybackStreamParameters(
        oboe::AudioStreamBuilder *builder, oboe::AudioApi audioApi, oboe::AudioFormat audioFormat,
        oboe::AudioStreamCallback *audioStreamCallback, int32_t deviceId, int32_t sampleRate,
        int channelCount) {
    LOGD(TAG, "setUpPlaybackStreamParameters() called");
    assert(audioStreamCallback != nullptr);
    builder->setAudioApi(audioApi)
            ->setFormat(audioFormat)
            ->setSharingMode(oboe::SharingMode::Shared)
            ->setPerformanceMode(oboe::PerformanceMode::LowLatency)
            ->setCallback(audioStreamCallback)
            ->setDeviceId(deviceId)
            ->setDirection(oboe::Direction::Output)
            ->setSampleRate(sampleRate)
            ->setChannelCount(channelCount);
    return builder;
}